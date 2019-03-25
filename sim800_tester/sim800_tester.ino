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
//#define SIM800_ORIGINAL_CODE

#ifdef SIM800_ORIGINAL_CODE
	#define		PRINT_VERS()	Serial.println("---\tUsing ORIGINAL CODE\t---")
#else
	#define		PRINT_VERS()	Serial.println("---\tUsing DRances' CODE\t---")
#endif

//#define TEST_COMMS
#define DEBUG_PRINT_RESPONSE

#define DHTPIN  5
#define DHTTYPE DHT22

#define SHORT_DELAY		10
#define	LONG_DELAY		5000
#define	TIMEOUT			5000

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
char input;
uint8_t cmd_status = 0xFF;

uint32_t tick_start, tick_current;

void setup() {
	// Open serial communications and wait for port to open:
	Serial.begin(9600);
	while (!Serial)

	Serial.println("Testing sketch for Blake's project");
	dht.begin();

	Sim800l.begin(4800);

}

void loop() {
#ifdef TEST_COMMS
	//sim800_test_comms();
	//delay(2000);
#else
	if (Serial.available()) {
		input = Serial.read();
		menu_handler(input);
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

void sim800_transmit_data() {
	String cmd = "AT+CIPSEND=";
	uint16_t len = 39;
	
	str = "temperature,device=arduino01 value=" + (String)dht22_temperature;
	str += "\n\r";
	len = str.length() - 2;		//Blake said CR and NL characters are not to be counted in len
	cmd += (String)len + "\r\n";
	
	Serial.println("Sending data length");
	Sim800l.write(cmd.c_str());
	cmd_status = sim800_cmd_success(TIMEOUT);
	//handle_sim800_response();

	//Data being sent
	Serial.println("Sending data");
	Sim800l.write(str.c_str()); 
	cmd_status = sim800_cmd_success(TIMEOUT);
	//handle_sim800_response();

}

void test_value_update()
{
	if (test_value >= 500.0){
		test_value = 100.11;
	} else {
		test_value += 10.11;
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
#ifdef DEBUG_PRINT_RESPONSE
	Serial.print(">>\tSIM800 RESPONSE\t<<\n\r");
	Serial.print(sim800_response);
	Serial.print("\n\r>>\t******\t<<\n\r");
#endif	
	
	if (sim800_response.indexOf("OK") >= 0) {
		return CMD_RESPONSE_OK;
	} else {
		return CMD_RESPONSE_OTHER;
	}
	
}

void handle_sim800_response()
{
	cmd_status = sim800_cmd_success(TIMEOUT);	//Wait up to 4 seconds for this
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

void menu_handler(char c)
{
	uint16_t data_len = 0;
	
	switch (c) {
		case '0':	//Check the HW connection between Arduino and SIM800L
			Sim800l.write("AT\n\r");
			break;
		case '1':	//Check RF signal quality
			Sim800l.write("AT+CSQ\n\r");
			break;	
		case '2':	//Establish connection
			Sim800l.write("AT+CGATT=1;+CSTT=\"hologram\";+CIICR\n\r");
			break;
		case '3':	//Start UDP connection
			Sim800l.write("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"\r\n");
			break;
		case '4':	//End the PDP connection
			Sim800l.write("AT+CIPSHUT\n\r");
			break;		
		case '5':	//Send data length
			test_value_update();
			str = "temperature,device=arduino01 value=" + (String)test_value;
			data_len = str.length();
			str += "\n\r";
			sim800_response = "AT+CIPSEND=" + (String)data_len;
			sim800_response += "\n\r";
			Sim800l.write(sim800_response.c_str());
			break;
		case '6':	//Now send payload
			Sim800l.write(str.c_str());
			str = "";
			break;
	}
	sim800_cmd_success(3000);
}
