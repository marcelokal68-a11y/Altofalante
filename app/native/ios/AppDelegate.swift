//  AppDelegate.swift
//  Substitui o AppDelegate gerado pelo `flutter create` para registrar o
//  MethodChannel "altofalante/engine" e ligá-lo ao AudioEngineBridge.

import UIKit
import Flutter
import AVFoundation

@main
@objc class AppDelegate: FlutterAppDelegate {
    override func application(
        _ application: UIApplication,
        didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
    ) -> Bool {
        // Sessão de áudio para reprodução (alto-falante).
        try? AVAudioSession.sharedInstance().setCategory(.playback, mode: .default)
        try? AVAudioSession.sharedInstance().setActive(true)

        let controller = window?.rootViewController as! FlutterViewController
        let channel = FlutterMethodChannel(name: "altofalante/engine",
                                           binaryMessenger: controller.binaryMessenger)
        let engine = AudioEngineBridge.shared

        channel.setMethodCallHandler { call, result in
            let args = call.arguments as? [String: Any]
            switch call.method {
            case "load":
                guard let path = args?["path"] as? String else { return result(false) }
                do { try engine.load(path: path); result(true) }
                catch { result(FlutterError(code: "load", message: "\(error)", details: nil)) }
            case "play":       engine.play();   result(nil)
            case "pause":      engine.pause();  result(nil)
            case "setEnabled": engine.setEnabled((args?["enabled"] as? Bool) ?? true); result(nil)
            case "setPreset":  engine.setPreset((args?["preset"] as? String) ?? "balanced"); result(nil)
            default:           result(FlutterMethodNotImplemented)
            }
        }

        // Canal do multi-celular.
        let syncChannel = FlutterMethodChannel(name: "altofalante/sync",
                                               binaryMessenger: controller.binaryMessenger)
        let sync = SyncController.shared
        syncChannel.setMethodCallHandler { call, result in
            switch call.method {
            case "createGroup":    sync.createGroup(); result(nil)
            case "followerCount":  result(sync.followerCount())
            case "playSynced":     sync.playSynced(); result(nil)
            case "joinGroup":      sync.joinGroup { ok, info in result(["ok": ok, "leader": info]) }
            case "leave":          sync.leave(); result(nil)
            default:               result(FlutterMethodNotImplemented)
            }
        }

        GeneratedPluginRegistrant.register(with: self)
        return super.application(application, didFinishLaunchingWithOptions: launchOptions)
    }
}
