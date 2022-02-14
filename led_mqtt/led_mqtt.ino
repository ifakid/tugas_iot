#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "<SSID>";  // Change to match your SSID
const char* password = "<Password>";  // Change to match your password

const char* mqtt_server = "<MQTT Broker IP>";

const int ledPin = 2;
const int pushButton = 15;

const long period = 5000;  // Message publishing delay in millis
int frequency = 10;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long lastBlink = 0;

int buttonState = LOW;
int ledState = LOW;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(pushButton, INPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  pinMode(pushButton, OUTPUT);
}

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // For debugging
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

unsigned long calculateDelay() {
  // 1Hz = 1 cycle/second = switch led state every 0.5s or 500ms
  // 10Hz = 10 cycle/second = switch led state every 0.05s or 50 ms
  return(1000000ul / (frequency * 2));  // Count delay in microseconds
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = micros();
  unsigned long del = calculateDelay();
  if (now - lastMsg > period * 1000) {  // Convert period to micros
    lastMsg = now;

    String msgStr = "13518109: ";
    msgStr = msgStr + frequency;
    msgStr = msgStr +"Hz";
    
    unsigned int arrLen = msgStr.length() + 1;
    char msgArr[arrLen];
    msgStr.toCharArray(msgArr, arrLen); // Can only publish array of char using MQTT
    client.publish("test", msgArr);
  }
  
  if (now - lastBlink > del) {
    lastBlink = now;
    // To check delay between led switching
    // Serial.print(del);
    // Serial.println("us");
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }

  int readButton = digitalRead(pushButton);
  if (readButton != buttonState) {  // Assumption: can only press the button (not hold) to change frequency
     if (readButton == HIGH) {
        frequency++;
     }
     buttonState = !buttonState;
  }
}
