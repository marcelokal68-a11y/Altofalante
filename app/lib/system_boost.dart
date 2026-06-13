import 'package:flutter/services.dart';

/// "Amplificar TUDO" — efeitos de áudio globais (Pilar 2).
///
/// **Somente Android.** Turbina o som de qualquer app (Spotify, YouTube, jogos) via
/// efeitos na sessão de saída global. No iOS isso não existe (a Apple bloqueia), então
/// use o player próprio (`AudioEngine`). Cheque `Platform.isAndroid` antes de mostrar.
class SystemBoost {
  SystemBoost._();
  static final SystemBoost instance = SystemBoost._();
  static const MethodChannel _ch = MethodChannel('altofalante/system');

  /// Liga o boost global. Retorna true se o aparelho permitiu (sessão 0).
  Future<bool> enable(String preset) async =>
      (await _ch.invokeMethod<bool>('enable', {'preset': preset})) ?? false;

  Future<void> setPreset(String preset) => _ch.invokeMethod('setPreset', {'preset': preset});
  Future<void> setGainDb(double db) => _ch.invokeMethod('setGainDb', {'db': db});
  Future<void> disable() => _ch.invokeMethod('disable');
}
