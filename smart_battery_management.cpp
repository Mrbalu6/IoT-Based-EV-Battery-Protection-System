////////////// NODEMCU CODE //////////////

#define BLYNK_TEMPLATE_ID "TMPL3GhI-_DFf"
#define BLYNK_TEMPLATE_NAME "EV BMS"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

BlynkTimer timer;

int s_charge = D3;
int F_charge = D2;

String myString;
char rdata;

String voltage;
String current;
String power;
String temperature;
String percentage;

bool tempFlag = true;
bool batteryFlag = false;

void setup()
{
  Serial.begin(115200);

  pinMode(s_charge, INPUT);
  pinMode(F_charge, INPUT);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  timer.setInterval(1000L, sendData);
}

void loop()
{
  Blynk.run();
  timer.run();

  while (Serial.available())
  {
    rdata = Serial.read();
    myString += rdata;

    if (rdata == '\n')
    {
      voltage = getValue(myString, ',', 0);
      current = getValue(myString, ',', 1);
      power = getValue(myString, ',', 2);
      temperature = getValue(myString, ',', 3);
      percentage = getValue(myString, ',', 4);

      myString = "";
    }
  }
}

void sendData()
{
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, temperature);
  Blynk.virtualWrite(V4, percentage);

  int slowCharge = digitalRead(s_charge);
  int fastCharge = digitalRead(F_charge);

  if (slowCharge == LOW)
  {
    Blynk.virtualWrite(V5, "SLOW CHARGING");
  }

  if (fastCharge == LOW)
  {
    Blynk.virtualWrite(V5, "FAST CHARGING");
  }

  if (slowCharge == HIGH && fastCharge == HIGH)
  {
    Blynk.virtualWrite(V5, "NO CHARGING");
  }

  if (temperature.toFloat() >= 40 && tempFlag)
  {
    Blynk.logEvent("temp");
    tempFlag = false;
  }

  if (temperature.toFloat() < 40)
  {
    tempFlag = true;
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

////////////// ARDUINO CODE //////////////

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

SoftwareSerial nodemcu(9, 8);

int batteryVoltagePin = A1;
int currentPin = A2;
int tempPin = A6;

int slowRelay = A3;
int fastRelay = A4;
int loadRelay = A5;

int buzzer = 13;

float R1 = 100000.0;
float R2 = 10000.0;

float batteryVoltage;
float currentValue;
float powerValue;
float temperatureC;

void setup()
{
  Serial.begin(9600);
  nodemcu.begin(115200);

  lcd.begin(16, 2);

  pinMode(slowRelay, OUTPUT);
  pinMode(fastRelay, OUTPUT);
  pinMode(loadRelay, OUTPUT);

  pinMode(buzzer, OUTPUT);

  lcd.setCursor(0,0);
  lcd.print("EV BMS SYSTEM");

  delay(2000);

  lcd.clear();
}

void loop()
{
  readBatteryVoltage();
  readCurrent();
  readTemperature();

  powerValue = batteryVoltage * currentValue;

  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(batteryVoltage);

  lcd.setCursor(9,0);
  lcd.print("I:");
  lcd.print(currentValue);

  lcd.setCursor(0,1);
  lcd.print("T:");
  lcd.print(temperatureC);

  lcd.print("C ");

  if (temperatureC >= 45)
  {
    digitalWrite(buzzer, HIGH);

    digitalWrite(slowRelay, LOW);
    digitalWrite(fastRelay, LOW);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("TEMP HIGH");

    lcd.setCursor(0,1);
    lcd.print("CHARGER OFF");

    delay(1000);
  }
  else
  {
    digitalWrite(buzzer, LOW);
  }

  String data = "";

  data += String(batteryVoltage);
  data += ",";
  data += String(currentValue);
  data += ",";
  data += String(powerValue);
  data += ",";
  data += String(temperatureC);
  data += ",";
  data += String(85);

  Serial.println(data);
  nodemcu.println(data);

  delay(1000);
}

void readBatteryVoltage()
{
  int value = analogRead(batteryVoltagePin);

  float vout = (value * 5.0) / 1024.0;

  batteryVoltage = vout / (R2 / (R1 + R2));

  if (batteryVoltage < 0.1)
  {
    batteryVoltage = 0.0;
  }
}

void readCurrent()
{
  int sensorValue = analogRead(currentPin);

  float adcVoltage = (sensorValue / 1024.0) * 5000;

  currentValue = ((adcVoltage - 2500) / 185);
}

void readTemperature()
{
  long value = analogRead(tempPin);

  temperatureC =
    3950 / (log((1024.0 * 10 / value - 10) / 10) + 3950 / 298.0) - 273.0;
}
