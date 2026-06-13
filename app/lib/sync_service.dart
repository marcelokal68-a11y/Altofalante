import 'package:flutter/services.dart';

enum SyncRole { none, leader, follower }

/// Cliente do recurso multi-celular (canal `altofalante/sync`).
///
/// O trabalho de rede (descoberta multicast, sincronização de relógio, agendamento)
/// roda no lado NATIVO via a API C do sync-core (`altofalante/sync_api.h`). Aqui só
/// disparamos os comandos e recebemos o estado.
class SyncService {
  SyncService._();
  static final SyncService instance = SyncService._();

  static const MethodChannel _ch = MethodChannel('altofalante/sync');

  /// Vira "líder": começa a anunciar a presença na rede Wi-Fi.
  Future<void> createGroup() => _ch.invokeMethod('createGroup');

  /// Quantos aparelhos já entraram no grupo (líder).
  Future<int> followerCount() async =>
      (await _ch.invokeMethod<int>('followerCount')) ?? 0;

  /// Procura um líder na rede e sincroniza. Retorna {ok, leader}.
  Future<Map<String, dynamic>> joinGroup() async {
    final r = await _ch.invokeMethod<Map<dynamic, dynamic>>('joinGroup');
    return (r ?? {}).map((k, v) => MapEntry(k.toString(), v));
  }

  /// (Líder) dispara o início sincronizado em todos os aparelhos do grupo.
  Future<void> playSynced() => _ch.invokeMethod('playSynced');

  /// Sai do grupo.
  Future<void> leave() => _ch.invokeMethod('leave');
}
