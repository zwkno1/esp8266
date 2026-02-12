#include <mpu.h>

static MPU6050 mpu;

bool MPUClass::begin() {
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
    return false;
  }
  mpu.setIntI2CMasterEnabled(false);  // Disable I2C master mode

  mpu.setXAccelOffset(0);  // Set your accelerometer offset for axis X
  mpu.setYAccelOffset(0);  // Set your accelerometer offset for axis Y
  mpu.setZAccelOffset(0);  // Set your accelerometer offset for axis Z
  mpu.setXGyroOffset(0);   // Set your gyro offset for axis X
  mpu.setYGyroOffset(0);   // Set your gyro offset for axis Y
  mpu.setZGyroOffset(0);   // Set your gyro offset for axis Z

  MPU.update();
  MPU.debug();
  return true;
}

void MPUClass::update() { mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); }

const char* MPUClass::to_string() const {
  static char buf[128];
  snprintf(buf, sizeof(buf), "%d\t%d\t%d\t%d\t%d\t%d", ax, ay, az, gx, gy, gz);
  return buf;
}

void MPUClass::debug() const {
  Serial.print("IMUClass:\t");
  Serial.println(to_string());
}

MPUClass MPU;