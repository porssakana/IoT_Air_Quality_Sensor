/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-bme680-sensor-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "bsec.h"
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "credentials.h"

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);
void ledBlink(void);
void ledBlinkRapid(void);
void UpdateLedFrequency(int var);


// Replace with your network credentials
const char ssid[] = WIFI_SSID;
const char password[] = WIFI_PASSWORD;

// ==========| Pin Declarations | ==========
const int LED_PIN = D3;

// ==========| Objects | ==========
Bsec bme;

// ==========| Sensor Variables | ==========
String output;
String calibration_text;
float temperature;
float humidity;
float pressure;
float gasResistance;
int iaq;
int calibration_status;

// ==========| Init Web Server | ==========
AsyncWebServer server(80);
AsyncEventSource events("/events");

// ==========| Time-based Variables | ==========
unsigned long lastTime_DATA = 0;
unsigned long lastTime_LED = 0; 
unsigned long timerDelay = 1000;  // frequency of reading values from the sensor
unsigned long ledDelay = 1;

// ==========| Main Data Update Function | ==========
void getBME680Readings(){
  // Tell BME680 to begin measurement.
  if (bme.run()) { //if new data is available
    temperature = bme.temperature;
    pressure = bme.pressure / 100.0;
    humidity = bme.humidity;
    iaq = bme.iaq;
    calibration_status = bme.iaqAccuracy;
    UpdateLedFrequency(iaq);
    
    if (calibration_status == 3) {
      calibration_text = "Done";
    } else {
      calibration_text = "In Progress...  " + String(calibration_status) + "/3";
    }
  } else {
    Serial.println(F("Failed to begin reading :("));
    checkIaqSensorStatus();
  }
}

String processor(const String& var){
  getBME680Readings();
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "HUMIDITY"){
    return String(humidity);
  }
  else if(var == "PRESSURE"){
    return String(pressure);
  }
 else if(var == "IAQ"){
    return String(iaq);
  }
 else if(var == "CALIBRATION"){
    return String(calibration_text);
  }
}

// ==========| WEB SERVER HTML AND CSS | ==========
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>BME680 Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #4B1D3F; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .card.temperature { color: #0e7c7b; }
    .card.humidity { color: #17bebb; }
    .card.pressure { color: #3fca6b; }
    .card.gas { color: #d62246; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>BME680 WEB SERVER</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> TEMPERATURE</h4><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> HUMIDITY</h4><p><span class="reading"><span id="hum">%HUMIDITY%</span> &percnt;</span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-angle-double-down"></i> PRESSURE</h4><p><span class="reading"><span id="pres">%PRESSURE%</span> hPa</span></p>
      </div>
      <div class="card iaq">
        <h4><i class="fas fa-broom"></i> IAQ</h4><p><span class="reading"><span id="iaq">%IAQ%</span> </span></p>
      </div>
      <div class="card calibration">
        <h4><i class="fas fa-wrench"></i> CALIBRATION</h4><p><span class="reading"><span id="cal">%CALIBRATION%</span> </span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
 
 source.addEventListener('pressure', function(e) {
  console.log("pressure", e.data);
  document.getElementById("pres").innerHTML = e.data;
 }, false);
  
 source.addEventListener('iaq', function(e) {
  console.log("iaq", e.data);
  document.getElementById("iaq").innerHTML = e.data;
 }, false);
 
 source.addEventListener('calibration', function(e) {
  console.log("calibration", e.data);
  document.getElementById("cal").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

// ==========| Setup | ==========
void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup");
  
  // pin setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Wire.begin();
  
 
  bme.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(bme.version.major) + "." + String(bme.version.minor) + "." + String(bme.version.major_bugfix) + "." + String(bme.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
  
  bme.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

// ==========| Main Loop | ==========
void loop() {
  
  //updates values from the sensor and outputs info to the server and console
  if ((millis() - lastTime_DATA) > timerDelay) {
    getBME680Readings();
    
    Serial.printf("Temperature = %.2f ºC \n", temperature);
    Serial.printf("Humidity = %.2f % \n", humidity);
    Serial.printf("Pressure = %.2f hPa \n", pressure);
    Serial.printf("Index of Air Quality = %d \n", iaq);
    Serial.printf("Calibration status = %d \n", calibration_status);
    Serial.print("Calibration text = ");
    Serial.print(calibration_text);
    Serial.println();
    Serial.println();

    // Send Events to the Web Server with the Sensor Readings
    events.send("ping",NULL,millis());
    events.send(String(temperature).c_str(),"temperature",millis());
    events.send(String(humidity).c_str(),"humidity",millis());
    events.send(String(pressure).c_str(),"pressure",millis());
    events.send(String(iaq).c_str(),"iaq",millis());
    events.send(String(calibration_text).c_str(),"calibration",millis());
    
    lastTime_DATA = millis();
  }
  
  //flashes the LED based on the sensor data
  if ((millis() - lastTime_LED) > ledDelay * 1000) {
    digitalWrite(LED_PIN, HIGH);
    lastTime_LED = millis();
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// Helper function definitions
void checkIaqSensorStatus(void) {
  if (bme.status == BSEC_OK) {
    for (;;) {
      ledBlink();
    }
  }
  
  if (bme.status != BSEC_OK) {
    if (bme.status < BSEC_OK) {
      output = "BSEC error code : " + String(bme.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
        ledBlinkRapid();
    } else {
      output = "BSEC warning code : " + String(bme.status);
      Serial.println(output);
    }
  }

  if (bme.bme680Status != BME680_OK) {
    if (bme.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(bme.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
        ledBlinkRapid();
    } else {
      output = "BME680 warning code : " + String(bme.bme680Status);
      Serial.println(output);
    }
  }
}


void errLeds(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// blinking slowly
void ledBlink(void) {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(2000);
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
}

// blinking rapidly
void ledBlinkRapid(void) {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
}

// this is called inside getBME680Readings
void UpdateLedFrequency(int var){
  ledDelay = map(var, 0, 500, 1, 5);
}
