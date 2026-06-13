// native-lib.cpp
// Reprodução de áudio de baixa latência no Android via Oboe, aplicando o dsp-core
// (C++) em cada bloco. Chamado pelo MainActivity.kt através de JNI.
//
// IMPORTANTE: os nomes das funções JNI seguem o pacote `com.altofalante.app`.
// Se o seu applicationId for outro, ajuste os nomes (Java_<pacote>_MainActivity_...).

#include <jni.h>
#include <oboe/Oboe.h>
#include <vector>
#include <mutex>
#include <cstring>
#include "altofalante/dsp.h"

namespace {

class Player : public oboe::AudioStreamDataCallback {
public:
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream*, void* audioData,
                                          int32_t numFrames) override {
        auto* out = static_cast<float*>(audioData);
        std::lock_guard<std::mutex> lk(mtx_);
        const int ch = channels_;
        if (!playing_ || readIndex_ >= pcm_.size()) {
            std::memset(out, 0, sizeof(float) * numFrames * ch);
            return oboe::DataCallbackResult::Continue;
        }
        int framesAvail = (int)((pcm_.size() - readIndex_) / ch);
        int n = std::min(numFrames, framesAvail);
        std::memcpy(out, pcm_.data() + readIndex_, sizeof(float) * n * ch);
        readIndex_ += (size_t)n * ch;
        if (n < numFrames)
            std::memset(out + (size_t)n * ch, 0, sizeof(float) * (numFrames - n) * ch);
        af_process(dsp_, out, n); // turbina o bloco antes de sair
        return oboe::DataCallbackResult::Continue;
    }

    void setPcm(const float* data, size_t count, int sr, int ch) {
        std::lock_guard<std::mutex> lk(mtx_);
        closeStream();
        pcm_.assign(data, data + count);
        readIndex_ = 0; channels_ = ch; sampleRate_ = sr; playing_ = false;
        if (dsp_) af_destroy(dsp_);
        dsp_ = af_create(sr, ch);
        af_set_preset(dsp_, AF_PRESET_MUSIC_BALANCED);
        openStream();
    }

    void play()  { std::lock_guard<std::mutex> lk(mtx_); playing_ = true;  if (stream_) stream_->requestStart(); }
    void pause() { std::lock_guard<std::mutex> lk(mtx_); playing_ = false; }
    void setEnabled(bool on) { af_set_enabled(dsp_, on ? 1 : 0); }
    void setPreset(AfPreset p) { af_set_preset(dsp_, p); }

private:
    void openStream() {
        oboe::AudioStreamBuilder b;
        b.setDirection(oboe::Direction::Output)
         ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
         ->setSharingMode(oboe::SharingMode::Shared)
         ->setFormat(oboe::AudioFormat::Float)
         ->setChannelCount(channels_)
         ->setSampleRate(sampleRate_)
         ->setDataCallback(this);
        b.openStream(stream_);
    }
    void closeStream() {
        if (stream_) { stream_->stop(); stream_->close(); stream_.reset(); }
    }

    std::shared_ptr<oboe::AudioStream> stream_;
    std::vector<float> pcm_;
    size_t readIndex_ = 0;
    int channels_ = 2, sampleRate_ = 48000;
    bool playing_ = false;
    AfEngine* dsp_ = nullptr;
    std::mutex mtx_;
};

Player g_player;

AfPreset presetFor(const std::string& s) {
    if (s == "voice")    return AF_PRESET_VOICE;
    if (s == "balanced") return AF_PRESET_MUSIC_BALANCED;
    if (s == "bass")     return AF_PRESET_MUSIC_BASS;
    if (s == "party")    return AF_PRESET_PARTY;
    if (s == "portable") return AF_PRESET_PORTABLE;
    return AF_PRESET_BYPASS;
}

} // namespace

extern "C" {

JNIEXPORT void JNICALL
Java_com_altofalante_app_MainActivity_nativeSetPcm(JNIEnv* env, jobject,
        jfloatArray pcm, jint sampleRate, jint channels) {
    jsize n = env->GetArrayLength(pcm);
    jfloat* data = env->GetFloatArrayElements(pcm, nullptr);
    g_player.setPcm(data, (size_t)n, sampleRate, channels);
    env->ReleaseFloatArrayElements(pcm, data, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_com_altofalante_app_MainActivity_nativePlay(JNIEnv*, jobject) { g_player.play(); }

JNIEXPORT void JNICALL
Java_com_altofalante_app_MainActivity_nativePause(JNIEnv*, jobject) { g_player.pause(); }

JNIEXPORT void JNICALL
Java_com_altofalante_app_MainActivity_nativeSetEnabled(JNIEnv*, jobject, jboolean on) {
    g_player.setEnabled(on == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_altofalante_app_MainActivity_nativeSetPreset(JNIEnv* env, jobject, jstring name) {
    const char* s = env->GetStringUTFChars(name, nullptr);
    g_player.setPreset(presetFor(s));
    env->ReleaseStringUTFChars(name, s);
}

} // extern "C"
