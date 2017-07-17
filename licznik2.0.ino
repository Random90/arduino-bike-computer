#include <SPI.h>
#include <EEPROM.h>
#include "LCD_Functions.h"
#include <stdlib.h>

const PROGMEM uint16_t CIRCUMFERENCE = 2010;
const PROGMEM uint16_t NOT_RIDING_AFTER = 5000;
const PROGMEM uint16_t DISTANCE_ADRR = 10;
const PROGMEM uint16_t MAX_SPEED_ADRR = 20;

volatile unsigned long turns = 0;
volatile unsigned long  magnet_time = 0;  
volatile unsigned long  last_magnet_time = 0;

volatile unsigned long c_turns = 0;
volatile unsigned long  c_magnet_time = 0;  
volatile unsigned long  c_last_magnet_time = 0;

volatile float bike_speed;
float bike_speed2;
char _bike_speed[6];

volatile float average = 0; 
char _average[6];

volatile float distance = 0;
char _distance[8];

volatile float rotations = 0;
char _rotations[6];

volatile uint8_t cadence_count;
char _cadence_count[3];

float cad_spd = 0.00f;
char _cad_spd[4];

unsigned long ride_time;
unsigned long disp_time;
bool ride_started = false;
bool riding = false;

float full_distance = 0;
char _full_distance[7];
float read_full_distance = 0;

uint16_t savings = 0;
char _savings[4];

float max_speed = 0;
float read_max_speed = 0;
char _max_speed[6];
char _read_max_speed[6];

int serial_count = 0;
char serial_data[30];
boolean serial_dataComplete = false;

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  //pullup
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  attachInterrupt(digitalPinToInterrupt(2), rotation, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), cadence, FALLING);
  lcdBegin();
  updateDisplay();
  setContrast(40);
  clearDisplay(WHITE);

  disp_time = millis();
  EEPROM.get(DISTANCE_ADRR,read_full_distance);
  EEPROM.get(MAX_SPEED_ADRR, read_max_speed);
  if(!(read_max_speed > 0)) {
    read_max_speed = 0.00f;
  }
  if(!(read_full_distance > 0)) {
    read_full_distance = 0.00f;
  }
}

void loop() {
if (Serial.available() > 0){
  if(Serial.read()) {
      EEPROM.put(MAX_SPEED_ADRR, 0.00f);
      Serial.println("clear");
  }
    }
  
   if(millis() - magnet_time > 1000) {
      bike_speed = 0;
    }
    if(millis() - c_magnet_time > 3000){
      cadence_count = 0;
    }
  if(millis() - disp_time > 400){   
    if(bike_speed) {
      ride_time += 400;
      rotations = float(turns)/(float(ride_time)/60000);
      average = distance/(float(ride_time)/3600000);
      if(bike_speed > read_max_speed) {
        read_max_speed = bike_speed;
        EEPROM.put(MAX_SPEED_ADRR, read_max_speed);
      }
      if(bike_speed > max_speed){
        max_speed = bike_speed;
      }
    }
   
    clearDisplay(WHITE);

    //predkosc
    bike_speed2 = bike_speed;
    dtostrf(bike_speed2, 6, 3, _bike_speed);
    setStr(_bike_speed, 0, 0, WHITE);
    
    //dystans
    dtostrf(distance, 6, 2, _distance);
    setStr(_distance, 0, 8, BLACK);

    //kadencja
    itoa(cadence_count, _cadence_count,10);
    setStr(_cadence_count,18,16,BLACK); 
     //ilość zapisów
    itoa(savings,_savings,10);
    setStr(_savings,0,16,BLACK);
    
    
    //DRUGA LINIA
    setLine(0,23,38,23,BLACK);

  
    //srednie obroty kola
    dtostrf(rotations, 5, 1, _rotations);
    setStr(_rotations, 0, 24, BLACK);
    //srednia predkosc
    
    dtostrf(average, 6, 3, _average);
    setStr(_average, 0, 32, BLACK);

    //PIONOWA LINIA
    setLine(38,0,38,48, BLACK);

 //readmaksymalna predkosc
    dtostrf(max_speed, 6,3,_max_speed);
    setStr(_max_speed,40,0,WHITE);

    //readmaksymalna predkosc
    dtostrf(read_max_speed, 6,3,_read_max_speed);
    setStr(_read_max_speed,40,40,BLACK);

    
    //całkowity dystans
    full_distance = distance+read_full_distance;
    dtostrf(full_distance,7,2,_full_distance);
    setStr(_full_distance,40,8,BLACK);

   
    //stosune prędkości do kadencji
    cad_spd = bike_speed/cadence_count;
    dtostrf(cad_spd,4,2,_cad_spd);
    setStr(_cad_spd,58,16,BLACK);
    
   
    
    updateDisplay();
    disp_time = millis();
  }
  if( ((int)(millis() - (int)magnet_time) > (int)NOT_RIDING_AFTER) && riding ) {
      //zapis
      riding = false;
      float temp;
      EEPROM.put(DISTANCE_ADRR, full_distance);
      savings++;
    }
}

void rotation() {  
  magnet_time = millis();
  if (magnet_time - last_magnet_time > 50) {
    turns++;
    bike_speed = (float(CIRCUMFERENCE)/1000000)/((float(magnet_time) - float(last_magnet_time))/3600000); //km/h
    distance = float(turns) * float(CIRCUMFERENCE)/1000000;
    last_magnet_time = magnet_time;
    riding = true;
  }
  
}

void cadence() {
  c_magnet_time = millis();
  if(c_magnet_time - c_last_magnet_time > 50) {
    c_turns++;
    cadence_count = 60000/(c_magnet_time - c_last_magnet_time);
    c_last_magnet_time = c_magnet_time;
  }
}

