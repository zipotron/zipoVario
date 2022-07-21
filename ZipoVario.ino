#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <TimerOne.h>

#define DATA_BUFFER_SIZE 3

#define TASK_DATA_TICKS 30 //100 system ticks is 1 second
#define TASK_BUZZER_TICKS 1
#define TASK_MOTOR_TICKS 30

#define TIMMER_BUZZER 50
#define BASE_TONE 1500
#define BASE_POWER 100
#define MAX_POWER 200
#define SENSIBILITY 0.1 // Speed m/s

Adafruit_BMP280 bmp; // I2C
int motorPin = 5; //motor transistor is connected to pin 5
const int buzzerPin = 2  ; //buzzer to arduino pin 2
float pressure_sea_level = 1013.25;
float alt_data[DATA_BUFFER_SIZE];
float filter_altitude=0, vertical_speed=0;
int buzzer=0, motor=0;
volatile int tone_buzzer=BASE_TONE; // Send 1KHz sound signal...
volatile int power_motor=BASE_POWER;

void setup() {
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));

  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  pinMode(motorPin, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Timer1.initialize(3333);//Now call 1/300 seconds. Interrupt call every second with value 1000000
  Timer1.attachInterrupt(scheduler);
}

void loop() {
  static int counter=0;
  static float raw_altitude=0;
  float aux=bmp.readAltitude(pressure_sea_level);
  if(aux != raw_altitude){
    if(counter==DATA_BUFFER_SIZE)
      counter=0;
    
    raw_altitude = aux;
    alt_data[counter]=raw_altitude;
    counter++;
  }
  
  if(buzzer)
    tone(buzzerPin, tone_buzzer);
  else
    noTone(buzzerPin);
  if(motor){
    if(power_motor<MAX_POWER)analogWrite(motorPin, power_motor); //vibrate
    else analogWrite(motorPin, MAX_POWER);
  }else
    analogWrite(motorPin, 0);
}

void data_filter(){
  float aux=0;
  for(int c=0; c<DATA_BUFFER_SIZE; c++){
    //Serial.print(alt_data[c]);Serial.print("-");
    aux += alt_data[c];
  }
  filter_altitude = aux / DATA_BUFFER_SIZE;
}

void scheduler(){
  static int counter=0;
  counter++;
  switch(counter){
    case 1:
      task_data_collector();
      break;
    case 2:
      task_buzzer();
      break;
    case 3:
      task_motor();
      counter=0;
  }
}
void task_data_collector(){
  static int counter=0;
  counter++;
  if(counter==TASK_DATA_TICKS){
    float aux = filter_altitude;
    data_filter();
    vertical_speed = (filter_altitude - aux) * (100 / TASK_DATA_TICKS);
    /*Serial.print(F("Filter altitude = "));
    Serial.print(filter_altitude);
    Serial.println(" m");
    Serial.print(F("vertical_speed = "));
    Serial.print(vertical_speed);
    Serial.println(" m/s");*/
    counter=0;
  }
}

void task_buzzer(){
  static int counter=0;
  static int timmer_buzzer=0;
  counter++;
  if(vertical_speed>0)tone_buzzer+=15*vertical_speed;
  else tone_buzzer+=5*vertical_speed;
  if(counter==TASK_BUZZER_TICKS){
    counter=0;
    timmer_buzzer++;
    if(timmer_buzzer == TIMMER_BUZZER){
      tone_buzzer=BASE_TONE;
      timmer_buzzer=0;
      if(buzzer)buzzer=0;
      else buzzer=1;
    }
  }
  if((vertical_speed > -SENSIBILITY) && (vertical_speed < SENSIBILITY)) buzzer=0;
}

void task_motor(){
  static int counter=0;
  counter++;
  if(vertical_speed > SENSIBILITY){
    power_motor = BASE_POWER + (vertical_speed * 50);
    if(counter>=TASK_MOTOR_TICKS - (vertical_speed * 10)){
      if(motor)motor=0;
      else motor=1;
      counter=0;
    }
  }else motor = 0;
}
