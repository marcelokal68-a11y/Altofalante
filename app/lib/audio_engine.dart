import 'package:flutter/services.dart';

/// Presets do motor de DSP (espelham o enum AfPreset do dsp-core C++).
enum DspPreset {
  bypass('Original', 'Sem processamento'),
  voice('Voz', 'Podcast / fala'),
  balanced('Equilibrado', 'Musica geral'),
  bass('Grave+', 'Mais corpo'),
  party('Festa', 'Loudness maximo'),
  portable('Caixa', 'Voicing portatil');

  const DspPreset(this.label, this.hint);
  final String label;
  final String hint;
}

/// Cliente Dart do motor de audio nativo.
///
/// O processamento de audio em tempo real roda no lado NATIVO (iOS:
/// AVAudioEngine + dsp-core; Android: Oboe + dsp-core). Esta classe so envia
/// comandos/parametros via MethodChannel — nunca processa amostras em Dart
/// (ver specs/02-architecture.md).
class AudioEngine {
  AudioEngine._();
  static final AudioEngine instance = AudioEngine._();

  static const MethodChannel _ch = MethodChannel('altofalante/engine');

  /// Carrega um arquivo de audio local (caminho no dispositivo).
  Future<void> load(String path) => _ch.invokeMethod('load', {'path': path});

  Future<void> play() => _ch.invokeMethod('play');
  Future<void> pause() => _ch.invokeMethod('pause');

  /// Liga/desliga o DSP (comparacao A/B instantanea — RF4).
  Future<void> setEnabled(bool enabled) =>
      _ch.invokeMethod('setEnabled', {'enabled': enabled});

  /// Troca de preset sem cortar o audio (RF3).
  Future<void> setPreset(DspPreset preset) =>
      _ch.invokeMethod('setPreset', {'preset': preset.name});
}
