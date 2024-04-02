#include<Wire.h>
const int MPU_addr=0x68; // Accelerometer's I2C address
const int accCollectionLimit=1000; // Defines a hard cap used for resetting the acceleration array once it reaches its limit. 1000 points = +-25 compressions
const int compLimit=25; // defines the amount of compressions necessary for the compression frequency to be calculated
bool compStart=0; // Tracks start of compressions
float acceleration[accCollectionLimit]={}; // acceleration values collected over time.
float accTrend=0, velTrend=0; // trend is used for detrending
float compressionDepth=0; // stores depth of compression 
int i=0; // used for passing values to the acceleration array
int riseCounter=0,compCounter=0; // riseCounter tracks downwars movement of acceleration, compCounter tracks amount of compressions made
unsigned long compStartTime=0, compEndTime=0; // compStartTime tracks the instant of time of the first compression, compEndTime tracks the instant of time after a specific number of compressions defined on compressionCount()
unsigned long time[accCollectionLimit]={}; // acceleration values collected over time.
double compFreq=0; // compFreq tracks the average amount of compressions per minute 

void setup(){
  Wire.begin(); // Starts I2C communication
  Wire.beginTransmission(MPU_addr); 
  Wire.write(0x6B); 
  Wire.write(0); // Wakes up the MPU 6050 
  Wire.endTransmission(true);

  // Sensor sensitivity config
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1C);                  
  Wire.write(0x10);                  // Defines the measurement scale as +/- 8g
  Wire.endTransmission(true);

  Serial.begin(115200); // Starts serial communication
  Serial.println("Programa iniciado, 5 segundos até cálculo de trend");
  delay(5000);

  // Measuring the average accelerometer readings during rest.
  int i=0;
  double accSum=0;
  do {
    i++;
    accSum += collectAcc();
  }while(millis()<10000);
  accTrend = accSum/i;

  Serial.print("Trend calculada:"); Serial.println(accTrend); // For initial test purposes only
}

void loop(){  
  acceleration[i]= collectAcc()-accTrend;
  time[i]=millis();
  compressionCount(); // Tracks compressions based on collected and detrended acceleration values
  i++;
  if(i>=(accCollectionLimit-1)) {
    i=0;
    compCounter=0;
    riseCounter=0;
    //compressionDepth = depthMeasure();
  } 
  
  
  //Serial.print(millis()); Serial.print(" ; "); Serial.print(acc); Serial.print(" ; "); Serial.print(compCounter); Serial.print(" ; "); Serial.print(compPeriod); Serial.print(" ; "); Serial.println(compFreq);
  
  delay(7);
}

float collectAcc() {
  int16_t rawAcc; // Stores acceleration from the sensor
  float gAcc; // Stores the acceleration in m/s²
  
  Wire.beginTransmission(MPU_addr); // Transmission start
  Wire.write(0x3F); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,2,true); // The only value being collected is the Z acceleration, so only 2 registers are read
  rawAcc=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  gAcc=(rawAcc/4096.0)*9.80665; 
  
  return gAcc;
}

void compressionCount() {
  float oldAcc = (i>0) ? acceleration[i-1] : acceleration[accCollectionLimit-1]; 
  float newAcc=acceleration[i];

  if(newAcc>oldAcc && riseCounter>3) { //Tracks the lowest point of acceleration
    riseCounter=0;
    compCounter++;
    if(compCounter==1){ // aumentar as condições de compcounter para evitar que no começo do programa ele pegue movimentação e interprete como compressão
      compStartTime=millis();
    }
    if(compCounter==compLimit){
      compEndTime=millis();
      compCounter=0;
      compStart=1;
      double compPeriod = (double)(compEndTime - compStartTime); //testar se precisa ser um double ou se pode ser outro valor
      compFreq = (double)(600000/compPeriod);
    }
  } 

  if(newAcc<oldAcc && abs(newAcc)>1) { // Tracks downwards movement as long as the acceleration is higher than 1m/s²
    riseCounter++;
  }
}

float depthMeasure() {

}