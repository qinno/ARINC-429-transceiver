/**
  \file Arinc429.ino

  \copyright Copyright (c) 2024 qinno Ralf Wolters. All right reserved.

  \license
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 3. \n
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>. \n
  If you are interested in OEM applications, you may prefer a commercial license. \n
  Contact us for more information on commercial or OEM licenses.

  \version  001 - inital \n

  \brief ARINC-429 proof of concept
*/

/*
 * commands for copy paste in terminal
 *  S 0 0xDEADBEEF  // send channel 1 hex-value
 *  BAUD 1 TX l // set TX Baudrate channel 1 to low (12,5 kBit)
 *  BAUD 1 RX l // set RX Baudrate channel 1 to low (12,5 kBit)
 *  BAUD 1 RX h // set TX Baudrate channel 1 to high (100 kBit)
 *  BAUD 1 TX h // set RX Baudrate channel 1 to low (100 kBit)
 */

// firmware info
#define FW_ITEM    "ARINC-429 transceiver"
#define FW_VERSION "008"

// channels and pinouts
#define TX_CHANNEL_NO 6  // max. 4
#define RX_CHANNEL_NO 6  // max. 4
#define LOOP_CHANNELS 4  // minimum of above

// helpers
#include "arinc429.h"
#include "SmartFrontend.h"

// serial communication
SmartFrontend sCmd;  // serial frontend

// transmitter
const uint8_t TX_PINS[] = { 4, 6, 12, 14, 20, A0 };  // TX HIGH pins, LOW pin = +1
// receiver
const uint8_t RX_PINS[] = { 0, 2, 8, 10, 16, 18 };  // RX HIGH pins, TRIG pin = +1

pioStruct txBase, rxBase;
pioStruct txPio[TX_CHANNEL_NO], rxPio[RX_CHANNEL_NO];

uint32_t arincSpeed = HIGH_SPEED;
uint32_t lastTX, lastRX, txOK, txSend;


void setup() {

  Serial.begin();  // no baudrate needed on pico USB
  delay(1000);
  deviceID();
  uint8_t count;

  // Setup callbacks for SmartFrontend commands
  sCmd.addCommand("*IDN?", deviceID);        // get device ID
  sCmd.addCommand("BOOT", deviceReboot);     // reboot device
  sCmd.addCommand("BAUD", cmndSetBaudrate);  // set baudrate RX channel
  sCmd.addCommand("S", cmndSendPaket);       // send paket with ack
  sCmd.addCommand("X", cmndSendPaketNoAck);  // send paket no ack
  arincSpeed = HIGH_SPEED;                   // preset speed
  clearPIOs();                               // reset PIOs

  for (count = 0; count < TX_CHANNEL_NO; ++count) {
    initPIO(PIO_TX, txPio[count], TX_PINS[count], arincSpeed);
  }

  for (count = 0; count < RX_CHANNEL_NO; ++count) {
    initPIO(PIO_RX, rxPio[count], RX_PINS[count], arincSpeed);
  }

  rp2040.wdt_begin(1000);  // enable watchdog in milliseconds, max 8300
  txOK   = 0;
  txSend = 0;
}
//#define TXLOOP
#define TXLOOPVAL 0x68AB55A7

uint8_t i;

void loop() {

  rp2040.wdt_reset();
  sCmd.readSerial();  // process serial commands
  for (i = 0; i < RX_CHANNEL_NO; i++) {
    if (arincAvailable(rxPio[i])) {
      Serial.print("*RX[");
      Serial.print(i);
      Serial.print("] 0x");
      Serial.println(receiveArinc(rxPio[i]), HEX);
    }
  }

#ifdef TXLOOP
  lastTX = getRandomMessage();
  sendArincNoBlocking(tx1, lastTX);
  ++txSend;
  delayMicroseconds(400);
  if (arincAvailable(rx1)) {
    lastRX = receiveArinc(rx1);
    if (lastRX == lastTX) ++txOK;
  }
  if ((txSend % 100) == 0) {
    Serial.print("#LP ");
    Serial.print(txSend);
    Serial.print(" ERR ");
    Serial.println(txSend - txOK);
  }
#endif
}

