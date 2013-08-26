// vim:set ft=c: HardwareSerial uart = HardwareSerial();
HardwareSerial uart = HardwareSerial();
const int LED_PIN = 11;
const int NUM_PINS = 6;
const int NUM_SAMPLES = 8;
const int FELT_ANALOG_PIN = 5;
const int FELT_SUPPLY_PIN = 5;

int samplebank[NUM_PINS][NUM_SAMPLES];
int medians[NUM_PINS];
int minPush = 0;
int maxPush = 0;
bool led = 0;

void setup(){
    uart.begin(19200);
    uart.print("Serial is go.");
    Serial.begin(115200);
    for (int i=0; i<6; i++, delay(50))
        digitalWrite(LED_PIN, led = !led);

    pinMode(FELT_SUPPLY_PIN, OUTPUT);

    while (0) {
        digitalWrite(FELT_SUPPLY_PIN, HIGH);
        int val = analogRead(FELT_ANALOG_PIN);
        digitalWrite(FELT_SUPPLY_PIN, LOW);

        Serial.print(val);
        Serial.print("\t");
        for (int i=0; i<val/8; i++)
            Serial.print("*");
        Serial.print("\n");

        digitalWrite(LED_PIN, led = !led);
        delay(50);
    }
    calibrate();
}

void calibrate(){
    digitalWrite(LED_PIN, LOW);
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
    Serial.println("Hit the sensor for 5 sec!");
    digitalWrite(LED_PIN, HIGH);

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

    Serial.print("minPush: ");
    Serial.println(minPush);
    Serial.print("maxPush: ");
    Serial.println(maxPush);
    delay(1000);
}

int median( int n, int arr[] ){
    float temp;
    int i, j;
    // the following two loops sort the array arr in ascending order
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(arr[j] < arr[i]) {
                // swap elements
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }

    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
        return((arr[n/2] + arr[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        return arr[n/2];
    }
}

void loop(){
    for (int j=0; j<NUM_SAMPLES; j++) {
        for (int i=0; i<NUM_PINS; i++) {
            // one of the sensors need to be supplied when it's measured:
            if (i == FELT_ANALOG_PIN) digitalWrite(FELT_SUPPLY_PIN, HIGH);
            samplebank[i][j] = analogRead(i);
            if (i == FELT_ANALOG_PIN) digitalWrite(FELT_SUPPLY_PIN, LOW);
        }
    }

    for (int i=0; i<NUM_PINS; i++) {
        medians[i] = median(NUM_PINS, samplebank[i]);

        // get a value in the inverted range [40; 0] so remap it to [0; 1023]:
        if (i == FELT_ANALOG_PIN) {
            medians[i] = map(medians[i], minPush,maxPush , 0,1023);
            medians[i] = constrain(medians[i], 0,1023);
            Serial.print(medians[i]);
            Serial.print("\t |");
            for (int j=0; j<medians[i]/16; j++)
                Serial.print("*");
            Serial.print("\n");
        }
    }

    uart.print("!");
    for (int i=0; i<NUM_PINS; i++) {
        uart.print(medians[i],DEC);
        if ( i < (NUM_PINS-1) ) { uart.print(","); }
    }
    uart.print(".\r\n");

    digitalWrite(LED_PIN, led = !led);
    delay(100); // miliseconds
}

