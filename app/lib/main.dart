import 'dart:async';
import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'audio_engine.dart';
import 'sync_service.dart';
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
  final _sync = SyncService.instance;
  bool _turbo = true;
  bool _playing = false;
  final List<String> _paths = [];
  final List<String> _names = [];
  int _index = 0;
  Timer? _endTimer;
  DspPreset _preset = DspPreset.balanced;

  String? get _currentName => _paths.isEmpty ? null : _names[_index];
  String get _subtitle =>
      _paths.isEmpty ? 'sua biblioteca' : 'faixa ${_index + 1} de ${_paths.length}';
  SyncRole _role = SyncRole.none;
  bool _stereo = false;
  int _channel = 0;
  int _followerCount = 0;
  Timer? _countTimer;

  @override
  void dispose() {
    _countTimer?.cancel();
    _endTimer?.cancel();
    super.dispose();
  }

  String? _transient; // mensagem temporária (ex.: "Procurando…")

  // Texto de status do grupo.
  String? get _groupInfo {
    if (_transient != null) return _transient;
    if (_role == SyncRole.none) return null;
    final ch = channelLabel(_channel);
    final tag = ch.isEmpty ? '' : ' · $ch';
    if (_role == SyncRole.leader) return 'Comando · $_followerCount conectado(s)$tag';
    return 'Conectado$tag';
  }

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

  /// Escolhe uma ou várias músicas (vira a fila).
  Future<void> _pickTracks() async {
    final res = await FilePicker.platform
        .pickFiles(type: FileType.audio, allowMultiple: true);
    if (res == null || res.files.isEmpty) return;
    final picked = res.files.where((f) => f.path != null).toList();
    _paths
      ..clear()
      ..addAll(picked.map((f) => f.path!));
    _names
      ..clear()
      ..addAll(picked.map((f) => f.name));
    _index = 0;
    await _engine.load(_paths[_index]);
    setState(() => _playing = false);
  }

  Future<void> _startPlayback() async {
    if (_role == SyncRole.leader) {
      await _sync.playSynced();          // líder dispara o início em todos juntos
      _channel = await _sync.channel();
    } else {
      await _engine.play();
    }
    _startEndTimer();
    setState(() => _playing = true);
  }

  Future<void> _togglePlay() async {
    if (_paths.isEmpty) return _pickTracks();
    if (_playing) {
      await _engine.pause();
      _endTimer?.cancel();
      setState(() => _playing = false);
    } else {
      await _startPlayback();
    }
  }

  Future<void> _next() async {
    if (_paths.isEmpty) return;
    if (_index < _paths.length - 1) {
      _index++;
      await _engine.load(_paths[_index]);
      await _startPlayback();
    } else {
      await _engine.pause();
      _endTimer?.cancel();
      setState(() => _playing = false);
    }
  }

  Future<void> _prev() async {
    if (_paths.isEmpty) return;
    _index = _index > 0 ? _index - 1 : 0;
    await _engine.load(_paths[_index]);
    await _startPlayback();
  }

  // Avança a fila sozinho quando a faixa termina.
  void _startEndTimer() {
    _endTimer?.cancel();
    _endTimer = Timer.periodic(const Duration(milliseconds: 500), (_) async {
      if (_playing && await _engine.isFinished()) _next();
    });
  }

  // --- multi-celular ---

  void _openSyncSheet() {
    showModalBottomSheet(
      context: context,
      backgroundColor: Brand.surface,
      shape: const RoundedRectangleBorder(
          borderRadius: BorderRadius.vertical(top: Radius.circular(24))),
      builder: (_) => _SyncSheet(
        initialStereo: _stereo,
        onCreate: (stereo) { Navigator.pop(context); _createGroup(stereo); },
        onJoin: () { Navigator.pop(context); _joinGroup(); },
      ),
    );
  }

  Future<void> _createGroup(bool stereo) async {
    _stereo = stereo;
    await _sync.setStereo(stereo);
    await _sync.createGroup();
    setState(() { _role = SyncRole.leader; _transient = null; _channel = 0; });
    _countTimer?.cancel();
    _countTimer = Timer.periodic(const Duration(seconds: 1), (_) async {
      final c = await _sync.followerCount();
      if (mounted) setState(() => _followerCount = c);
    });
  }

  Future<void> _joinGroup() async {
    setState(() => _transient = 'Procurando na rede…');
    final res = await _sync.joinGroup();
    if (res['ok'] == true) {
      final ch = await _sync.channel();
      setState(() { _role = SyncRole.follower; _transient = null; _channel = ch; });
    } else {
      setState(() => _transient = 'Nenhum grupo encontrado');
    }
  }

  Future<void> _showDevices() async {
    final names = await _sync.followerNames();
    if (!mounted) return;
    showDialog(
      context: context,
      builder: (_) => AlertDialog(
        backgroundColor: Brand.surface,
        title: const Text('Aparelhos conectados'),
        content: names.isEmpty
            ? const Text('Nenhum ainda. Peça para abrirem o app e tocarem '
                '"Entrar num grupo".')
            : Column(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  for (final n in names)
                    ListTile(
                      contentPadding: EdgeInsets.zero,
                      leading: const Icon(Icons.smartphone, color: Brand.accent),
                      title: Text(n),
                    ),
                ],
              ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: const Text('Fechar')),
        ],
      ),
    );
  }

  Future<void> _leaveGroup() async {
    _countTimer?.cancel();
    await _sync.leave();
    setState(() {
      _role = SyncRole.none; _transient = null; _channel = 0; _followerCount = 0;
    });
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
              _NowPlaying(track: _currentName, subtitle: _subtitle, onTap: _pickTracks),
              const Spacer(),
              _TurboButton(on: _turbo, onTap: _toggleTurbo),
              const SizedBox(height: 18),
              const _LoudBadge(),
              const SizedBox(height: 24),
              _Presets(selected: _preset, onSelect: _selectPreset),
              const SizedBox(height: 20),
              _MultiDeviceLink(
                  info: _groupInfo,
                  active: _role != SyncRole.none || _transient != null,
                  onTap: _openSyncSheet,
                  onLeave: _leaveGroup,
                  onInfoTap: _role == SyncRole.leader ? _showDevices : null),
              const Spacer(),
              _PlayerControls(
                  playing: _playing, onPlay: _togglePlay, onPrev: _prev, onNext: _next),
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
  const _NowPlaying({required this.track, required this.subtitle, required this.onTap});
  final String? track;
  final String subtitle;
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
                  Text(track ?? 'Toque para escolher músicas',
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: const TextStyle(
                          fontSize: 17, fontWeight: FontWeight.w700)),
                  const SizedBox(height: 2),
                  Text(subtitle,
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
  const _MultiDeviceLink(
      {this.info,
      required this.active,
      required this.onTap,
      required this.onLeave,
      this.onInfoTap});
  final String? info;
  final bool active;
  final VoidCallback onTap;
  final VoidCallback onLeave;
  final VoidCallback? onInfoTap;

  @override
  Widget build(BuildContext context) {
    if (active) {
      return Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Icon(Icons.podcasts, color: Brand.accent, size: 18),
          const SizedBox(width: 8),
          Flexible(
            child: GestureDetector(
              onTap: onInfoTap,
              child: Text(info ?? 'Em grupo',
                  overflow: TextOverflow.ellipsis,
                  style: const TextStyle(
                      color: Brand.accent, fontWeight: FontWeight.w700)),
            ),
          ),
          TextButton(onPressed: onLeave, child: const Text('sair')),
        ],
      );
    }
    return TextButton.icon(
      onPressed: onTap,
      icon: const Icon(Icons.add, color: Brand.accent, size: 18),
      label: const Text('tocar junto com outro celular',
          style: TextStyle(color: Brand.accent, fontWeight: FontWeight.w700)),
    );
  }
}

