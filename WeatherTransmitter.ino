#include <SoftwareSerial.h>
#include <dht.h>
// max 5 
//max 12

#define DHT22_PIN 4
#define WIND_SENSOR_PIN 9
#define RAIN_SENSOR_PIN 8
#define SERIAL_TIMEOUT 700
SoftwareSerial mySerial(3, 2); // TX, RX
dht DHT;


long transmitInterval = 60000l;
int dhtInterval = 1500;
int windInterval  = 246;
unsigned long dhtTime;
unsigned long qTime;
unsigned long windTime;
unsigned long rainSensorDealyTime;
float tempSum = 0.0;
float humSum = 0.0;
int dhtCounter = 0;
boolean windState1 = false;
boolean windState2 = false;
boolean rainState1 = false;
boolean rainState2 = false;
byte windCounter = 0;
float windSum = 0;
byte maxWind = 0;
int windCounter2 = 0;
int rainSensorDealy = 35;
byte rainCounter = 0;
boolean windCont = false;  /////////////

void setup()
{
  mySerial.begin(76800);    
 // Serial.begin(115200);
//  delay(1700);
 // mySerial.print("AT+FU3");
//  delay(800);
// mySerial.print("AT+B38400");
//  delay(800);
  
  mySerial.write(255);
  
  pinMode(WIND_SENSOR_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);

  delay(500);
  if (digitalRead(WIND_SENSOR_PIN) == LOW) {
    windState1 = false;
  } else {
    windState1 = true;
  }

  windState2 = windState1;


  dhtRead();
  
  windTime = millis();
  qTime = millis();
  dhtTime = millis();  
}
///pomiar czasu micros wynik przemnożyc x2
//main loop od 36 do 62 (czasami nawet 1200) , max 3400 podczas pomiaru dht

unsigned long t = 0; //////////

void loop()
{
  if (millis() > qTime + transmitInterval) {
     sendData();
     
     qTime = millis();
  }

  while(mySerial.available()){
    setSettings();
  }
    
  if (millis() > dhtTime + dhtInterval) {
    dhtRead();
    dhtTime = millis();
  }

  if (millis() > windTime + windInterval) {
    readWindSpeed();
    windTime = millis();
  }

  if (digitalRead(WIND_SENSOR_PIN) == LOW) {
    windState1 = false;
  } else {
    windState1 = true;
  }
  if (windState1 != windState2) {
    windCounter++;
  }
  windState2 = windState1;

  if (digitalRead(RAIN_SENSOR_PIN) == LOW) {
    rainState1 = false;
  } else {
    rainState1 = true;
  }
  if (rainState1 != rainState2 && millis() > rainSensorDealyTime + rainSensorDealy ) {
    rainCounter++;
    rainSensorDealyTime = millis();
  }
  rainState2 = rainState1;


}


void getInt(int &value){
  byte a,b;
  delay(2);
  if(mySerial.available()){
    a = mySerial.read();
  }else{
    mySerial.write((byte)0);
    return;
  }
  if(mySerial.available()){
    b = mySerial.read();
  }else{
    mySerial.write((byte)0);
    return;
  }
  value = a|b<<8;
  value*=100;
  mySerial.write((byte)1); 
}

int getSettings(){
  
  
}

