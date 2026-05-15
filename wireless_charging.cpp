///////////// WIRELESS EV CHARGING + BMS CODE /////////////

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

SoftwareSerial ESP8266(10, 11);

#define RELAY 8
#define BUZZER 9
#define RESET_SW 12

#define TEMP_SENSOR A1
#define VOLT_SENSOR A0
#define CURRENT_SENSOR A2

float batteryVoltage = 0;
float temperature = 0;
float currentValue = 0;
float powerValue = 0;

int alarmFlag = 0;

void setup()
{
  lcd.begin(16, 2);

  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RESET_SW, INPUT_PULLUP);

  digitalWrite(RELAY, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.begin(9600);
  ESP8266.begin(9600);

  lcd.setCursor(0, 0);
  lcd.print("WIRELESS EV");

  lcd.setCursor(0, 1);
  lcd.print("CHARGER");

  delay(2000);
  lcd.clear();
}

void loop()
{
  readTemperature();
  readVoltage();
  readCurrent();

  powerValue = batteryVoltage * currentValue;

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C ");

  lcd.setCursor(9, 0);
  lcd.print("V:");
  lcd.print(batteryVoltage);

  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(currentValue);

  lcd.setCursor(9, 1);
  lcd.print("P:");
  lcd.print(powerValue);

  Serial.print("TEMP:");
  Serial.print(temperature);
  Serial.print("C  ");

  Serial.print("VOLT:");
  Serial.print(batteryVoltage);
  Serial.print("V  ");

  Serial.print("CURR:");
  Serial.print(currentValue);
  Serial.print("A  ");

  Serial.print("POWER:");
  Serial.print(powerValue);
  Serial.println("W");

  ESP8266.print(temperature);
  ESP8266.print(",");

  ESP8266.print(batteryVoltage);
  ESP8266.print(",");

  ESP8266.print(currentValue);
  ESP8266.print(",");

  ESP8266.println(powerValue);

  if (temperature > 50)
  {
    digitalWrite(RELAY, HIGH);

    if (alarmFlag == 0)
    {
      digitalWrite(BUZZER, HIGH);
    }

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("TEMP HIGH");

    lcd.setCursor(0, 1);
    lcd.print("CHARGER OFF");

    delay(1000);
  }
  else
  {
    digitalWrite(RELAY, LOW);
    digitalWrite(BUZZER, LOW);

    alarmFlag = 0;
  }

  if (digitalRead(RESET_SW) == LOW)
  {
    digitalWrite(BUZZER, LOW);
    alarmFlag = 1;
  }

  delay(1000);
}

void readTemperature()
{
  int value = analogAverage(TEMP_SENSOR);

  float mv = (value * 500.0) / 1023.0;

  temperature = mv;
}

void readVoltage()
{
  int value = analogAverage(VOLT_SENSOR);

  float voltage = value * (5.0 / 1024.0);

  batteryVoltage = voltage * 6.0;
}

void readCurrent()
{
  int sensorValue = analogAverage(CURRENT_SENSOR);

  float adcVoltage = (sensorValue / 1024.0) * 5000;

  currentValue = (adcVoltage - 2500) / 185;
}

int analogAverage(int pin)
{
  unsigned int total = 0;

  for (int i = 0; i < 32; i++)
  {
    total += analogRead(pin);
  }

  return total / 32;
}
