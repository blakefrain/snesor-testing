#include <SoftwareSerial.h>

SoftwareSerial Sim800l(8,9); // RX, TX


void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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
