// vim:set ft=c: HardwareSerial uart = HardwareSerial();
HardwareSerial uart = HardwareSerial();
const int LED_PIN = 11;
const int NUM_PINS = 5;
const int NUM_SAMPLES = 8;
const int SHROOM_PIN = 0;

int samplebank[NUM_PINS][NUM_SAMPLES];
int medians[NUM_PINS];
int naturalState;

void setup(){
    uart.begin(19200);
    uart.print("Serial is go.");
    naturalState = calibrate();
}

int calibrate(){
    const int N = 300; // take 300 samples...
    const int MS = 10; // ...every 10ms => 3 sec
    int acc = 0;

    for (int i=0; i<N; i++){
        acc += analogRead(SHROOM_PIN);
        delay(MS);
    }

    return acc/N;
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

bool foo = true;

void loop(){
    for (int j=0; j<NUM_SAMPLES; j++) {
        for (int i=0; i<NUM_PINS; i++) {
            // one of the sensors need to be supplied when it's measured:
            if (i == SHROOM_PIN) digitalWrite(SHROOM_PIN, HIGH);
            samplebank[i][j] = analogRead(i);
            if (i == SHROOM_PIN) {
                digitalWrite(SHROOM_PIN, LOW);
                // get a value in the inverted range [40; 0] so remap it to [0; 1023]:
                samplebank[i][j] = map(samplebank[i][j], naturalState,0 , 0,1023);
            }
        }
    }

    for (int i=0; i<NUM_PINS; i++) {
        medians[i] = median(NUM_PINS, samplebank[i]);
    }


    uart.print("!");
    for (int i=0; i<NUM_PINS; i++) {
        uart.print(medians[i],DEC);
        if ( i < (NUM_PINS-1) ) { uart.print(","); }
    }
    uart.print(".\r\n");

    digitalWrite(LED_PIN, foo);
    foo = !foo;

    delay(100); // miliseconds
}
