#include <WiFi.h>
#include <ESPmDNS.h>

const char* ssid = "";
const char* password = "";

WiFiServer server(80);

#define RELAY_PIN 23

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF by default

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Start mDNS
  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    return;
  }
  Serial.println("mDNS started: http://esp32.local");

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    if (req.indexOf("/on") != -1) {
      digitalWrite(RELAY_PIN, LOW);
    } else if (req.indexOf("/off") != -1) {
      digitalWrite(RELAY_PIN, HIGH);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html\r\n");
    client.println("<html><body>");
    client.println("<h1>ESP32 Relay Control</h1>");
    client.println("<p><a href=\"/on\"><button>ON</button></a></p>");
    client.println("<p><a href=\"/off\"><button>OFF</button></a></p>");
    client.println("</body></html>");
    
    client.stop();
  }
}
