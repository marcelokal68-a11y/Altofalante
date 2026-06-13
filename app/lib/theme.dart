import 'package:flutter/material.dart';

/// Tokens de marca do Altofalante (ver branding/brand-guide.md).
class Brand {
  static const bg = Color(0xFF0A0A0F);
  static const surface = Color(0xFF16161F);
  static const surface2 = Color(0xFF1F1F2B);
  static const primary = Color(0xFFA855F7); // violeta
  static const primary2 = Color(0xFFEC4899); // rosa (cor de marca)
  static const accent = Color(0xFF22D3EE); // ciano
  static const text = Color(0xFFF5F5FA);
  static const muted = Color(0xFF8B8B99);

  /// Gradiente da marca (botao Turbo, icone, destaques).
  static const gradient = LinearGradient(
    begin: Alignment.topLeft,
    end: Alignment.bottomRight,
    colors: [Color(0xFFA855F7), Color(0xFFD946C6), Color(0xFFEC4899)],
  );

  static ThemeData theme() {
    final base = ThemeData(brightness: Brightness.dark, useMaterial3: true);
    return base.copyWith(
      scaffoldBackgroundColor: bg,
      colorScheme: const ColorScheme.dark(
        primary: primary2,
        secondary: accent,
        surface: surface,
        onSurface: text,
      ),
      textTheme: base.textTheme.apply(
        bodyColor: text,
        displayColor: text,
        fontFamily: 'Sora', // adicione a fonte no pubspec ou via google_fonts
      ),
    );
  }
}
