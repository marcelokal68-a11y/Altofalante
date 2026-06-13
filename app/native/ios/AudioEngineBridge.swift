//  AudioEngineBridge.swift
//  Toca um arquivo de áudio aplicando o dsp-core (C++) em tempo real, via
//  AVAudioEngine + AVAudioSourceNode. Chamado pelo MethodChannel no AppDelegate.
//
//  Visão geral: o arquivo é decodificado para PCM float na memória; um nó-fonte
//  entrega blocos ao alto-falante, e cada bloco passa por `af_process` antes de sair.

import AVFoundation

final class AudioEngineBridge {
    static let shared = AudioEngineBridge()

    private let engine = AVAudioEngine()
    private var sourceNode: AVAudioSourceNode?
    private var buffer: AVAudioPCMBuffer?
    private var readIndex: Int = 0
    private var playing = false
    private var channels = 2

    private var dsp: OpaquePointer?            // AfEngine*
    private var scratch: UnsafeMutablePointer<Float>?
    private var scratchCap = 0                 // em floats (frames * canais)

    // MARK: - Comandos vindos do Flutter

    func load(path: String) throws {
        let url = URL(fileURLWithPath: path)
        let file = try AVAudioFile(forReading: url)
        let fmt = file.processingFormat        // float32, não-interleaved
        channels = Int(fmt.channelCount)

        guard let buf = AVAudioPCMBuffer(pcmFormat: fmt,
                                         frameCapacity: AVAudioFrameCount(file.length)) else {
            throw NSError(domain: "altofalante", code: 1)
        }
        try file.read(into: buf)

        // (re)cria o motor de DSP para o sample-rate/canais do arquivo.
        if let d = dsp { af_destroy(d) }
        dsp = af_create(Int32(fmt.sampleRate), Int32(channels))
        af_set_preset(dsp, AF_PRESET_MUSIC_BALANCED)

        buffer = buf
        readIndex = 0
        setupGraph(format: fmt)
    }

    func play() {
        playing = true
        if !engine.isRunning { try? engine.start() }
    }

    func pause() { playing = false }

    func setEnabled(_ on: Bool) { af_set_enabled(dsp, on ? 1 : 0) }

    func setPreset(_ name: String) { af_set_preset(dsp, presetFor(name)) }

    // MARK: - Grafo de áudio

    private func setupGraph(format: AVAudioFormat) {
        if let n = sourceNode { engine.detach(n) }
        let node = AVAudioSourceNode(format: format) { [weak self] _, _, frameCount, ablPtr in
            self?.render(frameCount: Int(frameCount), abl: ablPtr) ?? noErr
        }
        sourceNode = node
        engine.attach(node)
        // O mixer principal cuida da conversão para o sample-rate do hardware.
        engine.connect(node, to: engine.mainMixerNode, format: format)
        engine.prepare()
        try? engine.start()
    }

    /// Bloco de render (thread de áudio em tempo real).
    private func render(frameCount n: Int,
                        abl: UnsafeMutablePointer<AudioBufferList>) -> OSStatus {
        let out = UnsafeMutableAudioBufferListPointer(abl)
        guard playing, let buf = buffer,
              let src = buf.floatChannelData,
              readIndex < Int(buf.frameLength) else {
            for b in out { memset(b.mData, 0, Int(b.mDataByteSize)) }
            return noErr
        }

        let ch = channels
        let avail = min(n, Int(buf.frameLength) - readIndex)
        ensureScratch(avail * ch)

        // intercala canais -> scratch (formato esperado pelo dsp-core)
        for i in 0..<avail {
            for c in 0..<ch { scratch![i * ch + c] = src[c][readIndex + i] }
        }
        af_process(dsp, scratch!, Int32(avail))

        // desintercala scratch -> buffers de saída
        for (c, b) in out.enumerated() where c < ch {
            let dst = b.mData!.assumingMemoryBound(to: Float.self)
            for i in 0..<avail { dst[i] = scratch![i * ch + c] }
            if avail < n { for i in avail..<n { dst[i] = 0 } } // silêncio no fim
        }
        readIndex += avail
        return noErr
    }

    private func ensureScratch(_ needed: Int) {
        if needed <= scratchCap { return }
        scratch?.deallocate()
        scratch = UnsafeMutablePointer<Float>.allocate(capacity: needed)
        scratchCap = needed
    }

    private func presetFor(_ name: String) -> AfPreset {
        switch name {
        case "voice":    return AF_PRESET_VOICE
        case "balanced": return AF_PRESET_MUSIC_BALANCED
        case "bass":     return AF_PRESET_MUSIC_BASS
        case "party":    return AF_PRESET_PARTY
        case "portable": return AF_PRESET_PORTABLE
        default:         return AF_PRESET_BYPASS
        }
    }
}
