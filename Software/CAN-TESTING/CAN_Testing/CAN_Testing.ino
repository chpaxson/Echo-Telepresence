//==================================================================================//

// #include <CAN.h>
#include <Arduino.h>

#define TX_GPIO_NUM   A1  // Connects to CTX
#define RX_GPIO_NUM   A0  // Connects to CRX

//==================================================================================//

void setup() {
 pinMode( TX_GPIO_NUM, INPUT );
 pinMode( RX_GPIO_NUM, INPUT );
 Serial.begin( 115200 ); // 1Mbaud, CAN Bus is most likely 500kHz
}
void loop() {
 int a1Value = analogRead( TX_GPIO_NUM );
 int a2Value = analogRead( RX_GPIO_NUM );
 Serial.print( "A1:" );
 Serial.println( a1Value );
 Serial.print( ",A2:" );
 Serial.println( a2Value );
}