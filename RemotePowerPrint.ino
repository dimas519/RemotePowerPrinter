#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>

//untuk apinya
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>


const char *hostname="Automatic Printer";
const char *ssid= "xxxxxxxx";
const char *pass= "xxxxxxxxx";


#define pinRelay D7
#define pinSensorActive A0

String token="xxxxxxxxxxxxxxxxxxxxxx";

int i=0;
float value=0;
bool fiveMinutesSchedule=false;



ESP8266WebServer server(80);

// Serving Hello world
void postChangeStatus(){
  if(!server.hasHeader("Authorization")){
    server.send(401, "text/text", "Not Authorized");
    Serial.println("no auth");
  }else{
    if(!  (server.header("Authorization")==token) ){
      server.send(401, "text/text", "Not Authorized");
      Serial.println("wrong pass");
    }else{
      JsonDocument requestJson;
       DeserializationError error = deserializeJson(requestJson,server.arg("plain"));
       if(error){
        server.send(400, "text/text", "Json not recognize");
       }
      if(!requestJson.containsKey("set_on")){
        server.send(400, "text/text", "Body contain missing");
      }else{
        bool turningOn=requestJson["set_on"];
        Serial.print("Request: ");
        Serial.println(turningOn);

        int result=changeState(turningOn);
        Serial.print("hasil: ");
        Serial.println(result);

        String msg="{";
        msg+="\"status\":";
        if(result==1){
          msg+="\"success\"";
          server.send(200, "text/json", msg);
        }else{
          msg+="\"failed\"";
          msg+=",\"msg\":";
          if(result==-2){
            msg+="\"already on\"}";
          }else if(result==-1){
            msg+="\"already off\"}";
          }else {
            msg+="\"failed action\"}";
          }

          server.send(400, "text/json", msg);
        }

      }
    }
  }
}

void getStatus(){
  if(isPrinterOn()){
    server.send(200, "text/json", "{\"status_On\": true}");
  }else{
    server.send(200, "text/json", "{\"status_On\": false}");
  }
}


// Define routing
void restServerRouting() {
    server.on(F("/status"), HTTP_GET, getStatus);
    server.on(F("/status"), HTTP_POST, postChangeStatus);
}
 
// Manage not found URL
void handleNotFound() {
  String message = "Not Found\n";
  // message += "URI: ";
  // message += server.uri();
  // message += "\nMethod: ";
  // message += (server.method() == HTTP_GET) ? "GET" : "POST";
  // message += "\nArguments: ";
  // message += server.args();
  // message += "\n";
  // for (uint8_t i = 0; i < server.args(); i++) {
  //   message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  // }
  server.send(404, "text/plain", message);
}




void setup() {
  Serial.begin(9600);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, pass); 


  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay,HIGH);
  pinMode(pinSensorActive, INPUT);


  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Running");


 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  
  server.handleClient();


  if(i==172800){
    changeState(true);
    i=0;
  }

  if(fiveMinutesSchedule && i==300){
    changeState(false);
    fiveMinutesSchedule=false;
  }

  i++;
  delay(1000);
}


void pressButton(){
  digitalWrite(pinRelay, LOW);
  delay(700);
  digitalWrite(pinRelay,HIGH);
  delay(200); //make sure the printer already response
}




//return value:
//  -2 already Off
//  -1 already on
//  0 failed 
// 1 success
int changeState(bool turnOn){
  if(turnOn){
    if(isPrinterOn()){
      return -2;
    }else{
      pressButton();
      return isPrinterOn();
    }

  }else{
    if(isPrinterOn()){
      pressButton();
      delay(2000); //make sure printer not blinking anymore
      return !isPrinterOn();
    }else{
      return -1;
    }

  }
}




bool isPrinterOn(){
  
  // float R1 = 30000.0;
  // float R2 = 7500.0;
//  float vOUT = (value * 3.3) / 1024.0;
//   float vIN = vOUT / (R2/(R1+R2))
  for(int v=0;v<25;v++){
    value = analogRead(pinSensorActive);
    // Serial.println(value);
    if(value>62.06){ //precompute value
      Serial.println("isPrinterOn On");
      return true;
    }
    delay(50); //delay dibuat kecil agar lebih mengakomodasi blinking
  }
    Serial.println("isPrinterOn OFF");
    return false;
}






