/**
  \file arinc429.cpp

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

  \brief ARINC-429 proof of concept library
*/


#include "arinc429.h"
#include "arincTX.pio.h"
#include "arincRX.pio.h"
#include "Arduino.h"


pioAdmin pioUse[MAX_PIOS][MAX_STATEMACHINES];

/*
 *  get a random message with correct (odd) parity
 */
uint32_t getRandomMessage(){
  
  uint32_t result, n;
  uint8_t  ones;
  
  result = random();
  n = result;
  for (ones=0; n>0; n=n&(n-1)) ones++;  // count binary ones
  if ( (ones % 2) == 0) {               // parity is even
    result = result ^ 0x80000000;       // make odd -> toggle MSB
  }   
  return(result);
}


/*
 *  free all statemachines
 */
void clearPIOs() {
  for (uint8_t p=0; p<MAX_PIOS; p++) {
    for (uint8_t s=0; s<MAX_STATEMACHINES; s++) {
      pioUse[p][s].pioUsedFor = PIO_FREE;
      pioUse[p][s].offset = INVALID_OFFSET; 
    }
  }
}

/*
 *  get next free state machine and PIO
 */
bool getNextFreeSM(uint8_t &pioNr, uint8_t &smNr){
  for (pioNr=0; pioNr<MAX_PIOS; pioNr++) {
    for (smNr=0; smNr<MAX_STATEMACHINES; smNr++) {
      if (pioUse[pioNr][smNr].pioUsedFor == PIO_FREE) {
        return(true);
      }
    }
  }
  return(false);
}

/*
 *  check code for state machine loaded to PIO
 */
int8_t codeLoaded(uint8_t pioNr, pioDetails wanted) {
  for (uint8_t smNr=0; smNr<MAX_STATEMACHINES; smNr++) {
    if (pioUse[pioNr][smNr].pioUsedFor == wanted) {
        return(pioUse[pioNr][smNr].offset);
    }
  }
  return(INVALID_OFFSET);
}
 
/*
 *  try to get free PIO/state machine for PIO program
 */
bool initPIO(pioDetails wanted, pioStruct &newPio, uint8_t PIO_PIN, uint32_t baudrate) {
//bool loadTxCode(pioStruct &newPio) {
  uint8_t pioNr, smNr;
  
  if (not getNextFreeSM(pioNr, smNr)) {
    Serial.println("!no more state machines available");
    return(false);
  }
  int8_t offset = codeLoaded(pioNr, wanted);
  if (offset == INVALID_OFFSET) { // code not loaded
    switch (pioNr) {
      case 0:
        newPio.pio = pio0;
        break;
      case 1:
        newPio.pio = pio1;
        break;
      case 2:
        newPio.pio = pio2;
        break;
    }    
    switch (wanted) { // load wanted program
      case PIO_TX:
        newPio.offset = pio_add_program(newPio.pio, &arincTX_program);
        newPio.sm = pio_claim_unused_sm(newPio.pio, false);
        arincTX_program_init(newPio.pio, newPio.sm, newPio.offset, PIO_PIN, baudrate);
        break;
      case PIO_RX:
        newPio.offset = pio_add_program(newPio.pio, &arincRX_program);
        newPio.sm = pio_claim_unused_sm(newPio.pio, false);
        arincRX_program_init(newPio.pio, newPio.sm, newPio.offset, PIO_PIN, baudrate);
        break;
      default:
        Serial.println("!program not known");
    }
    pioUse[pioNr][smNr].pioUsedFor = wanted;    // block for further use
    pioUse[pioNr][smNr].offset = newPio.offset; 
    Serial.print("#added program to PIO[");
    Serial.print(pioNr);
    Serial.print("] SM: [");
    Serial.print(smNr);
    Serial.print("] offset: ");
    Serial.println(newPio.offset);
  } else { // code already loaded
    switch (pioNr) {
      case 0:
        newPio.pio = pio0;
        break;
      case 1:
        newPio.pio = pio1;
        break;
      case 2:
        newPio.pio = pio2;       
        break;
    }
    newPio.offset = offset;
    pioUse[pioNr][smNr].pioUsedFor = wanted;              // block for further use
    pioUse[pioNr][smNr].offset = offset;    
    // get state machine, false = no panic
    newPio.sm = pio_claim_unused_sm(newPio.pio, false);  
    switch (wanted) { // load wanted program
      case PIO_TX:
        arincTX_program_init(newPio.pio, newPio.sm, newPio.offset, PIO_PIN, baudrate);
        break;
      case PIO_RX:
        arincRX_program_init(newPio.pio, newPio.sm, newPio.offset, PIO_PIN, baudrate);
        break;
      default:
        Serial.println("!program not known");
    }
    Serial.print("#reuse program to PIO[");
    Serial.print(pioNr);
    Serial.print("] SM: [");
    Serial.print(smNr);
    Serial.print("] offset: ");
    Serial.println(newPio.offset);
  }
  return(true);
}

/*
 *  set ARINCs TX baudrate
 */
void setArincTxBaudrate(pioStruct pioUse, uint32_t baudrate) {
  float clkdiv = (float)clock_get_hz(clk_sys) / (baudrate * 16);
  pio_sm_set_clkdiv(pioUse.pio, pioUse.sm, clkdiv);
}

/*
 *  send 32 bit ARINC message blocking (8x 32 bit FIFO)
 */
void sendArincBlocking(pioStruct pioUse, uint32_t message) {
  pio_sm_put_blocking(pioUse.pio, pioUse.sm, message);
}

/*
 *  send 32 bit ARINC message non-blocking, returns false if FIFO full (8x 32 bit FIFO)
 */
bool sendArincNoBlocking(pioStruct pioUse, uint32_t message) {
  if (pio_sm_is_tx_fifo_full(pioUse.pio, pioUse.sm)) {
    return(false);
  } else {
    pio_sm_put(pioUse.pio, pioUse.sm, message);
  }
  return(true);
}

/*
 * RECEIVE PART
 */

/*
 *  set ARINCs RX baudrate
 */
void setArincRxBaudrate(pioStruct pioUse, uint32_t baudrate) {
  float clkdiv = (float)clock_get_hz(clk_sys) / (baudrate * 32);
  pio_sm_set_clkdiv(pioUse.pio, pioUse.sm, clkdiv);
}

/*
 *  check new message available
 */
bool arincAvailable(pioStruct pioUse) {

  return(pio_sm_get_rx_fifo_level(pioUse.pio, pioUse.sm) > 0);
}

/*
 *  get new message
 */
uint32_t receiveArinc(pioStruct pioUse) {
  
  return(pio_sm_get(pioUse.pio, pioUse.sm));
}
