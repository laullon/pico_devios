
#include <Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

byte config[7][2] = {
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
};

byte state[7] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

WebServer server(80);

Servo myservo;
Servo myservo2;

const char compile_date[] = __DATE__ " " __TIME__;

void initWiFi();
void handleRoot();
void handleConfig();
void handleState();
void handleStatePost();

void setup()
{
  Serial.begin(115200);

  myservo.attach(3, 220, 2200, 90);
  myservo2.attach(0, 0, 2200, 90);

  Serial.println("setup()");

  LittleFS.begin();

  pinMode(PIN_LED, OUTPUT);

  initWiFi();

  for (int s = 0; s < 7; s++)
  {
    for (int p = 0; p < 2; p++)
    {
      config[s][p] = s * 2 + p;
    }
  }
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
  server.on("/config", handleConfig);
  // server.on("/state", HTTP_GET,handleState);
  server.on("/state", HTTP_POST, handleStatePost);
  server.on("/state", handleState);

  server.addHook([](const String &method, const String &url, WiFiClient *client, WebServer::ContentTypeFunction contentType)
                 { 
                  Serial.print(method);
                  Serial.print(" - ");
                  Serial.println(url);
                  return WebServer::CLIENT_REQUEST_CAN_CONTINUE; });

  server.begin();
  Serial.println("HTTP server started");
}

void handleState()
{
  String out;
  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();
  for (auto &&i : state)
  {
    array.add(i);
  }

  serializeJson(doc, out);
  server.send(200, "text/html", out);
}

void handleStatePost()
{
  Serial.println("handleStatePost()");
  if (server.args() == 1)
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg(0));
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      server.send(500, "text/html", error.f_str());
      return;
    }
    for (int i = 0; i < doc.size(); i++)
    {
      int v = doc[i];
      state[i] = v;
    }
  }
  server.send(200, "text/html", "ok");
}

void handleConfig()
{
  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();
  for (int s = 0; s < 7; s++)
  {
    for (int p = 0; p < 2; p++)
    {
      array.add(config[s][p]);
    }
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "text/html", out);
}

void handleRoot()
{
  Serial.println("handleRoot()");
  File file = LittleFS.open("/index.html", "r");
  String index = file.readString();
  index.replace("#######", compile_date);
  server.send(200, "text/html", index);
}

void loop()
{
  server.handleClient();
  MDNS.update();
}

void loop1()
{
  for (auto &&s : state)
  {
    Serial.print(s);
    Serial.print(", ");
  }
  Serial.println("");
  delay(1000);
  // int pos;
  // for (pos = 0; pos <= 180; pos += 1)
  // {
  //   myservo.write(pos);
  //   myservo2.write(pos);
  //   delay(15);
  // }

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
