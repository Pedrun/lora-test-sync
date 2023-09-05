#include <SPI.h>
#include <LoRa.h>

const char ACK = '\x06';
int SF = 7;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(SF);           // ranges from 6-12,default 7 see API docs
  LoRa.setSignalBandwidth(500E3);          //7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3,250E3 and 500E3
  LoRa.setCodingRate4(4/8);                // 4/5, 4/6 , 4/7 e 4/8
  Serial.println("LoRa init succeeded.");
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    //Serial.print("Received packet '");

    // read packet
    String message = LoRa.readStringUntil('\n');
    message.trim();
    Serial.print("[↓] ");
    Serial.print(message);

    // print SF of packet
    Serial.print(",");
    Serial.print(SF);
    
    // print RSSI of packet
    Serial.print(",");
    Serial.print(LoRa.packetRssi());

    // print SNR of packet
    Serial.print(",");
    Serial.println(LoRa.packetSnr());

    if (message.startsWith("SFC")) { // Spreading Factor Change
      Serial.println("BEGIN SF CHANGE");
      int newSF = message.substring(3).toInt();
      
      // send ACK
      delay(50);
      LoRa.beginPacket();
      LoRa.println(ACK);
      LoRa.endPacket();
      Serial.println("[↑] ACK");

      // confirm initial message
      delay(50);
      LoRa.beginPacket();
      LoRa.println(message);
      LoRa.endPacket();
      Serial.println("[↑] " + message);

      // await for response
      int responseSize = 0;
      while (responseSize == 0) { responseSize = LoRa.parsePacket(); }
      
      // read response
      String response = LoRa.readStringUntil('\n');
      response.trim();
      Serial.print("[↓ (RESPONSE)] ");
      Serial.println(response);

      // change SF
      if (response[0] == ACK) {
        SF = newSF;
        LoRa.setSpreadingFactor(SF);
        Serial.print("SUCCESS - NEW SF: ");
        Serial.println(SF);
      } else {
        Serial.println("FAILED - NO ACK RECEIVED");
      }
    } 
  }
}
