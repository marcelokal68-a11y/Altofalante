/* Leitor/gravador WAV minimo (PCM 16-bit e float32), mono/estereo. Header-only. */
#ifndef AF_WAV_H
#define AF_WAV_H

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

namespace wav {

struct Audio {
    int sample_rate = 0;
    int channels = 0;
    std::vector<float> samples; // interleaved, [-1, 1]
    int frames() const { return channels ? (int)samples.size() / channels : 0; }
};

inline uint32_t rd32(const uint8_t* p) { return p[0]|(p[1]<<8)|(p[2]<<16)|((uint32_t)p[3]<<24); }
inline uint16_t rd16(const uint8_t* p) { return (uint16_t)(p[0]|(p[1]<<8)); }

inline bool read(const std::string& path, Audio& a) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> b(sz);
    if (fread(b.data(), 1, sz, f) != (size_t)sz) { fclose(f); return false; }
    fclose(f);
    if (sz < 44 || memcmp(b.data(), "RIFF", 4) || memcmp(b.data()+8, "WAVE", 4)) return false;

    uint16_t fmt = 1, ch = 0, bits = 0; uint32_t rate = 0;
    size_t pos = 12; const uint8_t* data = nullptr; uint32_t dataLen = 0;
    while (pos + 8 <= (size_t)sz) {
        const uint8_t* id = b.data() + pos;
        uint32_t clen = rd32(b.data() + pos + 4);
        const uint8_t* body = b.data() + pos + 8;
        if (!memcmp(id, "fmt ", 4)) {
            fmt = rd16(body); ch = rd16(body+2); rate = rd32(body+4); bits = rd16(body+14);
        } else if (!memcmp(id, "data", 4)) {
            data = body; dataLen = clen; break;
        }
        pos += 8 + clen + (clen & 1);
    }
    if (!data || ch == 0) return false;

    a.sample_rate = rate; a.channels = ch;
    if (fmt == 3 && bits == 32) {                 // float32
        size_t n = dataLen / 4; a.samples.resize(n);
        memcpy(a.samples.data(), data, dataLen);
    } else if (fmt == 1 && bits == 16) {          // PCM16
        size_t n = dataLen / 2; a.samples.resize(n);
        for (size_t i = 0; i < n; ++i)
            a.samples[i] = (int16_t)rd16(data + i*2) / 32768.0f;
    } else return false;
    return true;
}

inline void wr32(FILE* f, uint32_t v) { uint8_t p[4]={(uint8_t)v,(uint8_t)(v>>8),(uint8_t)(v>>16),(uint8_t)(v>>24)}; fwrite(p,1,4,f); }
inline void wr16(FILE* f, uint16_t v) { uint8_t p[2]={(uint8_t)v,(uint8_t)(v>>8)}; fwrite(p,1,2,f); }

// Grava como float32 (preserva tudo, inclusive picos para medicao de true-peak).
inline bool write(const std::string& path, const Audio& a) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    uint32_t dataLen = (uint32_t)(a.samples.size() * 4);
    uint32_t byteRate = a.sample_rate * a.channels * 4;
    fwrite("RIFF", 1, 4, f); wr32(f, 36 + dataLen); fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f); wr32(f, 16); wr16(f, 3); wr16(f, (uint16_t)a.channels);
    wr32(f, a.sample_rate); wr32(f, byteRate);
    wr16(f, (uint16_t)(a.channels * 4)); wr16(f, 32);
    fwrite("data", 1, 4, f); wr32(f, dataLen);
    fwrite(a.samples.data(), 1, dataLen, f);
    fclose(f);
    return true;
}

} // namespace wav

#endif