/*
 *  send device ID
 */
void deviceID() {
  Serial.print("#");  // to differentiate infos
  Serial.print(FW_ITEM);
  Serial.print(" ");
  Serial.print(TX_CHANNEL_NO);
  Serial.print("xTX ");
  Serial.print(RX_CHANNEL_NO);
  Serial.print("xRX");
  Serial.print(" V.");
  Serial.print(FW_VERSION);
  Serial.print(" CPU ");
  Serial.print(rp2040.f_cpu() / 1000000);
  Serial.println(" MHz");
}

/*
 *  reboot with help of watchdog, changes Arduino IDE USB port
 */
void deviceReboot() {
  while (true)
    ;  // reboot with help of watchdog, changes USB port
}

/*
 *  set Baudrate for RX channel
 *  commands: BAUD 1 RX l - set receiver channel1 baudrate to (l)ow speed
 *            BAUD 2 TX h - set transmitter channel 2 baudrate to (h)igh speed
 */
void cmndSetBaudrate() {
  bool     valid;
  int      arincChannel;
  String   arincDirection;
  String   arincSpeedId;
  uint32_t arincSpeed = HIGH_SPEED;

  arincChannel = sCmd.next_int(&valid);  // check channel is int
  if (!valid) return;
  arincDirection = sCmd.next();  // check direction
  if (arincDirection != "RX" and arincDirection != "TX") {
    Serial.println("!invalid direction");
  }
  arincSpeedId = sCmd.next();  // check speed
  if (arincSpeedId != "l" and arincSpeedId != "h") {
    Serial.println("!invalid speed");
  } else {
    if (arincSpeedId == "l") {
      arincSpeed = LOW_SPEED;
    } else {
      arincSpeed = HIGH_SPEED;
    }
  }
  if (arincDirection == "RX") {
    if (arincChannel >= RX_CHANNEL_NO or arincChannel < 0) {
      Serial.println("!invalid RX channel");
      return;
    }
    setArincRxBaudrate(rxPio[arincChannel], arincSpeed);
  } else {
    if (arincChannel >= TX_CHANNEL_NO or arincChannel < 0) {
      Serial.println("!invalid TX channel");
      return;
    }
    setArincTxBaudrate(txPio[arincChannel], arincSpeed);
  }
  Serial.print("#BAUD ");
  Serial.print(arincDirection);
  Serial.print("[");
  Serial.print(arincChannel);
  Serial.print("] = ");
  Serial.println(arincSpeed);
}

/*
 *  send paket over transmitter X
 *  command: S 1 01234567 - send 32-bit(Hex) = 0x1234567 out to transmitter 1
 */
void cmndSendPaket() {
  bool     valid;
  int      arincChannel;
  uint32_t arincMessage;

  arincChannel = sCmd.next_int(&valid);  // check channel is int
  if (!valid) return;
  arincMessage = sCmd.next_ulong(&valid);  // check message is uint32
  if (!valid) return;
  if (arincChannel >= TX_CHANNEL_NO or arincChannel < 0) {
    Serial.println("!invalid TX channel");
    return;
  }
  sendArincNoBlocking(txPio[arincChannel], arincMessage);
  Serial.print("#TX[");
  Serial.print(arincChannel);
  Serial.print("] 0x");
  Serial.println(arincMessage, HEX);
}

/*
 *  send paket over transmitter X (no acknowledge)
 *  command: S 1 01234567 - send 32-bit(Hex) = 0x1234567 out to transmitter 1
 */
void cmndSendPaketNoAck() {
  bool     valid;
  int      arincChannel;
  uint32_t arincMessage;

  arincChannel = sCmd.next_int(&valid);  // check channel is int
  if (!valid) return;
  arincMessage = sCmd.next_ulong(&valid);  // check message is uint32
  if (!valid) return;
  if (arincChannel >= TX_CHANNEL_NO or arincChannel < 0) {
    Serial.println("!invalid TX channel");
    return;
  }
  sendArincNoBlocking(txPio[arincChannel], arincMessage);
}
