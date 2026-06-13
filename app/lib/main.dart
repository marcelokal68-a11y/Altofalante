import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'audio_engine.dart';
import 'theme.dart';

void main() => runApp(const AltofalanteApp());

class AltofalanteApp extends StatelessWidget {
  const AltofalanteApp({super.key});
  @override
  Widget build(BuildContext context) => MaterialApp(
        title: 'Altofalante',
        debugShowCheckedModeBanner: false,
        theme: Brand.theme(),
        home: const HomeScreen(),
      );
}

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
    if (saved != null && mounted) {
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
    return Scaffold(
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 24),
          child: Column(
            children: [
              const SizedBox(height: 12),
              const _Header(),
              const SizedBox(height: 20),
              _NowPlaying(track: _trackName, onTap: _pickTrack),
              const Spacer(),
              _TurboButton(on: _turbo, onTap: _toggleTurbo),
              const SizedBox(height: 18),
              const _LoudBadge(),
              const SizedBox(height: 24),
              _Presets(selected: _preset, onSelect: _selectPreset),
              const SizedBox(height: 20),
              const _MultiDeviceLink(),
              const Spacer(),
              _PlayerControls(playing: _playing, onPlay: _togglePlay),
              const SizedBox(height: 16),
            ],
          ),
        ),
      ),
    );
  }
}

class _Header extends StatelessWidget {
  const _Header();
  @override
  Widget build(BuildContext context) => Row(
        children: [
          Container(
            width: 30,
            height: 30,
            decoration: BoxDecoration(
                gradient: Brand.gradient,
                borderRadius: BorderRadius.circular(8)),
            child: const Icon(Icons.graphic_eq, size: 18, color: Colors.white),
          ),
          const SizedBox(width: 10),
          RichText(
            text: const TextSpan(
              style: TextStyle(fontSize: 20, fontWeight: FontWeight.w800),
              children: [
                TextSpan(text: 'alto', style: TextStyle(color: Brand.text)),
                TextSpan(text: 'falante', style: TextStyle(color: Brand.primary2)),
              ],
            ),
          ),
          const Spacer(),
          const Icon(Icons.more_vert, color: Brand.muted),
        ],
      );
}

class _NowPlaying extends StatelessWidget {
  const _NowPlaying({required this.track, required this.onTap});
  final String? track;
  final VoidCallback onTap;
  @override
  Widget build(BuildContext context) => InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(14),
        child: Row(
          children: [
            Container(
              width: 56,
              height: 56,
              decoration: BoxDecoration(
                  gradient: Brand.gradient,
                  borderRadius: BorderRadius.circular(14)),
              child: const Icon(Icons.music_note, color: Colors.white),
            ),
            const SizedBox(width: 14),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(track ?? 'Toque para escolher uma música',
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: const TextStyle(
                          fontSize: 17, fontWeight: FontWeight.w700)),
                  const SizedBox(height: 2),
                  Text(track == null ? 'sua biblioteca' : 'tocando agora',
                      style: const TextStyle(fontSize: 13, color: Brand.muted)),
                ],
              ),
            ),
          ],
        ),
      );
}

/// Botão grande Turbo com halo e equalizador animado.
class _TurboButton extends StatelessWidget {
  const _TurboButton({required this.on, required this.onTap});
  final bool on;
  final VoidCallback onTap;
  @override
  Widget build(BuildContext context) => GestureDetector(
        onTap: onTap,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 250),
          width: 230,
          height: 230,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            gradient: on ? Brand.gradient : null,
            color: on ? null : Brand.surface2,
            boxShadow: on
                ? [
                    BoxShadow(
                        color: Brand.primary2.withOpacity(0.55),
                        blurRadius: 60,
                        spreadRadius: 4)
                  ]
                : null,
          ),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              SizedBox(
                  height: 80,
                  child: EqualizerBars(
                      active: on,
                      color: on ? Colors.white : Brand.muted)),
              const SizedBox(height: 12),
              Text('TURBO ${on ? "ON" : "OFF"}',
                  style: TextStyle(
                      fontSize: 22,
                      fontWeight: FontWeight.w800,
                      letterSpacing: 1,
                      color: on ? Colors.white : Brand.muted)),
            ],
          ),
        ),
      );
}

