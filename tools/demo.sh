#!/usr/bin/env bash
# Build + prova de valor end-to-end (Fase 1). Roda sem device.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo "==> Configurando e compilando (CMake)"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build build -j >/dev/null
echo "    OK"

echo "==> Testes de invariantes"
./build/tools/test_invariants

echo "==> Gerando audio de teste"
python3 tools/gen_test_audio.py

echo "==> Processando todos os presets"
mkdir -p test-audio/out
for p in voice balanced bass party portable; do
    ./build/tools/process_wav test-audio/sample.wav "test-audio/out/${p}.wav" "$p" >/dev/null
    echo "    -> ${p}"
done

echo "==> Medindo loudness (LUFS) e true-peak"
python3 tools/measure.py test-audio/sample.wav \
    test-audio/out/voice.wav \
    test-audio/out/balanced.wav \
    test-audio/out/bass.wav \
    test-audio/out/party.wav \
    test-audio/out/portable.wav
