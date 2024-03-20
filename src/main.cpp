
#include <Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

byte config[7][2] = {
    {0, 180},
    {0, 180},
    {0, 180},
    {0, 180},
    {0, 180},
    {0, 180},
    {0, 180},
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

Servo servo[7];

struct repeating_timer timer;

const char compile_date[] = __DATE__ " " __TIME__;

void initWiFi();
void handleRoot();

void handleConfig();
void handleConfigPost();

void handleState();
void handleStatePost();

bool saveConfig(struct repeating_timer *t);

bool setupDone = false;

void setup()
{
  Serial.begin(115200);

  Serial.println("setup()");

  LittleFS.begin();

  pinMode(PIN_LED, OUTPUT);

  initWiFi();
  setupDone = true;
}

void setup1()
{
  //   myservo.attach(3, 220, 2200, 90);
  // myservo2.attach(0, 0, 2200, 90);
  for (int s = 0; s < 7; s++)
  {
    servo[s].attach(s, 200, 3000, 90);
  }

  EEPROM.begin(14);
  for (int s = 0; s < 7; s++)
  {
    for (int p = 0; p < 2; p++)
    {
      int addr = s * 2 + p;
      config[s][p] = EEPROM.read(addr);
    }
  }
  add_repeating_timer_ms(500, saveConfig, NULL, &timer);

  while (!setupDone)
  {
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
  }
  digitalWrite(PIN_LED, HIGH);
}

bool saveConfig(struct repeating_timer *t)
{
  bool needsCommint = false;
  for (int s = 0; s < 7; s++)
  {
    for (int p = 0; p < 2; p++)
    {
      int addr = s * 2 + p;
      byte v = EEPROM.read(addr);
      if (config[s][p] != v)
      {
        EEPROM.write(addr, config[s][p]);
        needsCommint = true;
      }
    }
  }

  if (needsCommint)
  {
    if (EEPROM.commit())
    {
      Serial.println("EEPROM successfully committed");
    }
    else
    {
      Serial.println("ERROR! EEPROM commit failed");
    }
  }

  return true;
}

void initWiFi()
{
  WiFi.begin("Amiga500", "poipoi1234567890");
  while (WiFi.status() != WL_CONNECTED)
  {
  }

  Serial.println("");
  Serial.print("WiFi connected at IP Address ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("picow"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/config", HTTP_POST, handleConfigPost);
  server.on("/state", HTTP_POST, handleStatePost);
  server.on("/config", handleConfig);
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

void handleConfigPost()
{
  if (server.args() == 1)
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg(0));
    if (error)
    {
      server.send(500, "text/html", error.f_str());
      return;
    }
    for (int i = 0; i < doc.size(); i++)
    {
      config[i][0] = doc[i][0];
      config[i][1] = doc[i][1];
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
  // Serial.println("------");
  for (int s = 0; s < 7; s++)
  {
    int stt = state[s];
    int pos = config[s][stt];
    int old_pos = servo[s].read();
    // Serial.printf("%d - %d (%d)(%d)\n", s, pos, old_pos, servo[s].readMicroseconds());
    if (old_pos < pos)
    {
      servo[s].write(old_pos + 1);
    }
    else if (old_pos > pos)
    {
      servo[s].write(old_pos - 1);
    }
  }
  delay(15);
  // delay(1000);
}

// Serial.print("state: ");
// for (auto &&s : state)
// {
//   Serial.print(s);
//   Serial.print(", ");
// }
// Serial.println("");

// Serial.print("config: ");
// for (auto &&c : config)
// {
//   for (auto &&v : c)
//   {
//     Serial.print(v);
//     Serial.print(", ");
//   }
// }
// Serial.println("");
// delay(1000);

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
