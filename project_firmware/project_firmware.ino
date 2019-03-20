/****************************************************

Instructions: 
1. Ensure that the DHT and HX711 library files are installed correctly.
2. Wiring set up
	a. DHT22 data --> D4
	b. HX711 data --> D2
	c. HX711 data --> D3
	d. SIM800L wiring is the same 
3. 



*****************************************************/



#include <SoftwareSerial.h>
#include <DHT.h>
#include <HX711.h>

//First test the Arduino code with the line below as is. Then test it with the line below commented out
#define SIM800_ORIGINAL_CODE

#ifdef SIM800_ORIGINAL_CODE
	#define		PRINT_VERS()	Serial.println("---\tUsing ORIGINAL CODE\t---")
#else
	#define		PRINT_VERS()	Serial.println("---\tUsing DRances' CODE\t---")
#endif

//#define TEST_COMMS

#define DHTPIN  5
#define DHTTYPE DHT22

#define SHORT_DELAY		10
#define	LONG_DELAY		5000
#define	TIMEOUT			2000

#define	CMD_RESPONSE_OK		0x00
#define CMD_RESPONSE_OTHER	0x01
#define	CMD_TIMEOUT 		0x02
#define CMD_NO_RESPONSE		0x04

SoftwareSerial Sim800l(8,9); // RX, TX
DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

float dht22_humidity;
float dht22_temperature;
float hx711_lbs;
float hx711_calib_factor = -7050; //Taken from provided sketch, assumed correct
String str = "";
String sim800_response = "";
float test_value = 100.11;

uint8_t cmd_status;

uint32_t tick_start, tick_current;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Testing sketch for Blake's project");
  dht.begin();
  
#ifdef TEST_COMMS
	sim800_test_comms();
#endif

  setup_sim800();
  connect_sim800();
  
  //Close UDP Context
  // Sim800l.write("AT+CIPSHUT\r\n"); 
  // delay(5000); 
  // if (Sim800l.available()) {
    // Serial.write(Sim800l.read());
  // }
}

