
#include <Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <LittleFS.h>

WebServer server(80);

Servo myservo;
Servo myservo2;

void initWiFi();
void handleRoot();

void setup()
{
  Serial.begin(115200);

  myservo.attach(3, 220, 2200, 90);
  myservo2.attach(0, 0, 2200, 90);

  Serial.println("setup()");

  LittleFS.begin();

  pinMode(PIN_LED, OUTPUT);

  initWiFi();
}

void initWiFi()
{
  digitalWrite(PIN_LED, LOW);

  WiFi.begin("Amiga500", "poipoi1234567890");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected at IP Address ");
  Serial.println(WiFi.localIP());

  digitalWrite(PIN_LED, HIGH);

  if (MDNS.begin("picow"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot()
{
  File file = LittleFS.open("/index.html", "r");
  server.send(200, "text/html", file.readString());
}

void loop1()
{
  server.handleClient();
  MDNS.update();
}

void loop()
{
  int pos;
  for (pos = 0; pos <= 180; pos += 1)
  {
    myservo.write(pos);
    myservo2.write(pos);
    delay(15);
  }

  // Serial.println("1");
  // for (int i = 0; i < 100; i++)
  // {
  //   delay(20);
  // }

  // myservo.write(90);
  // myservo2.write(90);
  // Serial.println("2");

  // for (int i = 0; i < 100; i++)
  // {
  //   delay(20);
  // }
}
