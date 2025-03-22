import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart' as ffi;
import 'dart:typed_data' show Uint32List;
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:gameboy/src/bridge/game_boy_ffi.dart' as gb;
import 'package:gameboy/src/renderer/frame_buffer_painter.dart';


const frameDuration = Duration(milliseconds: 166);

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {

  gb.Gameboy? gameboy;
  final frameBuffer = ValueNotifier<Uint32List>(Uint32List(160 * 144));
  bool _running = false;

  @override
  void initState() {
    super.initState();
    init();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Game Boy'), centerTitle: true),
      backgroundColor: Colors.grey,
      body: Center(
        child: AspectRatio(
          aspectRatio: 160 / 144,
          child: ValueListenableBuilder(
            valueListenable: frameBuffer,
            builder: (_, value, _) {
              return CustomPaint(
                painter: FrameBufferPainter(
                  frameBuffer: value,
                ),
              );
            },
          ),
        ),
      ),
    );
  }

  Future<void> init() async {
    final dylib = ffi.DynamicLibrary.open('libgameboy.dylib');

    gameboy = gb.Gameboy(dylib);

    gameboy?.gb_init();

    await loadRom();

    startEmulatorLoop();

  }

  Future<void> loadRom() async {

    final romData = await rootBundle.load('assets/roms/Tetris.gb');
    final bytes = romData.buffer.asUint8List();
    final romSize = bytes.length;

    final ptr = ffi.malloc<ffi.Uint8>(romSize);

    ptr.asTypedList(romSize).setAll(0, bytes);
    gameboy?.gb_load_rom(ptr, romSize);

    ffi.malloc.free(ptr);
  }

  void startEmulatorLoop() async {

    if (_running) return;
    _running = true;

    final fbPtr = gameboy?.gb_get_framebuffer();
    if (fbPtr == null) {
      print("Framebuffer is null");
      return;
    }

    final fb = fbPtr.asTypedList(160 * 144);

    while (_running) {

      gameboy?.gb_step_frame();
      frameBuffer.value = fb;

      await Future.delayed(frameDuration); // 16 мс ≈ 60 FPS
    }

  }

  void stopEmulatorLoop() {
    _running = false;
  }

}
