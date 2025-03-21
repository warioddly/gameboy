import 'dart:typed_data' show Uint32List;

import 'package:flutter/cupertino.dart';

class GameBoyScreenPainter extends CustomPainter {

  final Uint32List buffer;

  GameBoyScreenPainter({super.repaint, required this.buffer});


  @override
  void paint(Canvas canvas, Size size) {

    final pixelSize = size.width / 160;

    for (int y = 0; y < 144; y++) {
      for (int x = 0; x < 160; x++) {
        final color = Color.fromRGBO(
          buffer[y * 160 + x] & 0xFF,
          (buffer[y * 160 + x] >> 8) & 0xFF,
          (buffer[y * 160 + x] >> 16) & 0xFF,
          1,
        );
        canvas.drawRect(
          Rect.fromLTWH(x * pixelSize, y * pixelSize, pixelSize, pixelSize),
          Paint()..color = color,
        );
      }
    }

  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return true;
  }
}
