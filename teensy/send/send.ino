// vim:set ft=c: HardwareSerial uart = HardwareSerial();
HardwareSerial uart = HardwareSerial();
const int LED_PIN = 11;
const int NUM_PINS = 5;
const int NUM_SAMPLES = 8;

int samplebank[NUM_PINS][NUM_SAMPLES];
int medians[NUM_PINS];

void setup(){
    uart.begin(19200);
    uart.print("Serial is go.");
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
            samplebank[i][j] = analogRead(i);
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