/// Folha "tocar junto": toggle de estéreo + criar/entrar.
class _SyncSheet extends StatefulWidget {
  const _SyncSheet(
      {required this.initialStereo, required this.onCreate, required this.onJoin});
  final bool initialStereo;
  final ValueChanged<bool> onCreate;
  final VoidCallback onJoin;
  @override
  State<_SyncSheet> createState() => _SyncSheetState();
}

class _SyncSheetState extends State<_SyncSheet> {
  late bool _stereo = widget.initialStereo;
  @override
  Widget build(BuildContext context) => Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            const Text('Tocar junto com outros celulares',
                textAlign: TextAlign.center,
                style: TextStyle(fontSize: 18, fontWeight: FontWeight.w800)),
            const SizedBox(height: 6),
            const Text('Todos na mesma Wi-Fi. Um vira o comando, os outros entram.',
                textAlign: TextAlign.center,
                style: TextStyle(color: Brand.muted, fontSize: 13)),
            const SizedBox(height: 16),
            SwitchListTile(
              value: _stereo,
              onChanged: (v) => setState(() => _stereo = v),
              activeColor: Brand.accent,
              contentPadding: EdgeInsets.zero,
              title: const Text('Modo estéreo',
                  style: TextStyle(fontWeight: FontWeight.w700)),
              subtitle: const Text('cada celular vira um lado (esquerdo/direito)',
                  style: TextStyle(color: Brand.muted, fontSize: 12)),
            ),
            const SizedBox(height: 8),
            FilledButton.icon(
              onPressed: () => widget.onCreate(_stereo),
              icon: const Icon(Icons.podcasts),
              label: const Text('Criar grupo (sou o comando)'),
              style: FilledButton.styleFrom(minimumSize: const Size(0, 52)),
            ),
            const SizedBox(height: 12),
            OutlinedButton.icon(
              onPressed: widget.onJoin,
              icon: const Icon(Icons.wifi_find),
              label: const Text('Entrar num grupo'),
              style: OutlinedButton.styleFrom(minimumSize: const Size(0, 52)),
            ),
          ],
        ),
      );
}

class _PlayerControls extends StatelessWidget {
  const _PlayerControls(
      {required this.playing,
      required this.onPlay,
      required this.onPrev,
      required this.onNext});
  final bool playing;
  final VoidCallback onPlay;
  final VoidCallback onPrev;
  final VoidCallback onNext;
  @override
  Widget build(BuildContext context) => Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          IconButton(
              onPressed: onPrev,
              icon: const Icon(Icons.skip_previous, color: Brand.muted, size: 34)),
          const SizedBox(width: 16),
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
          const SizedBox(width: 16),
          IconButton(
              onPressed: onNext,
              icon: const Icon(Icons.skip_next, color: Brand.muted, size: 34)),
        ],
      );
}
