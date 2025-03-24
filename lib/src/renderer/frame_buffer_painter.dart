import 'dart:ui';

import 'package:flutter/cupertino.dart' hide Image;


class FrameBufferPainter extends CustomPainter {

  final Image? image;

  FrameBufferPainter({super.repaint, required this.image});

  @override
  void paint(Canvas canvas, Size size) {

    final image = this.image;

    if (image != null) {

      final paint = Paint()..color = const Color(0xFF000000);

      canvas
        ..drawRect(Rect.fromLTWH(0, 0, size.width, size.height), paint)
        ..drawImageRect(
            image,
            const Rect.fromLTWH(0, 0, 160, 144),
            Rect.fromLTWH(
              0,
              0,
              size.width,
              size.height,
            ),
            paint,
        );

    }

  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return true;
  }
}
