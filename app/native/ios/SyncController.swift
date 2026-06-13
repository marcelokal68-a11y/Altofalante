//  SyncController.swift
//  Multi-celular no iOS: dirige a API C do sync-core (altofalante/sync_api.h) e agenda
//  o início da reprodução no AudioEngineBridge para todos tocarem juntos.

import Foundation

final class SyncController {
    static let shared = SyncController()
    private var sync: OpaquePointer?           // AfSync*
    private let outputLatency = 0.0            // ajuste fino por device, se necessário

    func createGroup() {
        destroy()
        sync = af_sync_create(0, nil)          // theta 0 em produção
        af_sync_start_leader(sync)
    }

    func followerCount() -> Int { Int(af_sync_follower_count(sync)) }

    /// (Líder) dispara o início sincronizado e agenda o próprio áudio.
    func playSynced() {
        guard let s = sync else { return }
        let fire = af_sync_leader_play(s, 0.8, outputLatency)
        AudioEngineBridge.shared.playAt(fire)
    }

    /// (Seguidor) procura o líder, sincroniza e passa a aguardar o PLAY numa thread.
    func joinGroup(_ completion: @escaping (Bool, String) -> Void) {
        destroy()
        sync = af_sync_create(0, nil)
        DispatchQueue.global(qos: .userInitiated).async { [weak self] in
            guard let s = self?.sync else { return completion(false, "") }
            let ok = af_sync_join(s, 4000) == 0
            var info = ""
            if ok {
                var ip = [CChar](repeating: 0, count: 64)
                let port = af_sync_leader_endpoint(s, &ip, 64)
                info = "\(String(cString: ip)):\(port)"
                self?.waitLoop(s)
            }
            DispatchQueue.main.async { completion(ok, info) }
        }
    }

    /// Aguarda PLAYs do líder e agenda cada início (enquanto estiver no grupo).
    private func waitLoop(_ s: OpaquePointer) {
        DispatchQueue.global(qos: .userInteractive).async {
            while self.sync == s {
                let fire = af_sync_wait_play(s, self.outputLatency, 60_000)
                if fire >= 0 { AudioEngineBridge.shared.playAt(fire) }
            }
        }
    }

    func leave() { destroy() }

    private func destroy() {
        if let s = sync { sync = nil; af_sync_destroy(s) }
    }
}
