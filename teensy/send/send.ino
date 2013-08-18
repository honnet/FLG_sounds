// Tests with arduino uno:
const int FELT_ANALOG_PIN = 5;
const int FELT_SUPPLY_PIN = 5;
int minPush = 0;
int maxPush = 0;
bool foo = true;

void setup(){
    Serial.begin(115200);
    Serial.println("Start, don't touch the mushroom for 5 sec!");

    pinMode(FELT_SUPPLY_PIN, OUTPUT);
    calibrate();

    Serial.println("=> calibration done: ");
    Serial.println(minPush);
    Serial.println(maxPush);
    delay(1000);
}

void calibrate(){
    const int SAMPLE_PERIOD = 10;  // ms
    const int SAMPLE_NUMBER = 500; // 5 seconds

    // measure the average, don't touch for 5 seconds!
    float accumulator = 0;
    for (int i=0; i<SAMPLE_NUMBER; i++){
        digitalWrite(FELT_SUPPLY_PIN, HIGH);
        int val = analogRead(FELT_ANALOG_PIN);
        digitalWrite(FELT_SUPPLY_PIN, LOW);

        accumulator += val;
        delay(SAMPLE_PERIOD);
    }
    minPush = 0.85*accumulator/SAMPLE_NUMBER; // 15% security margin

    delay(1000);

    // measure the maximum pressure by hitting the felt:
    for (int i=0; i<SAMPLE_NUMBER; i++){
        digitalWrite(FELT_SUPPLY_PIN, HIGH);
        int val = analogRead(FELT_ANALOG_PIN);
        digitalWrite(FELT_SUPPLY_PIN, LOW);

        // Warning: inverted logic on purpose
        if (val < maxPush)
            maxPush = val;
        delay(SAMPLE_PERIOD);
    }
    maxPush *= 1.15; // 15% security margin
}

void loop(){
    // one of the sensors need to be supplied when it's measured:
    digitalWrite(FELT_SUPPLY_PIN, HIGH);
    int val = analogRead(FELT_ANALOG_PIN);
    digitalWrite(FELT_SUPPLY_PIN, LOW);

    val = map(val, minPush,maxPush , 0,1023);
    val = constrain(val, 0,1023);

    Serial.print(val); Serial.print("\t");
    for (int i=0; i<val/10; i++) {
        Serial.print("*");
    }
    Serial.print("\n");

    digitalWrite(13, foo = !foo);
    delay(20); // miliseconds
}

