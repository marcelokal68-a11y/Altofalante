import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'audio_engine.dart';

void main() => runApp(const AltofalanteApp());

class AltofalanteApp extends StatelessWidget {
  const AltofalanteApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Altofalante',
      theme: ThemeData(
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFF1DB954),
          brightness: Brightness.dark,
        ),
      ),
      home: const HomeScreen(),
    );
  }
}

/// Tela unica — "simplicidade extrema": 1 botao grande de Turbo, presets e play.
class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});
  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  final _engine = AudioEngine.instance;
  bool _turbo = true;
  bool _playing = false;
  String? _trackName;
  DspPreset _preset = DspPreset.balanced;

  @override
  void initState() {
    super.initState();
    _restorePreset();
  }

  Future<void> _restorePreset() async {
    final prefs = await SharedPreferences.getInstance();
    final saved = prefs.getString('preset');
    if (saved != null) {
      setState(() => _preset = DspPreset.values
          .firstWhere((p) => p.name == saved, orElse: () => DspPreset.balanced));
      await _engine.setPreset(_preset);
    }
  }

  Future<void> _pickTrack() async {
    final res = await FilePicker.platform.pickFiles(type: FileType.audio);
    final path = res?.files.single.path;
    if (path == null) return;
    await _engine.load(path);
    setState(() {
      _trackName = res!.files.single.name;
      _playing = false;
    });
  }

  Future<void> _togglePlay() async {
    if (_trackName == null) return _pickTrack();
    _playing ? await _engine.pause() : await _engine.play();
    setState(() => _playing = !_playing);
  }

  Future<void> _toggleTurbo() async {
    setState(() => _turbo = !_turbo);
    await _engine.setEnabled(_turbo);
  }

  Future<void> _selectPreset(DspPreset p) async {
    setState(() => _preset = p);
    await _engine.setPreset(p);
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('preset', p.name);
  }

  @override
  Widget build(BuildContext context) {
    final scheme = Theme.of(context).colorScheme;
    return Scaffold(
      appBar: AppBar(title: const Text('Altofalante')),
      body: Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            Text(_trackName ?? 'Toque para escolher uma musica',
                style: Theme.of(context).textTheme.titleMedium,
                textAlign: TextAlign.center),

            // Botao grande de TURBO (liga/desliga o DSP).
            GestureDetector(
              onTap: _toggleTurbo,
              child: Container(
                width: 200,
                height: 200,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  color: _turbo ? scheme.primary : scheme.surfaceContainerHighest,
                  boxShadow: _turbo
                      ? [BoxShadow(color: scheme.primary.withOpacity(0.5), blurRadius: 40)]
                      : null,
                ),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(Icons.graphic_eq,
                        size: 72,
                        color: _turbo ? scheme.onPrimary : scheme.onSurfaceVariant),
                    Text('TURBO ${_turbo ? "ON" : "OFF"}',
                        style: TextStyle(
                            color: _turbo ? scheme.onPrimary : scheme.onSurfaceVariant,
                            fontWeight: FontWeight.bold)),
                  ],
                ),
              ),
            ),

            // Seletor de preset.
            Wrap(
              spacing: 8,
              runSpacing: 8,
              alignment: WrapAlignment.center,
              children: [
                for (final p in DspPreset.values)
                  if (p != DspPreset.bypass)
                    ChoiceChip(
                      label: Text(p.label),
                      selected: _preset == p,
                      onSelected: (_) => _selectPreset(p),
                    ),
              ],
            ),

            // Play / pause.
            FilledButton.icon(
              onPressed: _togglePlay,
              icon: Icon(_playing ? Icons.pause : Icons.play_arrow),
              label: Text(_playing ? 'Pausar' : 'Tocar'),
              style: FilledButton.styleFrom(
                  minimumSize: const Size(180, 56),
                  textStyle: const TextStyle(fontSize: 18)),
            ),
          ],
        ),
      ),
    );
  }
}
