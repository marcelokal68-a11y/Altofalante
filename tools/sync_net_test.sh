#!/usr/bin/env bash
# Teste de sincronizacao multi-celular sobre UDP REAL (localhost).
# Sobe 1 lider + 3 seguidores (relogios e latencias diferentes) e mede a
# defasagem real entre os instantes de emissao. Meta: < 10 ms.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/build/sync-core/sync_node"
PORT=45123
RES="$(mktemp)"
: > "$RES"

[ -x "$BIN" ] || { echo "compile primeiro: cmake --build build"; exit 1; }

# Lider (theta=0, espera 3 seguidores).
"$BIN" leader "$PORT" 3 0.0 &
LPID=$!
sleep 0.3

# Seguidores: relogios bem dessincronizados e latencias de saida distintas.
"$BIN" follower 127.0.0.1 "$PORT"  12.500 0.030 "$RES" 20 & F1=$!
"$BIN" follower 127.0.0.1 "$PORT"  -7.000 0.045 "$RES" 20 & F2=$!
"$BIN" follower 127.0.0.1 "$PORT"  33.200 0.020 "$RES" 20 & F3=$!

wait $F1 $F2 $F3
kill $LPID 2>/dev/null || true
wait $LPID 2>/dev/null || true

echo "--- instantes de emissao (CLOCK_MONOTONIC, s) ---"
cat "$RES"
python3 - "$RES" <<'EOF'
import sys
v = [float(x) for x in open(sys.argv[1]) if x.strip()]
if len(v) < 2:
    print("ERRO: poucas amostras"); sys.exit(1)
skew = (max(v) - min(v)) * 1000.0
print(f"\nDefasagem real entre {len(v)} aparelhos: {skew:.3f} ms (meta < 10 ms)")
print("SYNC REDE OK" if skew < 10.0 else "SYNC REDE FALHOU")
sys.exit(0 if skew < 10.0 else 1)
EOF
rm -f "$RES"
