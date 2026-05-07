#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int RELAY_PIN = 2;
const int BUZZER_PIN = 5;
const int VOLT_PIN = A2;      // Analog input connected to ZMPT101B OUT
const int SWITCH_PIN = 7;

// --- CALIBRATION ---
// This ratio converts ADC RMS to real VRMS for the sensor + divider used
const float CALIBRATION_FACTOR = 2.96;
float smoothed_Vrms = 0.0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  // Configure pins before changing their output states
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Ensure relay starts OFF (Active Low)

  pinMode(BUZZER_PIN, OUTPUT);
  // Internal Pull-Up to prevent floating pins on the switch
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  // Analog pin used as input for voltage sensing
  pinMode(VOLT_PIN, INPUT);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Halt if OLED fails
  }
  
  // Startup beep test
  tone(BUZZER_PIN, 2000, 200); 
  delay(250);
  tone(BUZZER_PIN, 2500, 200); 

  display.clearDisplay();
  display.setTextColor(WHITE);
}

// --- PURE CODE TRUE RMS ALGORITHM ---
float getVrms() {
  unsigned long start_time = millis();
  unsigned long num_samples = 0;
  unsigned long long sum_squares = 0ULL; // avoid overflow when squaring many samples
  
  // Sample for exactly 40ms (2 full 50Hz cycles)
  while((millis() - start_time) < 40) {
    // 1. Read the raw analog pin directly
    long raw_adc = analogRead(VOLT_PIN);
    
    // 2. Strip away the 2.5V hardware DC offset (512 on Nano)
    long ac_val = raw_adc - 512; 
    
    // 3. Square the wave and add to total
    sum_squares += (unsigned long long)(ac_val * ac_val);
    num_samples++;
  }
  
  // Root Mean Square formula
  float rms_adc = sqrt((float)sum_squares / (float)num_samples);
  
  // Convert ADC RMS to Real Voltage
  float Vrms = rms_adc * CALIBRATION_FACTOR;
  
  if (Vrms < 20) Vrms = 0; // Ignore tiny ghost noise
  return Vrms;
}

void loop() {
  float raw_Vrms = getVrms();

  // --- FAULT SIMULATION LOGIC (D7) ---
  // Using LOW because of INPUT_PULLUP
  bool simActive = (digitalRead(SWITCH_PIN) == LOW);
  if (simActive && raw_Vrms > 90) {
    raw_Vrms = 160.0; // Force voltage into the Brownout trip zone
  }

  // --- SMOOTHING FILTER ---
  if (smoothed_Vrms < 20 || abs(raw_Vrms - smoothed_Vrms) > 40) {
    smoothed_Vrms = raw_Vrms; // Snap instantly on big real changes
  } else {
    smoothed_Vrms = (raw_Vrms * 0.10) + (smoothed_Vrms * 0.90); // Smooth minor noise
  }

  // --- OLED DISPLAY UI ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  
  if (simActive) {
    display.print("SIM: FAULT DETECTED");
  } else {
    display.print("SMART AC GUARD");
  }

  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print(smoothed_Vrms, 1); 
  display.print(" V");

  display.setTextSize(1);
  display.setCursor(0, 55);

  // --- LOGIC: PROTECT & ALARM ---
  const float TRIP_LOW = 90.0;
  const float TRIP_MIN = 180.0;
  const float TRIP_MAX = 250.0;

  if (smoothed_Vrms <= TRIP_LOW) {
    digitalWrite(RELAY_PIN, HIGH);       // OFF (Disconnect Load)
    noTone(BUZZER_PIN);                  // SILENCE ALARM
    display.print("STATUS: NO INPUT");
  }
  else if (smoothed_Vrms < TRIP_MIN || smoothed_Vrms > TRIP_MAX) {
    digitalWrite(RELAY_PIN, HIGH);       // TRIP RELAY (OFF)
    tone(BUZZER_PIN, 1000);              // SOUND ALARM (1000Hz)

    if (simActive) {
      display.print("PROTECTING LOAD...");
    } else {
      display.print("STATUS: TRIP!");
    }
  } 
  else {
    digitalWrite(RELAY_PIN, LOW);        // HEALTHY (Connect load)
    noTone(BUZZER_PIN);                  // SILENCE ALARM
    display.print("STATUS: HEALTHY");
  }

  display.display();
  delay(500); // Wait half a second before updating again


}
