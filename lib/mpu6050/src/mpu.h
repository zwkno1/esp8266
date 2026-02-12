#include <MPU6050.h>

struct MPUClass {
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;

  bool begin();

  void update();

  const char* to_string() const;

  void debug() const;
};

extern MPUClass MPU;