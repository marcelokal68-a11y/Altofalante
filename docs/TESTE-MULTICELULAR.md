# Como testar a sincronização multi-celular — passo a passo

> **Leia primeiro (a verdade honesta):**
> - ✅ **Dá pra testar HOJE:** a *sincronização* (o coração da mágica) — os
>   "aparelhos" se acham na rede e disparam juntos, no mesmo instante.
> - ⏳ **Ainda NÃO dá:** *ouvir música sincronizada* nos dois celulares. Isso depende
>   da **ponte nativa** (em construção) + instalar o app no aparelho.
>
> Ou seja: hoje você prova que a sincronia funciona; ouvir nos celulares é o próximo marco.

Tem dois testes. Comece pelo A (mais fácil, 1 computador).

---

## ✅ Teste A — Agora, num computador só (recomendado começar aqui)

Esse teste cria **vários "celulares virtuais"** no seu computador e mostra, em números,
o quão sincronizados eles ficam.

**Precisa de:** um Mac ou Linux (no Windows, use o "WSL"). 10 minutos.

1. **Abra o Terminal.**

2. **Instale as ferramentas (uma vez só):**
   - **Mac:** 
     ```bash
     xcode-select --install      # instala o compilador (clique em "Instalar")
     brew install cmake          # se não tiver o brew: https://brew.sh
     ```
   - **Ubuntu/Linux:**
     ```bash
     sudo apt update && sudo apt install -y build-essential cmake git
     ```

3. **Baixe o projeto e entre na pasta:**
   ```bash
   git clone https://github.com/marcelokal68-a11y/Altofalante.git
   cd Altofalante
   ```

4. **Compile:**
   ```bash
   cmake -S . -B build && cmake --build build
   ```

5. **Rode o teste:**
   ```bash
   bash tools/sync_net_test.sh
   ```

6. **O que você deve ver** (algo assim):
   ```
   follower theta=+12.500: descobriu lider em 127.0.0.1:45815
   ...
   Defasagem real entre 3 aparelhos: 0.009 ms (meta < 10 ms)
   SYNC REDE OK
   ```
   👉 **O que isso significa:** 3 aparelhos com relógios totalmente diferentes se
   acharam sozinhos e dispararam com **menos de 1 centésimo de milissegundo** de
   diferença. O ouvido humano só percebe a partir de ~20 ms — então isso é
   **sincronia perfeita**.

---

## 📡 Teste B — Entre dois computadores na mesma Wi-Fi (avançado)

Aqui você vê os dois se **acharem de verdade pela rede Wi-Fi**, sem digitar IP de cara.

**Precisa de:** dois computadores (Mac/Linux) na **mesma rede Wi-Fi**, ambos com o
projeto já compilado (repita os passos 1–4 do Teste A nos dois).

1. **Descubra o IP local de cada computador:**
   - **Mac:** `ipconfig getifaddr en0`  (ex.: `192.168.0.10`)
   - **Linux:** `hostname -I`  (pegue o primeiro número)
   - Anote: digamos **PC-A = 192.168.0.10** (será o líder) e **PC-B = 192.168.0.11**.

2. **No PC-A (líder)** — cole, trocando pelo IP do PC-A:
   ```bash
   AF_IFACE=192.168.0.10 ./build/sync-core/sync_node leader 1 0
   ```

3. **No PC-B (seguidor)** — cole, trocando pelo IP do PC-B:
   ```bash
   AF_IFACE=192.168.0.11 ./build/sync-core/sync_node follower 0 0 /tmp/resultado.txt 20
   ```

4. **O que você deve ver no PC-B:**
   ```
   follower theta=+0.000: descobriu lider em 192.168.0.10:48123
   ```
   👉 **Isso é o momento mágico:** o PC-B achou o PC-A sozinho, pela Wi-Fi, e os dois
   se sincronizaram. (A defasagem em milissegundos só aparece somada quando é tudo no
   mesmo computador, porque aí eles compartilham o mesmo relógio de referência.)

**Se não funcionar:**
- Confirme que os dois estão na **mesma Wi-Fi**.
- **Firewall:** libere conexões UDP (no Mac, Ajustes > Rede > Firewall; tente desligar
  pra testar).
- Alguns roteadores têm "isolamento de clientes" (AP isolation) — desligue.

---

## ⏳ Teste C — Ouvir nos dois celulares (próximo marco)

Para tocar a **mesma música em sincronia em dois celulares de verdade**, falta:
1. A **ponte nativa** (liga o motor de som ao áudio do celular) — _em construção_.
2. Instalar o app nos aparelhos (via Xcode/Android Studio).
3. Trocar a interface de loopback pela Wi-Fi no app (o `AF_IFACE` que você viu acima).

Quando essas três peças estiverem prontas, o teste vira: abrir o app nos dois celulares,
um vira "líder", tocar — e os dois soltam o som juntos. 🎶
