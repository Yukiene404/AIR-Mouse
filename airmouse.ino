
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <BleMouse.h>

MPU6050 mpu(0x68);
BleMouse bleMouse("AirMouse", "ESP32", 100);

#define LEFT_BUTTON_PIN  5
#define RIGHT_BUTTON_PIN 6

float offX = 0, offZ = 0;
float moveX = 0, moveY = 0;

bool mpuReady = false;

void calibrateGyro() {
  long sx = 0, sz = 0;

  Serial.println("Keep AirMouse completely still!");

  for (int i = 0; i < 500; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    sx += gx;
    sz += gz;
    delay(3);
  }

  offX = sx / 500.0;
  offZ = sz / 500.0;

  Serial.print("Offset X: ");
  Serial.println(offX);

  Serial.print("Offset Z: ");
  Serial.println(offZ);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting AirMouse...");

  Wire.begin(8, 9);
  Wire.setClock(100000);

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected!");
    mpu.setSleepEnabled(false);
    calibrateGyro();
    mpuReady = true;
  } else {
    Serial.println("MPU6050 NOT DETECTED!");
  }

  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);

  bleMouse.begin();

  Serial.println("Bluetooth ready. Pair AirMouse.");
}

void loop() {

  if (!bleMouse.isConnected() || !mpuReady) {
    delay(10);
    return;
  }

  int16_t ax, ay, az, gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float gxc = gx - offX;
  float gzc = gz - offZ;

  // Dead zone
  if (abs(gxc) < 150) gxc = 0;
  if (abs(gzc) < 150) gzc = 0;

  // Sensitivity
  float dx = -gxc / 180.0;
  float dy = -gzc / 180.0;

  // Smooth movement
  static float avgDx = 0;
  static float avgDy = 0;

  avgDx = (avgDx * 3 + dx) / 4;
  avgDy = (avgDy * 3 + dy) / 4;

  // Accumulate fractional movements
  moveX += avgDx;
  moveY += avgDy;

  int mx = constrain((int)moveX, -127, 127);
  int my = constrain((int)moveY, -127, 127);

  if (mx != 0 || my != 0) {
    bleMouse.move(mx, my);
    moveX -= mx;
    moveY -= my;
  }

  // Left click
  if (digitalRead(LEFT_BUTTON_PIN) == LOW)
    bleMouse.press(MOUSE_LEFT);
  else
    bleMouse.release(MOUSE_LEFT);

  // Right click
  if (digitalRead(RIGHT_BUTTON_PIN) == LOW)
    bleMouse.press(MOUSE_RIGHT);
  else
    bleMouse.release(MOUSE_RIGHT);

  // Debug readings
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 200) {
    Serial.print("GX: ");
    Serial.print(gxc);

    Serial.print(" | GZ: ");
    Serial.print(gzc);

    Serial.print(" | MX: ");
    Serial.print(mx);

    Serial.print(" | MY: ");
    Serial.println(my);

    lastPrint = millis();
  }

  delay(10);
}
