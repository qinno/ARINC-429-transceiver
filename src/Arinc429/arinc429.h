/**
  \file arinc429.h
  \brief ARINC-429 proof of concept library header
*/


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#pragma once

const uint32_t LOW_SPEED  =  12500;  
const uint32_t HIGH_SPEED = 100000;  

#ifdef PICO_RP2350
const uint8_t MAX_PIOS  =  3; 
#else // RP2040
const uint8_t MAX_PIOS  =  2; 
#endif
const uint8_t MAX_STATEMACHINES = 4;


struct pioStruct { // structure for PIO access
  PIO  pio;
  uint offset;
  int  sm;
};

enum pioDetails { PIO_TX, PIO_RX, PIO_OTHER, PIO_FREE }; // 

const int8_t  INVALID_OFFSET = -1;
struct pioAdmin { // structure for PIO administration
  pioDetails pioUsedFor;
  int8_t     offset;
};

// misc.
uint32_t getRandomMessage();

void clearPIOs();
bool getNextFreeSM(uint8_t &pioNr, uint8_t &smNr);
int8_t codeLoaded(uint8_t pioNr, pioDetails wanted);


// transmitter
//bool loadTxCode(pioStruct &pioBase);
//bool initArincTX(pioStruct &pioUse, uint8_t TX_PIN, uint32_t baudrate);
bool initPIO(pioDetails wanted, pioStruct &newPio, uint8_t PIO_PIN, uint32_t baudrate);

void setArincTxBaudrate(pioStruct pioUse, uint32_t baudrate);
void sendArincBlocking(pioStruct pioUse, uint32_t message);
bool sendArincNoBlocking(pioStruct pioUse, uint32_t message);

// receiver
bool loadRxCode(pioStruct &pioBase);
bool initArincRX(pioStruct pioBase, pioStruct &pioUse, uint8_t TX_PIN, uint32_t baudrate);
void setArincRxBaudrate(pioStruct pioUse, uint32_t baudrate);
bool arincAvailable(pioStruct pioUse);
uint32_t receiveArinc(pioStruct pioUse);
