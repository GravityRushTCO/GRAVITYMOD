package com.android.support;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.view.View;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class ShootingStarView extends View {

    private static final int STAR_COUNT = 60;
    private static final int SPARK_COUNT = 40;
    private final List<Star> stars = new ArrayList<>();
    private final List<Spark> sparks = new ArrayList<>();
    private final Paint paint = new Paint();
    private final Paint glowPaint = new Paint();
    private final Random random = new Random();

    // Red and blue fireworks palette
    private final int[] redColors = {
            Color.parseColor("#FF0000"),
            Color.parseColor("#FF3300"),
            Color.parseColor("#FF0055"),
            Color.parseColor("#FF6666"),
    };
    private final int[] blueColors = {
            Color.parseColor("#0055FF"),
            Color.parseColor("#0033FF"),
            Color.parseColor("#3388FF"),
            Color.parseColor("#00AAFF"),
    };

    public ShootingStarView(Context context) {
        super(context);
        init();
    }

    public ShootingStarView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.FILL);
        glowPaint.setAntiAlias(true);
        glowPaint.setStyle(Paint.Style.STROKE);
        for (int i = 0; i < STAR_COUNT; i++) {
            stars.add(new Star(true));
        }
        for (int i = 0; i < SPARK_COUNT; i++) {
            sparks.add(new Spark(true));
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int width = getWidth();
        int height = getHeight();
        if (width == 0 || height == 0) return;

        // Draw sparkling static stars (small twinkling dots)
        for (Spark spark : sparks) {
            spark.update();
            spark.draw(canvas, paint);
        }

        // Draw shooting stars
        for (Star star : stars) {
            star.update(width, height);
            star.draw(canvas, paint, glowPaint);
        }

        invalidate();
    }

    // Small twinkling static dots
    private class Spark {
        float x, y;
        float size;
        int color;
        float alpha;
        float alphaDir;
        int w = 1000, h = 2000;

        Spark(boolean randomStart) {
            reset(randomStart);
        }

        void reset(boolean randomStart) {
            x = random.nextFloat() * w;
            y = random.nextFloat() * h;
            size = 0.5f + random.nextFloat() * 2f;
            boolean isRed = random.nextBoolean();
            color = isRed ? redColors[random.nextInt(redColors.length)] : blueColors[random.nextInt(blueColors.length)];
            alpha = random.nextFloat();
            alphaDir = 0.01f + random.nextFloat() * 0.03f;
        }

        void update() {
            alpha += alphaDir;
            if (alpha >= 1f) { alpha = 1f; alphaDir = -alphaDir; }
            if (alpha <= 0f) { alpha = 0f; alphaDir = -alphaDir; reset(true); }
        }

        void draw(Canvas canvas, Paint p) {
            p.setColor(color);
            p.setAlpha((int)(alpha * 200));
            canvas.drawCircle(x, y, size, p);
        }
    }

    // Shooting star with trail and glow
    private class Star {
        float x, y;
        float speedX, speedY;
        float size;
        float length;
        int color;
        float alpha;
        boolean isRed;

        Star(boolean randomStart) {
            reset(randomStart);
        }

        void reset(boolean randomStart) {
            // Alternate red and blue
            isRed = random.nextBoolean();
            int[] palette = isRed ? redColors : blueColors;
            color = palette[random.nextInt(palette.length)];

            float speed = 4f + random.nextFloat() * 12f;
            // Direction: mostly diagonal (top-right to bottom-left or top to bottom)
            float angle = (float)(Math.PI * 0.3 + random.nextFloat() * Math.PI * 0.4); // 54° to 126°
            speedX = -speed * (float)Math.cos(angle);
            speedY = speed * (float)Math.sin(angle);

            size = 1.5f + random.nextFloat() * 3f;
            length = size * (15f + random.nextFloat() * 30f);
            alpha = 0.6f + random.nextFloat() * 0.4f;

            int w = getWidth() > 0 ? getWidth() : 400;
            int h = getHeight() > 0 ? getHeight() : 800;

            if (randomStart) {
                x = random.nextFloat() * w;
                y = random.nextFloat() * h;
            } else {
                // Spawn from right edge or top edge
                if (random.nextBoolean()) {
                    x = w + length;
                    y = random.nextFloat() * h;
                } else {
                    x = random.nextFloat() * w + w * 0.3f;
                    y = -length;
                }
            }
        }

        void update(int width, int height) {
            x += speedX;
            y += speedY;
            if (x < -length * 2 || y > height + length * 2) {
                reset(false);
            }
        }

        void draw(Canvas canvas, Paint p, Paint glow) {
            // Normalize direction for trail
            float len = (float)Math.sqrt(speedX*speedX + speedY*speedY);
            float nx = -speedX / len;
            float ny = -speedY / len;
            float tailX = x + nx * length;
            float tailY = y + ny * length;

            // Draw glow trail (wider, transparent)
            glow.setColor(color);
            glow.setAlpha((int)(alpha * 80));
            glow.setStrokeWidth(size * 3f);
            canvas.drawLine(x, y, tailX, tailY, glow);

            // Draw main trail
            p.setColor(color);
            p.setAlpha((int)(alpha * 200));
            p.setStrokeWidth(size);
            glow.setStrokeWidth(size);
            glow.setAlpha((int)(alpha * 200));
            canvas.drawLine(x, y, tailX, tailY, glow);

            // Draw bright head
            p.setAlpha(255);
            canvas.drawCircle(x, y, size * 2f, p);

            // Draw inner white core
            p.setColor(Color.WHITE);
            p.setAlpha(230);
            canvas.drawCircle(x, y, size * 0.8f, p);
        }
    }
}
