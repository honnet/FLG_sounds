// vim:set ft=c:

HardwareSerial uart = HardwareSerial();
int LED_PIN = 11;
int counter = 0;

void setup(){
  uart.begin(19200);
  Serial.begin(19200);
}

void loop(){
    int c;
    if (uart.available()) {
        c = uart.read();
        if ( c == '!' ) {
            digitalWrite(LED_PIN, counter++ % 2);
        }
        Serial.write( c );
    }
}
