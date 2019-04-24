/****************************************************

Instructions: 
1. Ensure that the DHT and HX711 library files are installed correctly.
2. Wiring set up
	a. DHT22 data --> D6
	b. HX711 data --> D2
	c. HX711 data --> D3
	d. SIM800L wiring is the same 
3. 



*****************************************************/

#include <SoftwareSerial.h>
#include <DHT.h>
#include <HX711.h>
#include <LowPower.h>

#define DHTPIN  6
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

float dht22_humidity;
float dht22_temperature;
float hx711_lbs;
float hx711_calib_factor = -7050; //Taken from provided sketch, assumed correct

float test_value = 100.11;

uint32_t tick_start, tick_current;

SoftwareSerial Sim800l(8,9); // RX, TX

String str = "";

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  dht.begin();
  
  // set the data rate for the SoftwareSerial port
  Sim800l.begin(4800);
  delay(5000);
  Sim800l.write("AT \n"); 
  delay(10000); 
  
  Serial.println("AT+CGATT=1");
  Sim800l.write("AT+CGATT=1\r\n"); 
   delay(7000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Serial.println("AT+CSTT=hologram");
  Sim800l.write("AT+CSTT=\"hologram\"\r\n");
  delay(5000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Serial.println("AT+CIICR");
  Sim800l.write("AT+CIICR\r\n"); 
  delay(6000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Serial.println("AT+CIFSR");
 Sim800l.write("AT+CIFSR\r\n"); 
  delay(5000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  Serial.println("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"");
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
  // Sim800l.write("AT+CIPSHUT\r\n"); 
  // delay(5000); 
  // if (Sim800l.available()) {
    // Serial.write(Sim800l.read());
  // }
}
void loop() {
	getDHT22Data();
	//getHX711Data();
	sendPayload();
	delay(500);
	Sim800l.write("AT+CIPSHUT\r\n"); 
    delay(5000); 
    if (Sim800l.available()) {
		Serial.write(Sim800l.read());
    }
	
	//15 mins * 60s/min = 900 secs
	//900 s / 8s = 112.5 which rounds up to 113
	for (uint8_t i =113; i > 0; i--) {
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
	}
	setup_sim800();
}

/*
 * HX711 Data Reading, in lbs
 * Calibration factor is from the sketch provided by Blake
 * Assumption - no need to change this factor or re-calibrate
 */
void getHX711Data() {
  scale.set_scale(hx711_calib_factor);
  hx711_lbs = scale.get_units();
  str = "HX711 Data: " + String(hx711_lbs);
  str += "lbs\n\r";
  Serial.println(str);
}

/*
 * DHT22 Humidity and Temperature Reading
 * Returns true if data reading is correct
 */
bool getDHT22Data() {
  dht22_humidity = dht.readHumidity();
  dht22_temperature = dht.readTemperature(true);
  //if (isnan(dht22_humidity) || isnan(dht22_temperature)) {
    //Serial.println(F("Failed to read from DHT sensor!"));
    //return false;
  //}
  str = "Humidity = \t" + String(dht22_humidity);
  str += "\n\rTemperature = \t" + String(dht22_temperature);
  Serial.println(str);
  return true;
}

void sendPayload(void) 
{
	String cmd = "AT+CIPSEND=";
	uint16_t len = 39;
	
	str = "temperature,device=arduino01 value=" + (String)dht22_temperature;
	len = str.length();// - 2;		//Blake said CR and NL characters are not to be counted in len
	cmd += (String)len + "\r\n";
	
	Serial.println("Sending data length");
	Sim800l.write(cmd.c_str());
	delay(10000);
	if (Sim800l.available()) {
		Serial.write(Sim800l.read());
	}

	//Data being sent
	Serial.println("Sending data");
	Sim800l.write(str.c_str());
	delay(10000);
	if (Sim800l.available()) {
		Serial.write(Sim800l.read());
	}
}

void setup_sim800(void)
{
	// set the data rate for the SoftwareSerial port
  Sim800l.begin(4800);
  delay(5000);
  Sim800l.write("AT \n"); 
  delay(10000); 
  
  Serial.println("AT+CGATT=1");
  Sim800l.write("AT+CGATT=1\r\n"); 
   delay(7000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Serial.println("AT+CSTT=hologram");
  Sim800l.write("AT+CSTT=\"hologram\"\r\n");
  delay(5000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Serial.println("AT+CIICR");
  Sim800l.write("AT+CIICR\r\n"); 
  delay(6000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  
  Serial.println("AT+CIFSR");
 Sim800l.write("AT+CIFSR\r\n"); 
  delay(5000); 
  if (Sim800l.available() > 0) {
    Serial.write(Sim800l.read());
  }
  Serial.println("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"");
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
}