void loop() {
#ifdef TEST_COMMS
	sim800_test_comms();
	delay(2000);
#else
	/* if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
  if (Serial.available()) {
    Sim800l.write(Serial.read());
  } */
  Serial.println("Reading HX711 data");
  getHX711Data();
  Serial.println("Reading DHT22 data");
  getDHT22Data();
  
  //SIM800 Setup-loop
  setup_sim800();
  connect_sim800();
  sim800_transmit_data();
  
  //Close the connection
  Sim800l.write("AT+CIPSHUT\r\n"); 
  delay(LONG_DELAY); 
  if (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }
#endif
	

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
  if (isnan(dht22_humidity) || isnan(dht22_temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return false;
  }
  str = "Humidity = \t" + String(dht22_humidity);
  str += "\n\rTemperature = \t" + String(dht22_temperature);
  Serial.println(str);
  return true;
}

void setup_sim800() {
#ifdef SIM800_ORIGINAL_CODE
	PRINT_VERS();
	// set the data rate for the SoftwareSerial port
	Serial.println("SIM800 - set baud rate");
	Sim800l.begin(4800);
	delay(5000);
	Sim800l.write("AT \n"); 
	delay(10000); 

	//Attach to a GPRS Service
	Serial.println("SIM800 - attach to GPRS service");
	Sim800l.write("AT+CGATT=1\r\n"); 
	delay(7000); 
	if (Sim800l.available() > 0) {
		Serial.write(Sim800l.read());
	}
	//Set APN = hologram, no user or pw
	Serial.println("SIM800 - set APN, user, pw");
	Sim800l.write("AT+CSTT=\"hologram\"\r\n");
	delay(5000); 
	if (Sim800l.available() > 0) {
		Serial.write(Sim800l.read());
	}
#else 
	PRINT_VERS();
	// set the data rate for the SoftwareSerial port
	Serial.println("SIM800 - set baud rate");
	Sim800l.begin(4800);
	delay(5000);
	Sim800l.write("AT \n");
	delay(10000); 
	
	//Attach to a GPRS Service
	Serial.println("SIM800 - attach to GPRS service");
	Sim800l.write("AT+CGATT=1\r\n");
	delay(SHORT_DELAY);
	while (Sim800l.available() > 0) {
		sim800_response += (String)Sim800l.read();
	}
	Serial.println(sim800_response);
	sim800_response = "";
	
	//Set APN = hologram, no user or pw
	Serial.println("SIM800 - set APN, user, pw");
	Sim800l.write("AT+CSTT=\"hologram\"\r\n");
	delay(SHORT_DELAY);
	while (Sim800l.available() > 0) {
		sim800_response += (String)Sim800l.read();
	}
	Serial.println(sim800_response);
	sim800_response = "";
#endif
	
}

void connect_sim800() {
	#ifdef SIM800_ORIGINAL_CODE
	PRINT_VERS();
	//Set up GPRS Wireless connection
	Serial.println("Setting up Wireless connection");
	Sim800l.write("AT+CIICR\r\n"); 
	delay(6000); 
	if (Sim800l.available() > 0) {
		Serial.write(Sim800l.read());
	}

	//Get local IP --> is this the IP that's passed in +CIPSTART?
	Serial.println("Setting local IP");
	Sim800l.write("AT+CIFSR\r\n"); 
	delay(5000); 
	if (Sim800l.available() > 0) {
		Serial.write(Sim800l.read());
	}

	//Start UDP connection at the address provided, at port 8888
	Serial.println("Setting up UDP connection at the IP provided");
	Sim800l.write("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"\r\n");
	delay(5000); 
	if (Sim800l.available()) {
		Serial.write(Sim800l.read());
	}
	
#else 
	PRINT_VERS();
	//Set up GPRS Wireless connection
	Serial.println("Setting up Wireless connection");
	Sim800l.write("AT+CIICR\r\n");
	delay(LONG_DELAY);
	while (Sim800l.available() > 0) {
		sim800_response += (String)Sim800l.read();
	}
	Serial.println(sim800_response);
	sim800_response = "";
	
	//Get local IP --> is this the IP that's passed in +CIPSTART?
	Serial.println("Setting local IP");
	Sim800l.write("AT+CIFSR\r\n"); 
	delay(LONG_DELAY);
	while (Sim800l.available() > 0) {
		sim800_response += (String)Sim800l.read();
	}
	Serial.println(sim800_response);
	sim800_response = "";
	
	//Start UDP connection at the address provided, at port 8888
	Serial.println("Setting up UDP connection at the IP provided");
	Sim800l.write("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"\r\n");
	delay(LONG_DELAY);
	while (Sim800l.available() > 0) {
		sim800_response += (String)Sim800l.read();
	}
	Serial.println(sim800_response);
	sim800_response = "";
	
#endif
}

void sim800_transmit_data() {
	String datalen = "AT+CIPSEND=";
	uint16_t len = 39;
	
#ifdef SIM800_ORIGINAL_CODE	
	//Send data through UDP connection. Need length of data
	//Is the length supposed to include the \r and the \n?
	Serial.println("Sending data length");
	Sim800l.write("AT+CIPSEND=39\r\n");
	delay(5000); 
	if (Sim800l.available()) {
		Serial.write(Sim800l.read());
	}
	//Data being sent
	Serial.println("Sending data");
	Sim800l.write("temperature,device=arduino01 value=84.9\r\n"); 
	delay(5000); 
	if (Sim800l.available()) {
		Serial.write(Sim800l.read());
	}
#else
	str = "temperature,device=arduino01 value=" + (String)dht22_temperature;
	str += "\n\r";
	len = str.length() - 2;		//Blake said CR and NL characters are not to be counted in len
	datalen += (String)len + "\r\n";
	Serial.println("Sending data length");
	Sim800l.write(datalen.c_str());
	//delay(LONG_DELAY);
	sim800_cmd_success(5000);
	// while (Sim800l.available() > 0) {
		// sim800_response += (String)Sim800l.read();
	// }
	// Serial.println(sim800_response);
	// sim800_response = "";
	
	//Data being sent
	Serial.println("Sending data");
	Sim800l.write(str.c_str()); 
	sim800_cmd_success(5000);
	// delay(LONG_DELAY);
	// while (Sim800l.available() > 0) {
		// sim800_response += (String)Sim800l.read();
	// }
	// Serial.println(sim800_response);
	// sim800_response = "";
	
#endif
}

void test_value_update()
{
	if (test_value >= 500.0){
		test_value = 100.11;
	} else {
		test_value += 10.11;
	}
}

void sim800_test_comms()
{
	sim800_response = "testing the sim800_response\n\r";
	
	Serial.println(sim800_response);
	sim800_response = "";
	
	// set the data rate for the SoftwareSerial port
	Serial.println("SIM800 - set baud rate");
	Sim800l.begin(4800);
	delay(5000);
	Sim800l.write("AT \n");
	cmd_status = sim800_cmd_success(1000);	//Wait up to one second for this
	switch (cmd_status) {
		case CMD_RESPONSE_OK:
			Serial.println("OK received! Command successful");
			break;
		case CMD_RESPONSE_OTHER:
			Serial.println("OK not received. SIM800 response ==\t==\t==");
			Serial.println(sim800_response);
			Serial.println("\n\r===\t===\t===\t===");
			break;
		case CMD_TIMEOUT:
			Serial.println("Command timed out!");
			break;
		case CMD_NO_RESPONSE:
			Serial.println("SIM800 unresponsive");
			break;
	}
}

uint8_t sim800_cmd_success(uint32_t x) 
{
	bool timed_out = false;
	uint32_t timeout_value = x;
	tick_start = millis();
	sim800_response = "";
	//Wait til data available from SIM800 or til timeout
	while (!Sim800l.available()) {
		tick_current = millis();
		if ((tick_current - tick_start) >= timeout_value) {
			timed_out = true;
			break;
		}
	}
	
	if (timed_out == true) {
		return CMD_TIMEOUT;
	}
	
	if (Sim800l.available() == 0) {
		return CMD_NO_RESPONSE;
	}
	//At this point, we received data
	while (Sim800l.available() > 0) {
		sim800_response += (char)Sim800l.read();
	}
	
	if (sim800_response.indexOf("OK") >= 0) {
		return CMD_RESPONSE_OK;
	} else {
		return CMD_RESPONSE_OTHER;
	}
	
}


