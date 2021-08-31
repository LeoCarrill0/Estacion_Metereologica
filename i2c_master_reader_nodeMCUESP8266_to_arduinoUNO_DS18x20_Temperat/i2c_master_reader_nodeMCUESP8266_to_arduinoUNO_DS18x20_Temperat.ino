#include <Wire.h>
#include <PolledTimeout.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>   //Include File System Headers
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define SDA_PIN 4
#define SCL_PIN 5
#define slave  0x11
const int16_t I2C_MASTER = 0x42;
const int16_t I2C_SLAVE = 0x08;
String celsius, fahrenheit, LDR, Humedad;

const char* htmlfile = "/index.html";

const char *ssid = "AndroidAP";
const char *password = "mandarina2019";

String ledState;

int sensorValue = 0; 
const int analogInPin = A0;

#define LED 2
ESP8266WebServer server(80);

void handleADC() {
 int a = analogRead(analogInPin);
 String adcValue = String(a);
 digitalWrite(LED,!digitalRead(LED)); //Toggle LED on data request ajax
 server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}

void handlePuls0() {
  String a = String(sensor_Temp());
  server.send(200, "text/plane", a);
  celsius = "";
  fahrenheit = "";
}

void handlePuls() {
  int cont = 0;
  {
    Wire.requestFrom(I2C_SLAVE, 10);    // request 6 bytes from slave device #8

    while (Wire.available()) { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      if(cont < 1)
        celsius += c;
      cont++;
    }
  }
  server.send(200, "text/plane", celsius);
  celsius = "";
  fahrenheit = "";
}

void handlePuls1() {
  float k;
  Wire.beginTransmission(slave); //Establecer comunicacion con el slave
  Wire.endTransmission();

  delay(5);
 
  Wire.requestFrom( slave, 1);//pide 1 byte al slave
  delay(10);
 
  while( Wire.available() ){
     k = Wire.read();
  }
  String adc = String(k);
  server.send(200, "text/plane", adc);
}

const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void handlePuls3() {
  int cont = 0;
  {
    Wire.requestFrom(I2C_SLAVE, 10);    // request 6 bytes from slave device #8

    while (Wire.available()) { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      if(cont>0 && cont < 4)
        LDR += c;
      cont++;
    }
  }
  LDR = String(map(LDR.toInt(), 100, 200, 0, 100));
  server.send(200, "text/plane", LDR);
  LDR = "";
}

void handlePuls4() {
  int cont = 0;
  {
    Wire.requestFrom(I2C_SLAVE, 10);    // request 6 bytes from slave device #8

    while (Wire.available()) { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      if(cont > 3 && cont < 7)
        Humedad += c;
      cont++;
    }
  }
  Humedad = String(map(Humedad.toInt(), 100, 200, 0, 100));
  server.send(200, "text/plane", Humedad);
  Humedad = "";
}

int a_1 = 1;

float sensor_Temp(){
 float a;
 sensors.requestTemperatures();
 a=sensors.getTempCByIndex(0);
 return a;
}

void handleLED() {
  String ledState = "OFF";
  String t_state = server.arg("LEDstate"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
  if (t_state == "1")
  {
    digitalWrite(LED, LOW); //LED ON
    ledState = "Apagar"; //Feedback parameter
  }
  else
  {
    digitalWrite(LED, HIGH); //LED OFF
    ledState = "Encender"; //Feedback parameter
  }
  server.send(200, "text/plane", ledState); //Send web page
}

void handleRoot() {
  server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  server.send(302, "text/plane", "");
}

void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN, I2C_MASTER);        // join i2c bus (address optional for master)
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  pinMode(LED, OUTPUT); //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/getADC", handlePuls0);
  server.on("/getADC1", handlePuls3);
  server.on("/getADC3", handlePuls);
  server.on("/getADC4", handlePuls4);
  server.on("/setLED", handleLED);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.begin();
  digitalWrite(LED, LOW);
}

void loop() {
  server.handleClient();
}

 bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.htm";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}
