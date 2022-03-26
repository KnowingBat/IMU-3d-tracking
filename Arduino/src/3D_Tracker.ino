#include <Wire.h>

#define IMU_I2C_ADDRESS 0x68
#define BEGIN 0x3B
#define PWR_MGMT_1 0x6B
#define GYRO_CONV 131

typedef union acc_gyro_union
{
  struct
  {
    uint8_t accX_h;
    uint8_t accX_l;
    uint8_t accY_h;
    uint8_t accY_l;
    uint8_t accZ_h;
    uint8_t accZ_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t gyroX_h;
    uint8_t gyroX_l;
    uint8_t gyroY_h;
    uint8_t gyroY_l;
    uint8_t gyroZ_h;
    uint8_t gyroZ_l;
  } reg;
  struct 
  {
    int accX;
    int accY;
    int accZ;
    int temperature;
    int gyroX;
    int gyroY;
    int gyroZ;
  } value;
};

float accX_offset, accY_offset, accZ_offset;
float gyroX_offset, gyroY_offset, gyroZ_offset; 

float prev_roll, prev_pitch, prev_yaw;
unsigned long prev_time;

float lambda = 0.98;  

void IMU_init(int);
void IMU_read(int, uint8_t, int);
void IMU_get_val(uint8_t *);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  IMU_init(PWR_MGMT_1);
  calibrate(20);
}

void loop() {
  double dt;
  unsigned long curr_time;
  float roll, pitch, yaw;
  float accX_angle, accY_angle;
  float gyroX_angle, gyroY_angle, gyroZ_angle;

  acc_gyro_union acc_gyro;
  IMU_get_val((uint8_t *) &acc_gyro);

  curr_time = millis();
  dt = (curr_time - prev_time) / 1000.0;
  // Determinazione pulizia dato dall'offset
  float gyroX = (acc_gyro.value.gyroX - gyroX_offset)/ GYRO_CONV;
  float gyroY = (acc_gyro.value.gyroY - gyroY_offset)/ GYRO_CONV;
  float gyroZ = (acc_gyro.value.gyroZ - gyroZ_offset)/ GYRO_CONV;

  float accX = (acc_gyro.value.accX);
  float accY = (acc_gyro.value.accY);
  float accZ = (acc_gyro.value.accZ);

  // Determinazione angoli da giroscopio e da accelerometro, conversione in radianti
  accX_angle = atan(accY/sqrt(pow(accX, 2)+pow(accZ, 2)))*180/PI;
  accY_angle = atan(-1*accX/sqrt(pow(accY, 2)+pow(accZ, 2)))*180/PI;
  // Integrazione gyro
  gyroX_angle = gyroX*dt + prev_roll;
  gyroY_angle = gyroY*dt + prev_pitch;
  gyroZ_angle = gyroZ*dt + prev_yaw;

  // Computo filtraggio
  roll = lambda*(gyroX_angle) + (1.0 - lambda)*accX_angle;
  pitch = lambda*(gyroY_angle) + (1.0 - lambda)*accY_angle;
  yaw = lambda*(gyroZ_angle);

  prev_roll = roll;
  prev_pitch = pitch;
  prev_yaw = yaw;

  prev_time = curr_time;
  
  // Invio a seriale
  Serial.print(roll, 2);
  Serial.print(F(","));
  Serial.print(pitch, 2);
  Serial.print(F(","));
  Serial.print(yaw, 2);
  Serial.println();

  //delay(5);
}

void IMU_init(int start){
  Wire.beginTransmission(IMU_I2C_ADDRESS);
  Wire.write(start);
  Wire.write(0);
  Wire.endTransmission(true);
}

void IMU_read(int start, uint8_t *buffer, int size){

  int i = 0;
  Wire.beginTransmission(IMU_I2C_ADDRESS);
  Wire.write(start);
  Wire.endTransmission(false);

  Wire.requestFrom(IMU_I2C_ADDRESS, size, true);
  while(Wire.available() && i < size){
    buffer[i++] = Wire.read();
  }

}

void IMU_get_val(uint8_t *buffer){
  
  acc_gyro_union* acc_gyro = (acc_gyro_union *) buffer;
  IMU_read(BEGIN, (uint8_t *)acc_gyro, sizeof(*acc_gyro));
  uint8_t s;
  #define SWAP(a,b) s = a; a = b; b = s;

  SWAP ((*acc_gyro).reg.accX_h, (*acc_gyro).reg.accX_l);
  SWAP ((*acc_gyro).reg.accY_h, (*acc_gyro).reg.accY_l);
  SWAP ((*acc_gyro).reg.accZ_h, (*acc_gyro).reg.accZ_l);
  SWAP ((*acc_gyro).reg.t_h, (*acc_gyro).reg.t_l);
  SWAP ((*acc_gyro).reg.gyroX_h, (*acc_gyro).reg.gyroX_l);
  SWAP ((*acc_gyro).reg.gyroY_h, (*acc_gyro).reg.gyroY_l);
  SWAP ((*acc_gyro).reg.gyroZ_h, (*acc_gyro).reg.gyroZ_l);

}

void calibrate(int sample){
  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;
  acc_gyro_union acc_gyro;

  IMU_get_val((uint8_t *) &acc_gyro);

  for(int i=0; i< sample; ++i){
    IMU_get_val((uint8_t *) &acc_gyro);
    accX += acc_gyro.value.accX;
    accY += acc_gyro.value.accY;
    accZ += acc_gyro.value.accZ; 
    gyroX += acc_gyro.value.gyroX;
    gyroY += acc_gyro.value.gyroY;
    gyroZ += acc_gyro.value.gyroZ;
  }

  accX_offset = accX / sample;
  accY_offset = accY / sample;
  accZ_offset = accZ / sample;
  gyroX_offset = gyroX / sample;
  gyroY_offset = gyroY / sample;
  gyroZ_offset = gyroZ / sample;

}
