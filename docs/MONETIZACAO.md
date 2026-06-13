# Monetização — como vender o Altofalante

> **Verdade primeiro:** ter o código não é ter o app à venda. Para vender faltam:
> build testado em aparelho, contas de desenvolvedor, sistema de pagamento (IAP) e
> conta bancária/impostos. Este doc define o **modelo** e os **passos**.

## Modelo recomendado: Freemium (grátis + "Pro")
A maioria dos apps de áudio/volume vende assim. Funciona melhor que app pago.

**Grátis (atrai usuários):**
- Botão Turbo + 2 modos (Equilibrado e Voz).
- 1 aparelho.
- (Opcional) anúncios discretos.

**Pro (o que você vende):**
- Todos os modos (Grave+, Festa, Caixa) + ✨ Auto.
- **Multi-celular** (festa).
- **Android: amplificar TUDO** (Spotify/YouTube).
- Sem anúncios.

## Preço sugerido (Brasil)
- **Pro vitalício (1x): R$ 11,90 – R$ 19,90** ← recomendado (usuário prefere pagar uma vez).
- Alternativa assinatura: **R$ 3,90/mês ou R$ 19,90/ano**.
- As lojas ficam com **15–30%** (Apple/Google). Você recebe o resto.
- Dica: comece com Pro vitalício barato (R$ 12,90) para validar conversão.

## Onde fica no código (a implementar)
- Dependência de compras já adicionada: `in_app_purchase` em `app/pubspec.yaml`.
- A criar: `app/lib/pro.dart` (estado Pro + compra/restauração) e o gating em
  `app/lib/main.dart` (modos/recursos bloqueados com 🔒 → tela "Desbloquear Pro").
- IDs dos produtos a criar nas lojas: `altofalante_pro` (vitalício) e/ou
  `altofalante_pro_year` (assinatura).

## Passo a passo para VENDER (na sua máquina/contas)
1. **Contas:** Apple Developer (US$99/ano) e Google Play (US$25 única vez).
2. **Conta bancária/impostos:** preencha "Payments profile" nas duas lojas (sem isso não
   recebe). Pode pedir dados fiscais.
3. **Criar os produtos IAP** no App Store Connect e no Play Console com os IDs acima e o
   preço.
4. **Compilar e testar** o app no aparelho (ver `docs/DO-ZERO-AO-CELULAR.md`), incluindo a
   compra em modo sandbox/teste.
5. **Listagens:** textos (`docs/LOJAS.md`, `docs/ANUNCIO-ANDROID.md`), screenshots,
   ícone, política de privacidade (`docs/privacidade.html` — hospede e ponha a URL).
6. **Enviar para revisão.** Apple costuma revisar com rigor (teste a compra antes).
7. **Publicar e divulgar.**

## Estratégia por plataforma
- **Android primeiro** (mais barato publicar, e tem o "amplificar tudo" como gancho forte).
- **iOS depois** (foco em "suas músicas/podcasts" + multi-celular).

## Checklist do que ainda falta para vender
- [ ] Build testado em aparelho real (iOS e/ou Android).
- [ ] Contas de desenvolvedor + perfil de pagamentos (banco).
- [ ] Produtos IAP criados e testados (sandbox).
- [ ] Screenshots reais + política hospedada (URL).
- [ ] Revisão das lojas aprovada.