/// Cinco barras de equalizador que pulsam quando o Turbo está ligado.
class EqualizerBars extends StatefulWidget {
  const EqualizerBars({super.key, required this.active, required this.color});
  final bool active;
  final Color color;
  @override
  State<EqualizerBars> createState() => _EqualizerBarsState();
}

class _EqualizerBarsState extends State<EqualizerBars>
    with SingleTickerProviderStateMixin {
  late final AnimationController _c =
      AnimationController(vsync: this, duration: const Duration(milliseconds: 900))
        ..repeat(reverse: true);
  static const _phases = [0.0, 0.5, 0.25, 0.7, 0.15];
  static const _mins = [0.45, 0.30, 0.25, 0.35, 0.50];

  @override
  void dispose() {
    _c.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => AnimatedBuilder(
        animation: _c,
        builder: (_, __) => Row(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: List.generate(5, (i) {
            final t = widget.active
                ? (0.5 + 0.5 * (1 - (2 * ((_c.value + _phases[i]) % 1) - 1).abs()))
                : _mins[i];
            return Padding(
              padding: const EdgeInsets.symmetric(horizontal: 5),
              child: Container(
                width: 14,
                height: 16 + 60 * t,
                decoration: BoxDecoration(
                    color: widget.color,
                    borderRadius: BorderRadius.circular(7)),
              ),
            );
          }),
        ),
      );
}

class _LoudBadge extends StatelessWidget {
  const _LoudBadge();
  @override
  Widget build(BuildContext context) => Container(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        decoration: BoxDecoration(
          color: Brand.accent.withOpacity(0.10),
          borderRadius: BorderRadius.circular(20),
          border: Border.all(color: Brand.accent.withOpacity(0.4)),
        ),
        child: const Text('+6 dB de volume · sem distorcer',
            style: TextStyle(
                color: Brand.accent, fontWeight: FontWeight.w700, fontSize: 13)),
      );
}

class _Presets extends StatelessWidget {
  const _Presets({required this.selected, required this.onSelect});
  final DspPreset selected;
  final ValueChanged<DspPreset> onSelect;
  @override
  Widget build(BuildContext context) => Wrap(
        spacing: 10,
        runSpacing: 10,
        alignment: WrapAlignment.center,
        children: [
          for (final p in DspPreset.values)
            if (p != DspPreset.bypass)
              _Chip(
                  label: p.label,
                  selected: selected == p,
                  onTap: () => onSelect(p)),
        ],
      );
}

class _Chip extends StatelessWidget {
  const _Chip(
      {required this.label, required this.selected, required this.onTap});
  final String label;
  final bool selected;
  final VoidCallback onTap;
  @override
  Widget build(BuildContext context) => GestureDetector(
        onTap: onTap,
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 22, vertical: 11),
          decoration: BoxDecoration(
            gradient: selected ? Brand.gradient : null,
            color: selected ? null : Brand.surface,
            borderRadius: BorderRadius.circular(22),
            border: selected ? null : Border.all(color: Brand.surface2),
          ),
          child: Text(label,
              style: TextStyle(
                  fontWeight: FontWeight.w700,
                  color: selected ? Colors.white : const Color(0xFFC8C8D2))),
        ),
      );
}

class _MultiDeviceLink extends StatelessWidget {
  const _MultiDeviceLink();
  @override
  Widget build(BuildContext context) => TextButton.icon(
        onPressed: () {
          // Fase 2: abrir fluxo de pareamento multi-celular.
        },
        icon: const Icon(Icons.add, color: Brand.accent, size: 18),
        label: const Text('tocar junto com outro celular',
            style: TextStyle(
                color: Brand.accent, fontWeight: FontWeight.w700)),
      );
}

class _PlayerControls extends StatelessWidget {
  const _PlayerControls({required this.playing, required this.onPlay});
  final bool playing;
  final VoidCallback onPlay;
  @override
  Widget build(BuildContext context) => Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Icon(Icons.skip_previous, color: Brand.muted, size: 34),
          const SizedBox(width: 28),
          GestureDetector(
            onTap: onPlay,
            child: Container(
              width: 84,
              height: 84,
              decoration: const BoxDecoration(
                  color: Colors.white, shape: BoxShape.circle),
              child: Icon(playing ? Icons.pause : Icons.play_arrow,
                  color: Brand.bg, size: 44),
            ),
          ),
          const SizedBox(width: 28),
          const Icon(Icons.skip_next, color: Brand.muted, size: 34),
        ],
      );
}
