#include <Arduino.h>
#include <TFT_eSPI.h>

// Joystick pins
constexpr int JOY_X  = 1;
constexpr int JOY_Y  = 2;
constexpr int JOY_SW = 4;

// Screen is used in landscape: 320 x 240
constexpr int W = 320;
constexpr int H = 240;

// Joystick tuning
constexpr int DEADZONE   = 300;  // ignore small movements around centre
constexpr int MAX_SPEED  = 6;    // pixels per frame at full deflection
constexpr uint32_t LONG_PRESS_MS = 800;

TFT_eSPI tft;

// 1 bit per pixel copy of the drawing (9.6 KB) so the cursor can be
// erased without wiping out the lines underneath it.
uint8_t canvas[W * H / 8];

int centreX = 2048, centreY = 2048;
int curX = W / 2, curY = H / 2;
bool penDown = true;

bool getPx(int x, int y) {
  int i = y * W + x;
  return canvas[i >> 3] & (1 << (i & 7));
}

void setPx(int x, int y) {
  int i = y * W + x;
  canvas[i >> 3] |= (1 << (i & 7));
}

void plot(int x, int y) {
  if (x < 0 || y < 0 || x >= W || y >= H) return;
  setPx(x, y);
  tft.drawPixel(x, y, TFT_WHITE);
}

// Bresenham line, drawn to both the screen and the canvas
void drawLineBits(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  while (true) {
    plot(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

// Redraw the saved drawing in a small square (removes the old cursor)
void restoreArea(int cx, int cy, int r) {
  for (int y = cy - r; y <= cy + r; y++) {
    for (int x = cx - r; x <= cx + r; x++) {
      if (x < 0 || y < 0 || x >= W || y >= H) continue;
      tft.drawPixel(x, y, getPx(x, y) ? TFT_WHITE : TFT_BLACK);
    }
  }
}

// Green crosshair = pen down, red = pen up
void drawCursor(int cx, int cy) {
  uint16_t col = penDown ? TFT_GREEN : TFT_RED;
  tft.drawFastHLine(cx - 4, cy, 9, col);
  tft.drawFastVLine(cx, cy - 4, 9, col);
}

void clearCanvas() {
  memset(canvas, 0, sizeof(canvas));
  tft.fillScreen(TFT_BLACK);
  drawCursor(curX, curY);
}

// Movement in pixels for one axis, with a deadzone
int axisToSpeed(int raw, int centre) {
  int d = raw - centre;
  if (abs(d) < DEADZONE) return 0;
  d += (d > 0) ? -DEADZONE : DEADZONE;
  return constrain(d * MAX_SPEED / (2048 - DEADZONE), -MAX_SPEED, MAX_SPEED);
}

void handleButton() {
  static bool wasPressed = false;
  static uint32_t pressedAt = 0;
  bool pressed = digitalRead(JOY_SW) == LOW;

  if (pressed && !wasPressed) {
    pressedAt = millis();
  } else if (!pressed && wasPressed) {
    uint32_t held = millis() - pressedAt;
    if (held >= LONG_PRESS_MS) {
      clearCanvas();                 // long press: clear
    } else if (held > 30) {
      penDown = !penDown;            // short press: pen up/down
      drawCursor(curX, curY);
    }
  }
  wasPressed = pressed;
}

void setup() {
  Serial.begin(115200);
  pinMode(JOY_SW, INPUT_PULLUP);
  analogReadResolution(12);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Leave the stick untouched at start-up: this records its centre position
  delay(500);
  long sx = 0, sy = 0;
  for (int i = 0; i < 32; i++) {
    sx += analogRead(JOY_X);
    sy += analogRead(JOY_Y);
    delay(5);
  }
  centreX = sx / 32;
  centreY = sy / 32;

  drawCursor(curX, curY);
}

void loop() {
  handleButton();

  int dx = axisToSpeed(analogRead(JOY_X), centreX);
  int dy = axisToSpeed(analogRead(JOY_Y), centreY);

  if (dx != 0 || dy != 0) {
    int oldX = curX, oldY = curY;
    curX = constrain(curX + dx, 0, W - 1);
    curY = constrain(curY + dy, 0, H - 1);

    if (penDown) drawLineBits(oldX, oldY, curX, curY);
    restoreArea(oldX, oldY, 5);
    drawCursor(curX, curY);
  }

  delay(15);
}