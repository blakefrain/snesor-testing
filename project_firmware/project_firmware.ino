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

//#define TEST_COMMS
#define DEBUG_PRINT_RESPONSE

#ifdef DEBUG_PRINT_RESPONSE
	#define	DBG_PRINT(x)	Serial.print(x)
	#define	DBG_PRINTLN(x)	Serial.println(x)
#else
	#define DBG_PRINT(x)
	#define	DBG_PRINTLN(x)
#endif

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

char response_buf[64] = { 0 };

uint8_t cmd_status = 0xFF;

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
  //Serial.println("Reading HX711 data");
  //getHX711Data();
  //Serial.println("Reading DHT22 data");
  //getDHT22Data();
  dht22_temperature = test_value;
  sim800_transmit_data();
  test_value_update();

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
	// set the data rate for the SoftwareSerial port
	Serial.println("SIM800 - set baud rate");
	Sim800l.begin(4800);
	delay(2000);
	sim800_getReply("AT", 1000);
	
	
	//Attach to GPRS service
	Serial.println("Attaching to GPRS service");
	sim800_getReply("AT+CGATT=1", 5000);
	
	//Set APN = hologram, no user or pw
	Serial.println("SIM800 - set APN, user, pw");
	sim800_getReply("AT+CSTT=\"hologram\"", 1000);
	
}

void connect_sim800() {
	//Set up GPRS Wireless connection
	Serial.println("Setting up Wireless connection");
	sim800_getReply("AT+CIICR", 2000);
	
	//Get local IP --> is this the IP that's passed in +CIPSTART?
	Serial.println("Setting local IP");
	sim800_getReply("AT+CIFSR", 2000);
	
	//Start UDP connection at the address provided, at port 8888
	Serial.println("Setting up UDP connection at the IP provided");
	sim800_getReply("AT+CIPSTART=\"UDP\",\"73.230.127.71\",\"8888\"", 5000);
	
}

void sim800_transmit_data() {
	String cmd = "AT+CIPSEND=";
	uint16_t len = 39;
	
	str = "temperature,device=arduino01 value=" + (String)dht22_temperature;
	//str += "\n\r";
	len = str.length();// - 2;		//Blake said CR and NL characters are not to be counted in len
	cmd += (String)len + "\r";
	
	Serial.println("Sending data length");
	sim800_getReply(cmd.c_str(), 5000);
	//Sim800l.write(cmd.c_str());
	//cmd_status = sim800_cmd_success(TIMEOUT);
	//handle_sim800_response();

	//Data being sent
	Serial.println("Sending data");
	sim800_getReply(str.c_str(), 5000);
	//Sim800l.write(str.c_str()); 
	//cmd_status = sim800_cmd_success(TIMEOUT);
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

void sim800_test_comms()
{	
	// set the data rate for the SoftwareSerial port
	Serial.println("SIM800 - set baud rate");
	Sim800l.begin(4800);
	delay(2000);
	sim800_getReply("AT", 1000);
}

//Similarly structure to Adafruit_FONA readline
uint8_t sim800_read(uint16_t timeout) 
{
	char read_in;
	uint8_t index = 0;
	
	while (timeout--) {
		while (Sim800l.available()) {
			read_in = Sim800l.read();
			if (read_in == '\r') continue;
			response_buf[index++] = read_in;
		}
		
		if (timeout == 0) {
			break;
		}
		delay(1);
	}
	
	response_buf[index] = 0;
	
	return index;
}

//Modeled after getReply from Adafruit_FONA
uint8_t sim800_getReply(char* send, uint16_t timeout) 
{
	Sim800l.flush();
	
	DBG_PRINT("\tSENT:: "); 
	DBG_PRINTLN(send);
	
	Sim800l.println(send);
	
	uint8_t len = sim800_read(timeout);
	
	DBG_PRINT("\tRCVD:: ");
	DBG_PRINTLN(response_buf);
	
	return len;
}

