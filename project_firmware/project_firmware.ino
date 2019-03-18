#include <SoftwareSerial.h>
#include <DHT.h>
#include <HX711.h>

#define DHTPIN  4
#define DHTTYPE DHT22

SoftwareSerial Sim800l(8,9); // RX, TX
DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

float dht22_humidity;
float dht22_temperature;
float hx711_lbs;
float hx711_calib_factor = -7050; //Taken from provided sketch, assumed correct
String str = "";

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Set up DHT22");
  dht.begin();

  // set the data rate for the SoftwareSerial port
  Sim800l.begin(4800);
  delay(5000);
  Sim800l.write("AT \n"); 
  delay(10000); 
  Sim800l.write("AT+CGATT=1\r\n"); 
   delay(7000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Sim800l.write("AT+CSTT=\"hologram\"\r\n");
  delay(5000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  
  Sim800l.write("AT+CIICR\r\n"); 
  delay(6000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  
 Sim800l.write("AT+CIFSR\r\n"); 
  delay(5000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Sim800l.write("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"\r\n");
  delay(5000); 
  if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
  
  Sim800l.write("AT+CIPSEND=39\r\n");
  delay(5000); 
  if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
   
  Sim800l.write("temperature,device=arduino01 value=84.9\r\n"); 
  delay(5000); 
  if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
  Sim800l.write("AT+CIPSHUT\r\n"); 
  delay(5000); 
  if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
}
void loop() {
if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
  if (Serial.available()) {
    Sim800l.write(Serial.read());
  }
}

/*
 * HX711 Data Reading, in lbs
 * Calibration factor is from the sketch provided by Blake
 * Assumption - no need to change this factor or re-calibrate
 */
void getHX711Data() {
  scale.set_scale(hx711_calib_factor);
  hx711_lbs = scale.get_units();
  
}

/*
 * DHT22 Humidity and Temperature Reading
 * Returns true if data reading is correct
 */
bool getDHT22Data() {
  delay(2000);
  Serial.println("Reading DHT22 device");
  dht22_humidity = dht.readHumidity();
  dht22_temperature = dht.readTemperature(true);
  if (isnan(dht22_humidity) || isnan(dht22_temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return false;
  }
  str = "Humidity = \t" + String(dht22_humidity);
  str = "\n\rTemperature = \t" + String(dht22_temperature);
  Serial.println(str);
  return true;
}


