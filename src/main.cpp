
#include <Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <LittleFS.h>

WebServer server(80);

Servo myservo;
Servo myservo2;

const char compile_date[] = __DATE__ " " __TIME__;

void initWiFi();
void handleRoot();
void pp();

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
  for (size_t i = 0; i < 5; i++)
  {
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
  }

  WiFi.begin("Amiga500", "poipoi1234567890");
  while (WiFi.status() != WL_CONNECTED)
  {
  }
  digitalWrite(PIN_LED, HIGH);

  Serial.println("");
  Serial.print("WiFi connected at IP Address ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("picow"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.addHook([](const String &method, const String &url, WiFiClient *client, WebServer::ContentTypeFunction contentType)
                 { 
                  Serial.print(method);
                  Serial.print(" - ");
                  Serial.println(url);
                  return WebServer::CLIENT_REQUEST_CAN_CONTINUE; });

  server.begin();
  Serial.println("HTTP server started");
}

void pp()
{
  Serial.println("handleRoot()");
}

void handleRoot()
{
  Serial.println("handleRoot()");
  File file = LittleFS.open("/index.html", "r");
  String index = file.readString();
  index.replace("#######", compile_date);
  server.send(200, "text/html", index);
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
