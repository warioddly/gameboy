import 'dart:ui';
import 'dart:typed_data' show Uint32List;

import 'package:flutter/cupertino.dart';


class FrameBufferPainter extends CustomPainter {

  final Uint32List frameBuffer;

  FrameBufferPainter({super.repaint, required this.frameBuffer});

  @override
  void paint(Canvas canvas, Size size) {

    final paint = Paint();

    for (int i = 0; i < 160; i++) {
      for (int j = 0; j < 144; j++) {
        int rgba = frameBuffer[j * 160 + i];
        int argb = (rgba << 24) | (rgba >> 8);
        paint.color = Color(argb);
        canvas.drawRect(
          Rect.fromLTWH(i * 2.0, j * 2.0, 2.0, 2.0),
          paint,
        );
      }
    }

  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return true;
  }
}
