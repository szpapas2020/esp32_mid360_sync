#include <Arduino.h>

// Pin definitions
#define PIN_10HZ 16  // Pin 10 for 10Hz pulse
#define PIN2_10HZ 18  // Pin 10 for 10Hz pulse
#define PIN3_10HZ 33  // Pin 10 for 10Hz pulse
#define PIN_1HZ  35  // Pin 11 for 1Hz pulse
#define PIN_GPIO17 17  // GPIO17 follows LED_BUILTIN state

#define PIN_GPIO37 37  // GPIO37 initialized to HIGH for 3.3V
#define PIN_GPIO39 39  // GPIO39 initialized to LOW for GND
#define PIN_GPRMC_TX 34 // Pin 34 for GPRMC UART TX

// Built-in LED pin (ESP32-S2: GPIO 38 for Lolin S2 Mini, or use LED_BUILTIN if defined)
#ifndef LED_BUILTIN
#define LED_BUILTIN 38  // Default for ESP32-S2
#endif

// Timing constants
#define PULSE_WIDTH_US 1000      // 1ms pulse width in microseconds for 10Hz
#define PERIOD_10HZ_US 100000    // 100ms period for 10Hz (1000000/10)
#define PULSE_1HZ_WIDTH_US 100000  // 100ms pulse width for 1Hz pulse

// Timer handle for 10Hz pulse (master timer)
hw_timer_t *timer10Hz = NULL;

// Counter for 1Hz pulse (counts 10Hz pulses)
volatile uint8_t pulseCounter = 0;
// Timestamp for 1Hz pulse start time (in microseconds)
volatile uint64_t pulse1HzStartTime = 0;

// GPS Time variables
volatile int gpsHour = 12;
volatile int gpsMinute = 0;
volatile int gpsSecond = 0;
volatile bool shouldSendGPRMC = false;

// Helper to calculate NMEA checksum
String getChecksum(String s) {
  byte checksum = 0;
  for (int i = 0; i < s.length(); i++) {
    checksum ^= s[i];
  }
  String hex = String(checksum, HEX);
  hex.toUpperCase();
  if (hex.length() < 2) hex = "0" + hex;
  return hex;
}

