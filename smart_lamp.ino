#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ssid = "";
const char* password = "";

WiFiServer server(80);
#define RELAY_PIN 23

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 60000);  // UTC+3

bool autoMode = true;

// Auto ON/OFF time in hours and minutes
int autoStartHour = 6;
int autoStartMinute = 0;
int autoEndHour = 21;
int autoEndMinute = 0;

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

  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    return;
  }

  Serial.println("mDNS started: http://esp32.local");

  timeClient.begin();
  server.begin();
}

void handleAutoMode() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int now = currentHour * 60 + currentMinute;
  int start = autoStartHour * 60 + autoStartMinute;
  int end = autoEndHour * 60 + autoEndMinute;

  if (autoMode) {
    if (start < end) {
      // Normal range: e.g. 06:30 to 21:15
      if (now >= start && now < end) {
        digitalWrite(RELAY_PIN, LOW);  // ON
      } else {
        digitalWrite(RELAY_PIN, HIGH); // OFF
      }
    } else {
      // Overnight range: e.g. 21:00 to 06:00
      if (now >= start || now < end) {
        digitalWrite(RELAY_PIN, LOW);  // ON
      } else {
        digitalWrite(RELAY_PIN, HIGH); // OFF
      }
    }
  }
}

void loop() {
  handleAutoMode();

  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    // Manual control
    if (req.indexOf("/on") != -1) {
      autoMode = false;
      digitalWrite(RELAY_PIN, LOW);
    } else if (req.indexOf("/off") != -1) {
      autoMode = false;
      digitalWrite(RELAY_PIN, HIGH);
    } else if (req.indexOf("/auto") != -1) {
      autoMode = true;
    }

    // Handle time form input: /set?sh=6&sm=30&eh=21&em=15
    if (req.indexOf("/set?") != -1) {
      autoStartHour = getParamValue(req, "sh", autoStartHour);
      autoStartMinute = getParamValue(req, "sm", autoStartMinute);
      autoEndHour = getParamValue(req, "eh", autoEndHour);
      autoEndMinute = getParamValue(req, "em", autoEndMinute);
      autoMode = true;
    }

    // Send webpage
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html\r\n");
    client.println("<html><body>");
    client.println("<h2>ESP32 Relay Control</h2>");
    client.println("<p><a href=\"/on\"><button>Manual ON</button></a></p>");
    client.println("<p><a href=\"/off\"><button>Manual OFF</button></a></p>");
    client.println("<p><a href=\"/auto\"><button>Enable Auto Mode</button></a></p>");
    client.println("<h3>Set Auto Time Range</h3>");
    client.println("<form action=\"/set\" method=\"get\">");

    client.printf("Start Hour: <input name=\"sh\" type=\"number\" min=\"0\" max=\"23\" value=\"%d\"><br>", autoStartHour);
    client.printf("Start Minute: <input name=\"sm\" type=\"number\" min=\"0\" max=\"59\" value=\"%d\"><br>", autoStartMinute);
    client.printf("End Hour: <input name=\"eh\" type=\"number\" min=\"0\" max=\"23\" value=\"%d\"><br>", autoEndHour);
    client.printf("End Minute: <input name=\"em\" type=\"number\" min=\"0\" max=\"59\" value=\"%d\"><br>", autoEndMinute);

    client.println("<input type=\"submit\" value=\"Set Auto Time\">");
    client.println("</form>");

    client.printf("<p>Auto Mode: %s</p>", autoMode ? "ON" : "OFF");
    client.printf("<p>Relay is currently: %s</p>", digitalRead(RELAY_PIN) == LOW ? "ON" : "OFF");
    client.printf("<p>Auto ON from %02d:%02d to %02d:%02d</p>", autoStartHour, autoStartMinute, autoEndHour, autoEndMinute);
    client.println("</body></html>");
    client.stop();
  }

  delay(1000);
}

// Helper to extract int param
int getParamValue(String req, String param, int defaultVal) {
  int index = req.indexOf(param + "=");
  if (index != -1) {
    int endIndex = req.indexOf("&", index);
    String valStr = (endIndex != -1) ? req.substring(index + param.length() + 1, endIndex)
                                     : req.substring(index + param.length() + 1);
    return valStr.toInt();
  }
  return defaultVal;
}