void getLong(long &value){
  byte a,b;
  delay(2);
  if(mySerial.available()){
    a = mySerial.read();
  }else{
    mySerial.write((byte)0);
    return;
  }
  if(mySerial.available()){
    b = mySerial.read();
  }else{
    mySerial.write((byte)0);
    return;
  }
  value = a|b<<8;
  value*=100;
  mySerial.write((byte)1); 
}
void setSettings(){
    byte value = mySerial.read();
      switch(value){
        case 1:
        sendLastData(); 
        break;
        case 2:   
        getLong(transmitInterval);
        break;
        case 3:
        getInt(dhtInterval);
        break;
        case 4:
        getInt(windInterval);
        break;
        case 5:
        getInt(rainSensorDealy);
        break;
        case 6:
        delay(1);
        if(mySerial.available()){
          if(mySerial.read() == 0){
            windCont = false;
          }else{
            windCont = true;
          }
          mySerial.write((byte)1);
        }else{
          mySerial.write((byte)0);
        }
        break; 
        case 7:
        sendData();
        qTime = millis();
        break;
        case 8:
        mySerial.write(lowByte(transmitInterval/100));
        mySerial.write(highByte(transmitInterval/100));
        mySerial.write(lowByte(dhtInterval));
        mySerial.write(highByte(dhtInterval));
        mySerial.write(lowByte(windInterval));
        mySerial.write(highByte(windInterval));
        mySerial.write((byte)windCont);
        break;
        case 9:
        byte data[7];
        unsigned long t;   
        int8_t index;
        index = 0;
        t = millis();
        while(millis() < t + SERIAL_TIMEOUT){
          if(mySerial.available()){
            data[index] = mySerial.read();
            if(index < 7){
              index++;
            }else{
              break;
            }
          }
        }
        if(index <7){
          mySerial.write((byte)0);
          break;
        }else{
          transmitInterval = data[0]|data[1] <<8;
          transmitInterval*=100;
          dhtInterval = data[2]|data[3]<<8;
          windInterval =data[4]|data[5] <<8;
          if(data[6] ==0){
            windCont = false;
          }else{
            windCont = true;
          }
          mySerial.write((byte)1);
          break;
        }
        case 10:
        mySerial.write((byte) rainSensorDealy);
        break;
        case 11:
        mySerial.print(millis());
        break;
        case 12:
        mySerial.println(checkDHT(DHT.read22(DHT22_PIN)));
        break;
        case 13:
        mySerial.println("millis: " +String(millis()));
        mySerial.println("dhtInterval: " +String(dhtInterval));
        mySerial.println("transmitInterval: " +String(transmitInterval));
        mySerial.println("rainSensorDealy: " +String(rainSensorDealy));
        mySerial.println("windInterval: " +String(windInterval));
        mySerial.println("realTimeWindEnabled: " +String(windCont));
        mySerial.println(F("codeVersion: 1.2"));
        break; 
        default: mySerial.flush();  break;  
      }
}
float temp;
float wind;
byte hum;

void sendLastData(){
  if(temp >=0){
     mySerial.write(floor(temp));
  }else{
     mySerial.write(floor(abs(temp))+50);
  }  
   mySerial.write((abs(temp) - floor(abs(temp)))*100);  
  if(!windCont){
   mySerial.write(floor(wind));
   mySerial.write((wind - floor(wind))*100);  
   mySerial.write(maxWind);
  }
  if(hum != 100){
   mySerial.write(hum+100);
  }  
  if(rainCounter != 0){
    mySerial.write(rainCounter);
  }
}
void sendData(){
  if(dhtCounter ==0){
    dhtCounter = 1;
  }
     temp = tempSum/dhtCounter;
  if(temp >=0){
     mySerial.write(floor(temp));
  }else{
      mySerial.write(floor(abs(temp))+50);
  }
   
   mySerial.write((abs(temp) - floor(abs(temp)))*100);
   
  if(!windCont){
   wind = windSum / windCounter2;
   mySerial.write(floor(wind));
   mySerial.write((wind - floor(wind))*100);
   
   mySerial.write(maxWind);
  }
  hum = round(humSum/dhtCounter);
  if(hum != 100){
   mySerial.write(hum+100);
  }  
  if(rainCounter != 0){
    mySerial.write(rainCounter);
  }
  
    dhtCounter = 0;
    windCounter2 = 0;
    rainCounter = 0;
    humSum = 0.0;
    tempSum = 0.0;
    windSum = 0.0;
    maxWind = 0;
}

void readWindSpeed() {
  if(!windCont){
  windSum += windCounter;
  if (windCounter > maxWind) {
    maxWind = windCounter;
  }
  windCounter2++;
  windCounter = 0;
  }else{
    windCounter+=100;  
    mySerial.write(windCounter);
    windCounter =0;
   
  }
}
void dhtRead() {
  if(DHT.read22(DHT22_PIN) == DHTLIB_OK){
    tempSum += DHT.temperature;
    humSum += DHT.humidity;
    dhtCounter++;
  }
}

String checkDHT(int chk){
  switch (chk)
    {
    case DHTLIB_OK:
        return "OK";
    case DHTLIB_ERROR_CHECKSUM:
        return "Checksum error";
    case DHTLIB_ERROR_TIMEOUT:
        return "Time out error";
    case DHTLIB_ERROR_CONNECT:
        return "Connect error";
    case DHTLIB_ERROR_ACK_L:
        return "Ack Low error";
    case DHTLIB_ERROR_ACK_H:
        return "Ack High error";
    default:
        return "Unknown error";
    }
}
//opis kabelkow do wgrywania 
//reset - jasnoniebieski kabelek polaczony na koncu z czarnym
//rx - ciemnoniebieski poloczony na koncu z zielonym
//tx - pomaranczowy polonaczony na koncu z zielonym

