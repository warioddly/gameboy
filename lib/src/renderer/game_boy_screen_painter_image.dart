
import 'dart:ui' as ui;

import 'package:flutter/material.dart';

class GameBoyScreenPainterImage extends CustomPainter {

  final ui.Image? buffer;

  GameBoyScreenPainterImage({super.repaint, required this.buffer});

  @override
  void paint(Canvas canvas, Size size) {

    if (buffer == null) return;

    final pixelSize = size.width / 160;

    canvas.drawImageRect(
      buffer!,
      Rect.fromLTWH(0, 0, 160, 144),
      Rect.fromLTWH(0, 0, 160 * pixelSize, 144 * pixelSize),
      Paint(),
    );


  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return true;
  }
}
