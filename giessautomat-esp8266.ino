#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#include "thingerio-credentials.h"

/* Configuration */

// Pins
const uint8_t PIN_PUMP = 4;
const uint8_t PIN_SENSOR_POWER = 5;
const uint8_t PIN_SENSOR_ANALOG_IN = A0;

// Timing
uint8_t LOOP_INTERVAL = 1; // in minutes
const uint16_t PUMPING_DURATION = 3500; // in ms

// Moisture threshold
uint16_t moistureLevelThreshold = 730; // higher values mean drier

// Last measured moisture leve
uint16_t lastMoistureLevelMeasurement = 0;

// Timestamp of last moisture level measurement
uint32_t lastMoistureLevelMeasurementTimestamp = 0;

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  /* Set up pins */

  // Pump: The pin connects to a FET which provides power to the pump
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);

  // Sensor Power: We only power the sensor to save energy and
  // prevent unnecessary currents in the soil leading to corrosion.
  pinMode(PIN_SENSOR_POWER, OUTPUT);
  digitalWrite(PIN_SENSOR_POWER, LOW);

  // Sensor Analog in: Thats the sensor´s voltage carrying
  // information on the degree of water in the soil
  pinMode(PIN_SENSOR_ANALOG_IN, INPUT);

  /* Debugging via serial */
  Serial.begin(115200);

  /* Setup Thinger.io cloud */
  thing.add_wifi(SSID, SSID_PASSWORD);
  thing["moistureLevel"] >> [](pson& out) { out = lastMoistureLevelMeasurement; };
  
  thing["moistureLevelThreshold"] << [](pson& in) {
    if (in.is_empty() || (uint32_t)in < 200) {
      in = moistureLevelThreshold;
    } else {
      Serial.print("Set moisture level threshold from ");
      Serial.print(moistureLevelThreshold);
      Serial.print(" to ");
      Serial.print((uint16_t)in);
      Serial.println(".");
      moistureLevelThreshold = (uint16_t)in;
    }
  };

  Serial.println("Setup done.");
}

void handleWatering() {

  /* Measure the water content of the soil */
  
  // Power up the sensor
  digitalWrite(PIN_SENSOR_POWER, HIGH);
  
  // Wait for the sensor to wake up and sense.
  delay(500);

  // Read the sensor value (TODO: Rename moisture level to sth like drynessLevel)
  lastMoistureLevelMeasurement = analogRead(PIN_SENSOR_ANALOG_IN);

  // Power down the sensor
  digitalWrite(PIN_SENSOR_POWER, LOW);

  // Debug out
  Serial.print("Moisture: ");
  Serial.println(lastMoistureLevelMeasurement);

  /* Pump water to the plant if necessary */

  if (lastMoistureLevelMeasurement > moistureLevelThreshold) {
    // Too dry: give it a sip of water
    digitalWrite(PIN_PUMP, HIGH);
    delay(PUMPING_DURATION);
    digitalWrite(PIN_PUMP, LOW);
  }
  
  lastMoistureLevelMeasurementTimestamp = millis();
}

void loop() {
  uint32_t now = millis();

  if (lastMoistureLevelMeasurementTimestamp == 0 ||
      now - lastMoistureLevelMeasurementTimestamp > (uint32_t)LOOP_INTERVAL * 60000UL) {
    handleWatering();
  }

  thing.handle();
}
