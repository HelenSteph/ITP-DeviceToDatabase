// Log received LoRa packets
#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println("Received packet");

    Serial.print("  length : ");
    Serial.print(packetSize);
    Serial.println();


    String s = "";
    Serial.print("   bytes : ");
    // read packet
    for (int i = 0; i < packetSize; i++) {
      byte b = LoRa.read();
      Serial.print(b, HEX);  // print byte as hex
      Serial.print(" ");  
      s += (char)b;          // append byte to the String
    }
    Serial.println();

    // replace linefeeds
    s.replace("\r","\\r");
    s.replace("\n","\\n");

    Serial.print("  string : ");
    Serial.println(s);
  
    Serial.print("    rssi : ");
    Serial.println(LoRa.packetRssi());
    Serial.println();
  }
}
