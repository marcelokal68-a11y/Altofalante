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

  /// Liga/desliga o modo estéreo (cada celular vira um canal L/R).
  Future<void> setStereo(bool on) => _ch.invokeMethod('setStereo', {'enabled': on});

  /// Canal deste aparelho: 0=ambos, 1=esquerda, 2=direita.
  Future<int> channel() async => (await _ch.invokeMethod<int>('channel')) ?? 0;

  /// Define o nome deste aparelho (mostrado para o comando).
  Future<void> setName(String name) => _ch.invokeMethod('setName', {'name': name});

  /// (Líder) nomes dos aparelhos conectados.
  Future<List<String>> followerNames() async {
    final r = await _ch.invokeListMethod<dynamic>('followerNames');
    return (r ?? []).map((e) => e.toString()).toList();
  }

  /// Sai do grupo.
  Future<void> leave() => _ch.invokeMethod('leave');
}

String channelLabel(int c) => c == 1 ? 'ESQ' : c == 2 ? 'DIR' : '';
