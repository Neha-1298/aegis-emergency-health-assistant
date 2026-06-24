

const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;
const int buzzer = 5;

// Keep false for normal LEDs connected to GND.
// Change to true for active-low LED modules.
const bool ACTIVE_LOW = false;

enum AlertLevel {
  ALERT_OFF,
  ALERT_GREEN,
  ALERT_YELLOW,
  ALERT_RED
};

AlertLevel currentAlert = ALERT_OFF;

bool blinkState = false;
unsigned long previousBlinkTime = 0;

void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);
  Serial.setTimeout(50);

  setAlert(ALERT_OFF);

  Serial.println(
    "{\"event\":\"ARDUINO_READY\",\"alert\":\"OFF\"}"
  );
}

void loop() {
  readSerialCommand();
  updateBlink();
}

void readSerialCommand() {
  if (Serial.available() <= 0) {
    return;
  }

  String command =
    Serial.readStringUntil('\n');

  command.trim();
  command.toUpperCase();

  if (command.length() == 0) {
    return;
  }

  executeCommand(command);
}

void executeCommand(
  const String &command
) {
  if (command == "PING") {
    sendStatus("PONG");
    return;
  }

  if (command == "STATUS") {
    sendStatus("STATUS");
    return;
  }

  if (
    command == "ALERT_GREEN" ||
    command == "GREEN_ON"
  ) {
    setAlert(ALERT_GREEN);
    sendStatus("ALERT_GREEN");
    return;
  }

  if (
    command == "ALERT_YELLOW" ||
    command == "YELLOW_ON"
  ) {
    setAlert(ALERT_YELLOW);
    sendStatus("ALERT_YELLOW");
    return;
  }

  if (
    command == "ALERT_RED" ||
    command == "RED_ON" ||
    command == "BLUE_ON"
  ) {
    setAlert(ALERT_RED);
    sendStatus("ALERT_RED");
    return;
  }

  if (
    command == "ALERT_CLEAR" ||
    command == "ALL_OFF" ||
    command == "OFF"
  ) {
    setAlert(ALERT_OFF);
    sendStatus("ALERT_CLEAR");
    return;
  }

  if (command == "SELF_TEST") {
    runSelfTest();
    sendStatus("SELF_TEST_COMPLETE");
    return;
  }

  sendStatus("UNKNOWN_COMMAND");
}

void setAlert(
  AlertLevel newAlert
) {
  currentAlert = newAlert;

  blinkState =
    newAlert != ALERT_OFF;

  previousBlinkTime =
    millis();

  applyCurrentOutput();
}

void updateBlink() {
  if (currentAlert == ALERT_OFF) {
    return;
  }

  unsigned long currentTime =
    millis();

  unsigned long blinkInterval =
    getBlinkInterval();

  if (
    currentTime - previousBlinkTime <
    blinkInterval
  ) {
    return;
  }

  previousBlinkTime =
    currentTime;

  blinkState = !blinkState;

  applyCurrentOutput();
}

void applyCurrentOutput() {
  turnAllOutputsOff();

  if (!blinkState) {
    return;
  }

  if (currentAlert == ALERT_GREEN) {
    writeLED(greenLED, true);
    return;
  }

  if (currentAlert == ALERT_YELLOW) {
    writeLED(yellowLED, true);
    return;
  }

  if (currentAlert == ALERT_RED) {
    writeLED(redLED, true);
    tone(buzzer, 1200);
  }
}

unsigned long getBlinkInterval() {
  if (currentAlert == ALERT_GREEN) {
    return 650;
  }

  if (currentAlert == ALERT_YELLOW) {
    return 350;
  }

  if (currentAlert == ALERT_RED) {
    return 160;
  }

  return 500;
}

void writeLED(
  int pin,
  bool enabled
) {
  if (ACTIVE_LOW) {
    digitalWrite(
      pin,
      enabled ? LOW : HIGH
    );
  } else {
    digitalWrite(
      pin,
      enabled ? HIGH : LOW
    );
  }
}

void turnAllOutputsOff() {
  writeLED(greenLED, false);
  writeLED(yellowLED, false);
  writeLED(redLED, false);

  noTone(buzzer);
  digitalWrite(buzzer, LOW);
}

void runSelfTest() {
  setAlert(ALERT_OFF);

  blinkTestOutput(
    greenLED,
    false
  );

  blinkTestOutput(
    yellowLED,
    false
  );

  blinkTestOutput(
    redLED,
    true
  );

  setAlert(ALERT_OFF);
}

void blinkTestOutput(
  int ledPin,
  bool soundBuzzer
) {
  for (
    int count = 0;
    count < 3;
    count++
  ) {
    writeLED(ledPin, true);

    if (soundBuzzer) {
      tone(buzzer, 1200);
    }

    delay(300);

    writeLED(ledPin, false);

    noTone(buzzer);
    digitalWrite(buzzer, LOW);

    delay(300);
  }
}

const char *getAlertName() {
  if (currentAlert == ALERT_GREEN) {
    return "GREEN";
  }

  if (currentAlert == ALERT_YELLOW) {
    return "YELLOW";
  }

  if (currentAlert == ALERT_RED) {
    return "RED";
  }

  return "OFF";
}

void sendStatus(
  const String &eventName
) {
  Serial.print("{");

  Serial.print("\"event\":\"");
  Serial.print(eventName);
  Serial.print("\",");

  Serial.print("\"alert\":\"");
  Serial.print(getAlertName());
  Serial.print("\",");

  Serial.print("\"blinking\":");
  Serial.print(
    currentAlert != ALERT_OFF
      ? "true"
      : "false"
  );

  Serial.println("}");
}
