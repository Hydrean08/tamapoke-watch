package com.tamapoke.watch;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

final class Utf8TextRasterizer {
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);

    Utf8TextRasterizer() {
        paint.setColor(Color.WHITE);
    }

    int[] rasterize(String text, int size, boolean drawPixels) {
        if (text == null || text.isEmpty()) return new int[]{0, 0, 0};
        paint.setTextSize(15.5f * Math.max(1, size));
        Paint.FontMetricsInt fm = paint.getFontMetricsInt();
        int w = Math.max(1, (int) Math.ceil(paint.measureText(text)));
        int h = Math.max(1, fm.descent - fm.ascent);
        if (!drawPixels) {
            return new int[]{w, h, fm.ascent};
        }
        Bitmap bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(bmp);
        c.drawText(text, 0f, -fm.ascent, paint);
        int[] pixels = new int[w * h];
        bmp.getPixels(pixels, 0, w, 0, 0, w, h);
        bmp.recycle();
        int[] out = new int[3 + pixels.length];
        out[0] = w;
        out[1] = h;
        out[2] = fm.ascent;
        for (int i = 0; i < pixels.length; i++) {
            out[3 + i] = Color.alpha(pixels[i]);
        }
        return out;
    }
}
