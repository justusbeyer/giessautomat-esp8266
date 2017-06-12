/* Configuration */

// Pins
const uint8_t PIN_PUMP = 4;
const uint8_t PIN_SENSOR_POWER = 5;
const uint8_t PIN_SENSOR_ANALOG_IN = A0;

// Timing
const uint8_t LOOP_INTERVAL = 5; // in minutes
const uint16_t PUMPING_DURATION = 3500; // in ms

// Moisture threshold
const uint16_t MOISTURE_THRESHOLD = 250; // higher values mean drier

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
}


void loop() {

  /* Measure the water content of the soil */
  
  // Power up the sensor
  digitalWrite(PIN_SENSOR_POWER, HIGH);
  
  // Wait for the sensor to wake up and sense.
  delay(500);

  // Read the sensor value (TODO: Rename moisture level to sth like drynessLevel)
  uint16_t moistureLevel = analogRead(PIN_SENSOR_ANALOG_IN);

  // Power down the sensor
  digitalWrite(PIN_SENSOR_POWER, LOW);

  // Debug out
  Serial.print("Moisture: ");
  Serial.println(moistureLevel);

  /* Pump water to the plant if necessary */

  if (moistureLevel > MOISTURE_THRESHOLD) {
    // Too dry: give it a sip of water
    digitalWrite(PIN_PUMP, HIGH);
    delay(PUMPING_DURATION);
    digitalWrite(PIN_PUMP, LOW);
  }

  // Give the plant some time to absorb the water, then check again.
  for (int i=0; i < LOOP_INTERVAL; i++)
    delay((uint16_t)60000);
}