// Interrupt service routine - handles both 10Hz and 1Hz pulses
void IRAM_ATTR onTimer() {
  static bool pulse10HzState = false;
  static bool pulse1HzState = false;
  
  if (!pulse10HzState) {
    // Start 10Hz pulse - set pin HIGH
    digitalWrite(PIN_10HZ, HIGH);   
    digitalWrite(PIN2_10HZ, HIGH);   
    digitalWrite(PIN3_10HZ, HIGH);   
    
    pulse10HzState = true;
    
    // Increment counter for 1Hz pulse
    pulseCounter++;
    if (pulseCounter >= 10) {
      // Time for 1Hz pulse - set pin HIGH
      digitalWrite(PIN_1HZ, HIGH); 
      pulseCounter = 0;  // Reset counter
      
      // Increment GPS time
      gpsSecond++;
      if (gpsSecond >= 60) {
        gpsSecond = 0;
        gpsMinute++;
        if (gpsMinute >= 60) {
          gpsMinute = 0;
          gpsHour++;
          if (gpsHour >= 24) {
            gpsHour = 0;
          }
        }
      }
      shouldSendGPRMC = true;
      
    } else if (pulseCounter == 1) {
      // 100ms passed since 1Hz pulse started - set pin LOW
      digitalWrite(PIN_1HZ, LOW);
    }
    
    // Set timer to turn off pulses after 1ms
    timerAlarmWrite(timer10Hz, PULSE_WIDTH_US, true);
  } else {
    // End pulses - set pins LOW
    digitalWrite(PIN_10HZ, LOW);
    digitalWrite(PIN2_10HZ, LOW);
    digitalWrite(PIN3_10HZ, LOW);
    pulse10HzState = false;
    
    // Set timer back to low period (full period minus pulse width)
    timerAlarmWrite(timer10Hz, PERIOD_10HZ_US - PULSE_WIDTH_US, true);
  }
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S2 Pulse Generator");
  Serial.println("Pin 16: 10Hz pulse (1ms high)");
  Serial.println("Pin 18: 10Hz pulse (1ms high)");
  Serial.println("Pin 33: 10Hz pulse (1ms high)");
  Serial.println("Pin 35: 1Hz pulse (100ms high)");
  Serial.println("Built-in LED: Blinking at 1Hz");
  Serial.println("GPIO17: Follows LED_BUILTIN state");
  Serial.println("GPIO37: Initialized to HIGH (3.3V)");
  Serial.println("GPIO39: Initialized to LOW (GND)");
  Serial.println("Pin 21: GPRMC UART TX (9600 baud)");
  Serial.println("Pulses are perfectly aligned!");
  
  // Configure pins as outputs
  pinMode(PIN_10HZ, OUTPUT);
  pinMode(PIN_1HZ, OUTPUT);
  pinMode(PIN2_10HZ, OUTPUT);
  pinMode(PIN3_10HZ, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_GPIO17, OUTPUT);
  pinMode(PIN_GPIO37, OUTPUT);
  pinMode(PIN_GPIO39, OUTPUT);
  
  // Initialize pins to LOW
  digitalWrite(PIN_10HZ, LOW);
  digitalWrite(PIN_1HZ, LOW);
  digitalWrite(PIN2_10HZ, LOW);
  digitalWrite(PIN3_10HZ, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_GPIO17, LOW);
  
  // Initialize GPIO37 to HIGH (3.3V)
  digitalWrite(PIN_GPIO37, HIGH);
  // Initialize GPIO39 to LOW (GND)
  digitalWrite(PIN_GPIO39, LOW);
  
  // Initialize Serial1 for GPRMC (TX only)
  Serial1.begin(9600, SERIAL_8N1, -1, PIN_GPRMC_TX);
  
  // Configure timer for 10Hz (timer 0)
  // Prescaler 80 gives 1MHz clock (80MHz / 80 = 1MHz)
  timer10Hz = timerBegin(0, 80, true);
  timerAttachInterrupt(timer10Hz, &onTimer, true);  // true = edge trigger
  timerAlarmWrite(timer10Hz, PERIOD_10HZ_US - PULSE_WIDTH_US, true);
  timerAlarmEnable(timer10Hz);
  
  Serial.println("Pulse generators started!");
  Serial.println("1Hz pulse aligns with every 10th 10Hz pulse.");
}

void loop() {
  // LED blink function - 500ms on, 500ms off (1Hz blink rate)
  static unsigned long previousMillis = 0;
  static bool ledState = false;
  const unsigned long blinkInterval = 500;  // 500ms interval
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
    // GPIO17 follows LED_BUILTIN state
    // digitalWrite(PIN_GPIO17, ledState);
    // Serial.println(ledState ? "LED ON" : "LED OFF");
  }
  
  // Handle GPRMC sending
  if (shouldSendGPRMC) {
    shouldSendGPRMC = false;
    
    char timeStr[10];
    // Format: hhmmss.ss
    sprintf(timeStr, "%02d%02d%02d.00", gpsHour, gpsMinute, gpsSecond);
    
    // Construct GPRMC sentence
    // $GPRMC,hhmmss.ss,A,lat,N,lon,E,speed,course,date,mag_var,mode*cs
    // Using fixed location (Latitude 30.0, Longitude 120.0) and date (01/01/2024)
    String content = "GPRMC," + String(timeStr) + ",A,3000.0000,N,12000.0000,E,0.0,0.0,010124,,,A";
    String checksum = getChecksum(content);
    String nmea = "$" + content + "*" + checksum + "\r\n";
    
    Serial1.print(nmea);
    // Also print to debug serial
    // Serial.print("Sent GPRMC: ");
    // Serial.print(nmea);
  }
}