// MainActivity.kt
// Substitui o MainActivity gerado pelo `flutter create`. Liga o MethodChannel
// "altofalante/engine" às funções nativas (Oboe + dsp-core) e decodifica o arquivo
// de áudio para PCM float usando o MediaCodec do Android.
//
// Ajuste o `package` abaixo para o seu applicationId (e os nomes JNI no native-lib.cpp).

package com.altofalante.app

import android.media.MediaCodec
import android.media.MediaExtractor
import android.media.MediaFormat
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import java.nio.ByteOrder
import kotlin.concurrent.thread

class MainActivity : FlutterActivity() {

    private external fun nativeSetPcm(pcm: FloatArray, sampleRate: Int, channels: Int)
    private external fun nativePlay()
    private external fun nativePause()
    private external fun nativeSetEnabled(enabled: Boolean)
    private external fun nativeSetPreset(preset: String)

    companion object {
        init { System.loadLibrary("altofalante") }
    }

    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, "altofalante/engine")
            .setMethodCallHandler { call, result ->
                when (call.method) {
                    "load" -> {
                        val path = call.argument<String>("path")
                        if (path == null) { result.success(false); return@setMethodCallHandler }
                        thread {
                            try {
                                val (pcm, sr, ch) = decodeToPcm(path)
                                runOnUiThread { nativeSetPcm(pcm, sr, ch); result.success(true) }
                            } catch (e: Exception) {
                                runOnUiThread { result.error("load", e.message, null) }
                            }
                        }
                    }
                    "play" -> { nativePlay(); result.success(null) }
                    "pause" -> { nativePause(); result.success(null) }
                    "setEnabled" -> { nativeSetEnabled(call.argument<Boolean>("enabled") ?: true); result.success(null) }
                    "setPreset" -> { nativeSetPreset(call.argument<String>("preset") ?: "balanced"); result.success(null) }
                    else -> result.notImplemented()
                }
            }
    }

    /// Decodifica um arquivo de áudio (MP3/AAC/WAV...) para PCM float interleaved.
    private fun decodeToPcm(path: String): Triple<FloatArray, Int, Int> {
        val extractor = MediaExtractor().apply { setDataSource(path) }
        val trackIndex = (0 until extractor.trackCount).first {
            extractor.getTrackFormat(it).getString(MediaFormat.KEY_MIME)?.startsWith("audio/") == true
        }
        val format = extractor.getTrackFormat(trackIndex)
        extractor.selectTrack(trackIndex)
        val sampleRate = format.getInteger(MediaFormat.KEY_SAMPLE_RATE)
        val channels = format.getInteger(MediaFormat.KEY_CHANNEL_COUNT)

        val codec = MediaCodec.createDecoderByType(format.getString(MediaFormat.KEY_MIME)!!)
        codec.configure(format, null, null, 0)
        codec.start()

        val out = ArrayList<Float>()
        val info = MediaCodec.BufferInfo()
        var inputDone = false
        var outputDone = false

        while (!outputDone) {
            if (!inputDone) {
                val inIdx = codec.dequeueInputBuffer(10_000)
                if (inIdx >= 0) {
                    val buf = codec.getInputBuffer(inIdx)!!
                    val size = extractor.readSampleData(buf, 0)
                    if (size < 0) {
                        codec.queueInputBuffer(inIdx, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                        inputDone = true
                    } else {
                        codec.queueInputBuffer(inIdx, 0, size, extractor.sampleTime, 0)
                        extractor.advance()
                    }
                }
            }
            val outIdx = codec.dequeueOutputBuffer(info, 10_000)
            if (outIdx >= 0) {
                val buf = codec.getOutputBuffer(outIdx)!!
                val shorts = buf.order(ByteOrder.nativeOrder()).asShortBuffer()
                while (shorts.hasRemaining()) out.add(shorts.get() / 32768.0f) // PCM16 -> float
                codec.releaseOutputBuffer(outIdx, false)
                if (info.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) outputDone = true
            }
        }
        codec.stop(); codec.release(); extractor.release()
        return Triple(out.toFloatArray(), sampleRate, channels)
    }
}
