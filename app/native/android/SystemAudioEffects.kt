// SystemAudioEffects.kt
// Pilar 2 (Android): turbina TODO o áudio do sistema — Spotify, YouTube, jogos —
// anexando efeitos à sessão de saída global (audioSession = 0).
// Permissão necessária: MODIFY_AUDIO_SETTINGS.
//
// Caveat honesto: alguns aparelhos/OEMs restringem efeitos na sessão 0. O código trata
// falhas com try/catch; se não pegar, o app deve usar o player próprio (dsp-core) como
// garantia. Ajuste o `package` para o seu applicationId.

package com.altofalante.app

import android.media.audiofx.BassBoost
import android.media.audiofx.DynamicsProcessing
import android.media.audiofx.Equalizer
import android.media.audiofx.LoudnessEnhancer
import android.os.Build

/** Efeitos globais (sessão 0). Mapeia os mesmos presets do dsp-core. */
class SystemAudioEffects {
    private var loudness: LoudnessEnhancer? = null
    private var bass: BassBoost? = null
    private var eq: Equalizer? = null
    private var dp: DynamicsProcessing? = null // limitador (evita distorcer o boost)
    private var enabled = false

    // Presets: ganho de loudness (mB), força do grave (0..1000), realce de presença (mB).
    private data class P(val gainMb: Int, val bass: Short, val presenceMb: Short)
    private val presets = mapOf(
        "voice"    to P(700,   0, 600),
        "balanced" to P(700, 300, 400),
        "bass"     to P(800, 800, 300),
        "party"    to P(1100, 600, 400),
        "portable" to P(800, 500, 600),
    )

    /** Liga os efeitos globais com um preset. Retorna true se conseguiu anexar. */
    fun enable(preset: String): Boolean {
        release()
        val p = presets[preset] ?: presets["balanced"]!!
        var ok = false
        try { loudness = LoudnessEnhancer(0).apply { setTargetGain(p.gainMb); enabled = true }; ok = true } catch (_: Throwable) {}
        // Limitador true-peak-ish global: segura o boost antes de distorcer (API 28+).
        try {
            if (Build.VERSION.SDK_INT >= 28) {
                val ch = 2
                val cfg = DynamicsProcessing.Config.Builder(
                    DynamicsProcessing.VARIANT_FAVOR_FREQUENCY_RESOLUTION, ch,
                    false, 0, false, 0, false, 0, true
                ).build()
                dp = DynamicsProcessing(0, 0, cfg).apply {
                    // Limiter(inUse, enabled, linkGroup, attack ms, release ms, ratio, threshold dB, postGain dB)
                    setLimiterAllChannelsTo(DynamicsProcessing.Limiter(true, true, 0, 1f, 60f, 20f, -1f, 0f))
                    enabled = true
                }
            }
        } catch (_: Throwable) {}
        try { bass = BassBoost(0, 0).apply { if (strengthSupported) setStrength(p.bass); enabled = true } } catch (_: Throwable) {}
        try {
            eq = Equalizer(0, 0).apply {
                enabled = true
                // realça a banda mais próxima de ~3 kHz (presença/inteligibilidade)
                val bands = numberOfBands.toInt()
                var best = 0; var bestDist = Int.MAX_VALUE
                for (b in 0 until bands) {
                    val f = getCenterFreq(b.toShort()) / 1000 // Hz
                    val d = kotlin.math.abs(f - 3000)
                    if (d < bestDist) { bestDist = d; best = b }
                }
                try { setBandLevel(best.toShort(), p.presenceMb) } catch (_: Throwable) {}
            }
        } catch (_: Throwable) {}
        enabled = ok
        return ok
    }

    /** Ajusta só o ganho de loudness (mB). 600 = +6 dB. */
    fun setGainMb(mb: Int) { try { loudness?.setTargetGain(mb) } catch (_: Throwable) {} }

    fun setPreset(preset: String) { if (enabled) enable(preset) }

    fun disable() { release(); enabled = false }

    private fun release() {
        try { loudness?.release() } catch (_: Throwable) {}
        try { bass?.release() } catch (_: Throwable) {}
        try { eq?.release() } catch (_: Throwable) {}
        try { dp?.release() } catch (_: Throwable) {}
        loudness = null; bass = null; eq = null; dp = null
    }
}
