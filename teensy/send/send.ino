// Tests with arduino uno:
const int FELT_ANALOG_PIN = 5;
const int FELT_SUPPLY_PIN = 5;
int naturalState;
bool foo = true;

void setup(){
    Serial.begin(115200);
    pinMode(FELT_SUPPLY_PIN, OUTPUT);

    Serial.println("calibration:");
    naturalState = calibrate();

    Serial.print("=> calibration done: ");
    Serial.println(naturalState);

    while (Serial.available() == 0);
}

int calibrate(){
    const int SAMPLE_NUMBER = 100; // take 100 samples...
    const int SAMPLE_PERIOD = 10;  // ...every 10ms => 1 sec
    float accumulator = 0;

    for (int i=0; i<SAMPLE_NUMBER; i++){
        digitalWrite(FELT_SUPPLY_PIN, HIGH);

        int val = analogRead(FELT_ANALOG_PIN);
        Serial.print(val);
        Serial.print(" ");

        accumulator += val;
        digitalWrite(FELT_SUPPLY_PIN, LOW);
        delay(SAMPLE_PERIOD);
    }
    Serial.println();
    return accumulator/SAMPLE_NUMBER;
}

void loop(){
    // one of the sensors need to be supplied when it's measured:
    digitalWrite(FELT_SUPPLY_PIN, HIGH);
    int val = analogRead(FELT_ANALOG_PIN);
    digitalWrite(FELT_SUPPLY_PIN, LOW);

    // get a value in the inverted range [0; 40] so remap it to [0; 1023]:
    val = map(val, naturalState,0 , 0,1023);
    val = constrain(val, 0,1023);

    Serial.print(val); Serial.print("\t");

    for (int i=0; i<val/10; i++) {
        Serial.print("*");
    }

    Serial.print("\n");

    digitalWrite(13, foo = !foo);
    delay(10); // miliseconds
}
