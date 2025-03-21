import 'dart:ffi' as ffi;
import 'dart:ffi';
import 'dart:typed_data' show Uint32List;
import 'dart:ui' as ui;

import 'package:ffi/ffi.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:gameboy/src/bridge/game_boy_ffi.dart' as gb;
import 'package:gameboy/src/renderer/game_boy_screen_painter.dart';

import 'game_boy_screen_painter_image.dart' show GameBoyScreenPainterImage;

const frameDuration = Duration(milliseconds: 16);

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {

  gb.NativeLibrary? gbEmulator;
  ValueNotifier buffer = ValueNotifier<Uint32List?>(Uint32List(160 * 144));
  ui.Image? image;

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
            valueListenable: buffer,
            builder: (_, value, _) {
              return CustomPaint(
                painter: GameBoyScreenPainter(
                    buffer: value
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
    gbEmulator = gb.NativeLibrary(dylib);

    final romData = await rootBundle.load('assets/roms/Tetris.gb');
    final bytes = romData.buffer.asUint8List();

    loadRom(bytes);
    startEmulatorLoop();
  }

  void loadRom(Uint8List romBytes) {
    final romLength = romBytes.length;
    final ptr = malloc<Uint8>(romLength);
    final byteList = ptr.asTypedList(romLength);
    byteList.setAll(0, romBytes);
    gbEmulator?.memory_load_rom(ptr, romLength);
    malloc.free(ptr);
  }

  void startEmulatorLoop() async {

    gbEmulator?.cpu_init();
    // gbEmulator?.ppu_init();

    while (true) {

      final cycles = gbEmulator?.cpu_step();

      for (int i = 0; i < (cycles ?? 0); i++) {
        gbEmulator?.ppu_step();
      }

      final fbPtr = gbEmulator?.ppu_get_framebuffer();
      final fb32 = fbPtr?.asTypedList(160 * 144);
      // final fb8 = fb32?.buffer.asUint8List() ?? Uint8List(160 * 144 * 4);

      // ui.decodeImageFromPixels(
      //   fb8,
      //   160,
      //   144,
      //   ui.PixelFormat.rgba8888,
      //   (img) {
      //     image = img;
      //     print('image $image');
      //   },
      // );

      buffer.value = fb32;

      await Future.delayed(frameDuration);

    }
  }
}
