
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Replace with your network credentials
const char* ssid = "brrrrr";
const char* password = "12345678";
const int thresh1 = 30;
// Initialize Telegram BOT
#define BOTtoken "5732756180:AAHq59gbm6IX8FMQExtnxX-7QI6Nmc6Q3Qg"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "2019810169"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int oneWireBus = 4;     
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

//Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) 
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) 
  {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    sensors.requestTemperatures(); 
    float temperature = sensors.getTempCByIndex(0);
    Serial.println(temperature);
    if(temperature>30) 
    {
      text = "/monitor";
    }
  
    String from_name = bot.messages[i].from_name;
    if (text == "/monitor") 
    { 
      String msg = "Sending next 5 readings...";
      bot.sendMessage(CHAT_ID, msg, "");
      for(int i=0; i<5; i++)
      {
          sensors.requestTemperatures(); 
          float temperature = sensors.getTempCByIndex(0);  //in ºC
          if(temperature > thresh1)
          {
            break;
          }
          String msg = "Reading #" + String(i+1) + ": " + String(temperature) + " ºC \n";
          bot.sendMessage(CHAT_ID, msg, "");
          delay(1000);
      }
    }  
  }
}

void setup() {
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() 
{    
  sensors.requestTemperatures(); 
  float temp = sensors.getTempCByIndex(0);  //in ºC
  Serial.println(temp);
  if(temp > thresh1)
  { 
    String alert_msg = "ALERT! Temperature threshold " + String(thresh1) + "ºC exceeded";
    bot.sendMessage(CHAT_ID, alert_msg, "");
    String msg = "Current Temperature: " + String(temp) + " ºC \n";
    bot.sendMessage(CHAT_ID, msg, "");
    String capture = "Capturing next 5 readings...";
    bot.sendMessage(CHAT_ID, capture, "");
    for(int j = 0; j<5; j++)
    { 
      sensors.requestTemperatures(); 
      temp = sensors.getTempCByIndex(0);  //in ºC
      Serial.println(temp);
      msg = "Reading #" + String(j+1) + ": " + String(temp) + " ºC \n";
      bot.sendMessage(CHAT_ID, msg, "");
      if(temp < 30)
      {
        msg = "Temperture back to normal";
        bot.sendMessage(CHAT_ID, msg, "");
        break;
      }
      delay(1000);  

      if(j==4)
      {
        msg = "OMG! System Failure";
        bot.sendMessage(CHAT_ID, msg, "");
      }
    }
    
  }

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  if(numNewMessages)
  {
    Serial.println("got response");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  delay(1000);
}
