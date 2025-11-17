#include <Arduino.h>

// Pin definitions
#define PIN_10HZ 16  // Pin 10 for 10Hz pulse
#define PIN_1HZ  18  // Pin 11 for 1Hz pulse
#define PIN_GPIO17 17  // GPIO17 follows LED_BUILTIN state
#define PIN_GPIO33 33  // GPIO33 initialized to HIGH

// Built-in LED pin (ESP32-S2: GPIO 38 for Lolin S2 Mini, or use LED_BUILTIN if defined)
#ifndef LED_BUILTIN
#define LED_BUILTIN 38  // Default for ESP32-S2
#endif

// Timing constants
#define PULSE_WIDTH_US 1000      // 1ms pulse width in microseconds
#define PERIOD_10HZ_US 100000    // 100ms period for 10Hz (1000000/10)

// Timer handle for 10Hz pulse (master timer)
hw_timer_t *timer10Hz = NULL;

// Counter for 1Hz pulse (counts 10Hz pulses)
volatile uint8_t pulseCounter = 0;

// Interrupt service routine - handles both 10Hz and 1Hz pulses
void IRAM_ATTR onTimer() {
  static bool pulse10HzState = false;
  static bool pulse1HzState = false;
  
  if (!pulse10HzState) {
    // Start 10Hz pulse - set pin HIGH
    digitalWrite(PIN_10HZ, HIGH);
    pulse10HzState = true;
    
    // Increment counter for 1Hz pulse
    pulseCounter++;
    if (pulseCounter >= 10) {
      // Time for 1Hz pulse - set pin HIGH
      digitalWrite(PIN_1HZ, HIGH);
      pulse1HzState = true;
      pulseCounter = 0;  // Reset counter
    }
    
    // Set timer to turn off pulses after 1ms
    timerAlarmWrite(timer10Hz, PULSE_WIDTH_US, true);
  } else {
    // End pulses - set pins LOW
    digitalWrite(PIN_10HZ, LOW);
    pulse10HzState = false;
    
    // Turn off 1Hz pulse if it was on
    if (pulse1HzState) {
      digitalWrite(PIN_1HZ, LOW);
      pulse1HzState = false;
    }
    
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
  Serial.println("Pin 18: 1Hz pulse (1ms high)");
  Serial.println("Built-in LED: Blinking at 1Hz");
  Serial.println("GPIO17: Follows LED_BUILTIN state");
  Serial.println("GPIO33: Initialized to HIGH");
  Serial.println("Pulses are perfectly aligned!");
  
  // Configure pins as outputs
  pinMode(PIN_10HZ, OUTPUT);
  pinMode(PIN_1HZ, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_GPIO17, OUTPUT);
  pinMode(PIN_GPIO33, OUTPUT);
  
  // Initialize pins to LOW
  digitalWrite(PIN_10HZ, LOW);
  digitalWrite(PIN_1HZ, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_GPIO17, LOW);
  
  // Initialize GPIO33 to HIGH
  digitalWrite(PIN_GPIO33, HIGH);
  
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
}