#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------------------- Pin Definitions --------------------
#define IR_PIN 15
#define BUZZER_PIN 4

// -------- RF Detector (AD8318 Interface) --------
#define RF_DETECTOR_PIN 34      // AD8318 analog output connected to ESP32 ADC
#define RF_THRESHOLD 1200       // RF power detection threshold (adjust experimentally)

// This detector is used to monitor RF signals mainly
// in the 2.4GHz and below range which includes:
// WiFi cameras, Bluetooth devices, wireless spy cameras
#define RF_MAX_FREQ 2400   // MHz

// -------------------- LCD --------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- OLED --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------------------- WiFi RSSI Range --------------------
const int MAX_RSSI = -30;
const int MIN_RSSI = -90;
const int FULL_STRENGTH_THRESHOLD = -30;

// -------------------- Timing Constants --------------------
const unsigned long BUZZER_INTERVAL = 400;
const unsigned long LCD_UPDATE_INTERVAL = 200;
const unsigned long OLED_UPDATE_INTERVAL = 200;
const unsigned long WIFI_SCAN_INTERVAL = 500;

// -------------------- Variables --------------------
bool irActive = false;
unsigned long lastBeep = 0;
bool buzzerState = false;
bool wifiFullStrength = false;

// RF detector value
int rfDetectorValue = 0;
bool rfDetected = false;

// Timing variables
unsigned long lastLcdUpdate = 0;
unsigned long lastOledUpdate = 0;
unsigned long lastWifiScan = 0;

// Cached values
int strongestRSSI = -100;
String lastLcdLine0 = "";
String lastLcdLine1 = "";

void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RF_DETECTOR_PIN, INPUT);

  analogReadResolution(12);  // ESP32 ADC resolution

  // I2C start
  Wire.begin(21, 22);

  // LCD Init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("RF/IR Detector");
  delay(1500);
  lcd.clear();

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (1) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.display();

  // WiFi scan mode only
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("System Ready");
}

// ------------------------------------------------
// Read RF power from AD8318 RF detector module
// ------------------------------------------------
void readRFDetector()
{
  rfDetectorValue = analogRead(RF_DETECTOR_PIN);

  // AD8318 converts RF signal strength into analog voltage
  // ESP32 reads that voltage through ADC

  Serial.print("RF Detector ADC Value: ");
  Serial.println(rfDetectorValue);

  // If RF power crosses threshold we consider RF activity present
  if(rfDetectorValue > RF_THRESHOLD)
  {
    rfDetected = true;

    Serial.println("RF activity detected using RF detector module");
    Serial.println("Monitoring band: <= 2.4 GHz");
  }
  else
  {
    rfDetected = false;
  }
}

void loop() {
  unsigned long currentTime = millis();

  // Read RF detector module
  readRFDetector();

  // ---------------- IR Detection ----------------
  checkIRDetection(currentTime);

  // ---------------- LCD Update ----------------
  if (currentTime - lastLcdUpdate >= LCD_UPDATE_INTERVAL) {
    updateLCD();
    lastLcdUpdate = currentTime;
  }

  // ---------------- WiFi Scan ----------------
  if (currentTime - lastWifiScan >= WIFI_SCAN_INTERVAL) {
    scanWiFiNetworks();
    lastWifiScan = currentTime;
  }

  // ---------------- OLED Update ----------------
  if (currentTime - lastOledUpdate >= OLED_UPDATE_INTERVAL) {
    updateOLED();
    lastOledUpdate = currentTime;
  }
}

void checkIRDetection(unsigned long currentTime) {
  bool currentIRState = (digitalRead(IR_PIN) == LOW);

  if (currentIRState) {
    if (!irActive) {
      irActive = true;
    }

    if (currentTime - lastBeep >= BUZZER_INTERVAL) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
      lastBeep = currentTime;
    }
  } 
  else {
    if (irActive) {
      irActive = false;
      if (!wifiFullStrength && !rfDetected) {
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }

  handleWiFiBuzzer(currentTime);
}

void handleWiFiBuzzer(unsigned long currentTime) {

  // RF detector alert
  if(rfDetected)
  {
    if (currentTime - lastBeep >= BUZZER_INTERVAL) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
      lastBeep = currentTime;
    }
  }

  else if (wifiFullStrength) {
    if (currentTime - lastBeep >= BUZZER_INTERVAL) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
      lastBeep = currentTime;
    }
  }

  else if (!irActive) {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void updateLCD() {

  String line0, line1;

  if (irActive) {
    line0 = "IR: ACTIVE      ";
  } else {
    line0 = "IR: INACTIVE    ";
  }

  if (rfDetected) {
    line1 = "RF DETECTOR ACT ";
  }
  else if (wifiFullStrength) {
    line1 = "RF: FULL STRNGTH";
  }
  else {
    line1 = "RF: SCANNING    ";
  }

  if (line0 != lastLcdLine0 || line1 != lastLcdLine1) {
    lcd.setCursor(0, 0);
    lcd.print(line0);
    lcd.setCursor(0, 1);
    lcd.print(line1);

    lastLcdLine0 = line0;
    lastLcdLine1 = line1;
  }
}

void scanWiFiNetworks() {

  int n = WiFi.scanNetworks(false, true, false, 200);

  if (n >= 0) {

    int newStrongestRSSI = -100;
    wifiFullStrength = false;

    for (int i = 0; i < n; i++) {

      if (WiFi.RSSI(i) > newStrongestRSSI) {
        newStrongestRSSI = WiFi.RSSI(i);
      }

    }

    if (newStrongestRSSI > -100) {

      strongestRSSI = newStrongestRSSI;

      if (strongestRSSI >= FULL_STRENGTH_THRESHOLD) {
        wifiFullStrength = true;
      }

    }

  } else {

    wifiFullStrength = false;

  }

  WiFi.scanDelete();

}

void updateOLED() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("RF SCANNER");

  display.drawRect(10, 20, 108, 12, SSD1306_WHITE);

  int barWidth = map(strongestRSSI, MIN_RSSI, MAX_RSSI, 0, 108);
  barWidth = constrain(barWidth, 0, 108);

  display.fillRect(11, 21, barWidth, 10, SSD1306_WHITE);

  display.setCursor(0, 40);
  display.print("ADC:");
  display.print(rfDetectorValue);

  display.setCursor(0, 50);
  display.println("Band <=2.4GHz");

  display.display();
}