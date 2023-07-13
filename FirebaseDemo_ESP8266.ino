
#include <HCSR04.h>
#include <ESP8266WiFi.h>
WiFiClient client;

#define WIFI_SSID "Nick Bateman" // nama wifi/hotspot
#define WIFI_PASSWORD "angela271111" // password wifi/hotspot


//ThingSpeak Config
String host = "api.thingspeak.com"; // connect ke ThingSpeak
String writeAPIKey = "6F8XARUJ42C6W47W"; //Ganti API Key di Channel Masing Masing
String request_string;

#include <FirebaseArduino.h>
#define FIREBASE_HOST "yukcoba-83a46-default-rtdb.firebaseio.com" //web sesuai masing - masing
#define FIREBASE_AUTH "LHEBiLveBJdNHMFmio0GtedQA5RGfdIz88RaaQ7Z" //secret sesuai masing - masing

int triggerPin = D0; // Jumper pin trigger sensor ke pin D0 NodeMCU
int echoPin = D1; // Jumper pin echo sensor ke pin D1 NodeMCU
int buzzer = D6; //Jumper pin buzzer ke pin D6 NodeMCU
long air;


void setup() {
  Serial.begin(9600);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);  
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  //connect to firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //pinmode
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer,OUTPUT);
  
}

void loop() {

 // Menghitung ketinggian air
  float duration, ketinggian,air;
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);         // delay tiap pengukuran (bisa diset sendiri)
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);        // delay tiap pengukuran (bisa diset sendiri)
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  ketinggian = (duration/2) / 29.1;
  air = 11.00-ketinggian;
  Serial.println("ketinggian air :"); //kirim tulisan 'Ketinggian air :' ke serial monitor
  Serial.print(air); //kirim tulisan 'air' ke serial monitor
  Serial.println(" cm"); //kirim tulisan 'cm' ke serial monitor
  delay(400);

// Mengirim pembacaan sensor ke firebase
  Firebase.pushFloat("/Ketinggian air (cm)/", air);

  if (air < 0) {
       Firebase.setString("Ketinggian air/", "ERROR"); 
  }
  if (air <= 7.00) {
       Firebase.setString("Ketinggian air/", "masih aman"); 
  }
  if (air > 7.00) {
   Firebase.setString("Ketinggian air/", "TERLALU BERLEBIH, BAHAYA!!!"); 
  }
  

// BUZZER
  if (air <= 7.00){
    digitalWrite(buzzer,LOW);
  }
  else if (air>7.00){
    digitalWrite(buzzer,HIGH);
  }
  
// Mengirim ke ThinkSpeak
    if (client.connect(host, 80))
  {
    request_string = "/update?key=" + writeAPIKey 
                    + "&field1=" + air;
                    
    Serial.println(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
                 
    client.print(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    
    while (client.available() == 0)
    {
      if (millis() - timeout > 5000)
      {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    while (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("Closing connection");
  }

  
  }
