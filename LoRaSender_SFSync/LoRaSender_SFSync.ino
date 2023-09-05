#include <SPI.h>
#include <LoRa.h>

const char ACK = '\x06';
int counter = 0;
int initialTxPower = 7;
int messagesPerSprint = 100;
int initialSF = 7;

bool changeSF(int newSF) {
  Serial.println("STARTED SF CHANGE ATTEMPT");
  
  // send initial message
  String message = "SFC" + String(newSF);
  LoRa.beginPacket();
  LoRa.println(message);
  LoRa.endPacket();
  Serial.println("[↑] " + message);

  // await for ACK
  int responseSize = 0;
  while (responseSize == 0) { responseSize = LoRa.parsePacket(); };

  String response1 = LoRa.readStringUntil('\n');
  response1.trim();
  Serial.println("[↓] " + response1);
  response1.trim();
  if (response1[0] != ACK) {
    Serial.println("FAILED - FIRST ACK FAILED");
    return false;
  }
  
  // await for response
  responseSize = 0;
  while (responseSize == 0) { responseSize = LoRa.parsePacket(); };

  String response2 = LoRa.readStringUntil('\n');
  response2.trim();
  Serial.println("[↓] " + response2);
  if (response2 != message) {
    Serial.println("FAILED - RESPONSE DIFFERS FROM INITIAL MESSAGE");
    return false;
  }
  
  delay(50);
  LoRa.beginPacket();
  LoRa.println(ACK);
  LoRa.endPacket();
  Serial.println("[↑] " + message);
  LoRa.setSpreadingFactor(newSF);
  Serial.println("SUCCESS - NEW SF" + String(newSF));
  return true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(initialSF);  // ranges from 6-12,default 7 see API docs
  LoRa.setTxPower(initialTxPower, 1);   // 2-17
  LoRa.setSignalBandwidth(500E3);       //7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3,250E3 and 500E3
  LoRa.setCodingRate4(4 / 8);           // 4/5, 4/6 , 4/7 e 4/8

  Serial.println("LoRa init succeeded.");
}

void loop() {
  for (int SF = initialSF; SF <= 12; SF++) {
    while (!changeSF(SF)) {
      delay(2000); // If failed, wait 2 secs to try again
    }

    for (int txPower = initialTxPower; txPower <= 17; txPower++) {
      LoRa.setTxPower(txPower, 1);
      for (int _i = 0; _i < messagesPerSprint; _i++) {
        String packet = txPower < 10 ? "0" + String(txPower) : String(txPower);
        packet += "hello";
        if (counter < 10) {
          packet += "000" + String(counter);
        } else if (counter < 100) {
          packet += "00" + String(counter);
        } else if (counter < 1000) {
          packet += "0" + String(counter);
        } else {
          packet += counter;
        }

        // send packet
        LoRa.beginPacket();
        LoRa.println(packet);
        LoRa.endPacket();
        Serial.println("[↑] " + packet);

        counter++;
        delay(1000);
      }
    }
  }
}
