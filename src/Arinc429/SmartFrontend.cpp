/**
  \file SmartFrontend.cpp

  \copyright Copyright (c) 2024 Ralf Wolters, 2012 Stefan Rado, 2011 Steven Cogswell <steven.cogswell@gmail.com>

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

  \version  001 - inital \n

  \brief A Arduino library to tokenize and parse commands received over a serial port.
*/

#include "SmartFrontend.h"


/**
    \brief Constructor makes sure some things are preset.
    \example SmartFrontend sCmd;
 */
SmartFrontend::SmartFrontend()
  : commandList(NULL),
    commandCount(0),
    defaultHandler(NULL),
    term('\n'),  // default terminator for commands, newline character
    last(NULL) {
  strcpy(delim, " ");  // strtok_r needs a null-terminated string
  clearBuffer();
}
/**
    \brief Adds a "command" and a handler function to the list of available commands.
    \param *command - command string to detect
    \param (*function)() - function to jump to in case of command
    \example addCommand("*IDN?", deviceID);
 */
void SmartFrontend::addCommand(const char *command, void (*function)()) {
#ifdef SMARTFRONTEND_DEBUG
    Serial.print("Adding command (");
  Serial.print(commandCount);
  Serial.print("): ");
  Serial.println(command);
#endif

  commandList = (SmartFrontendCallback *)realloc(commandList, (commandCount + 1) * sizeof(SmartFrontendCallback));
  strncpy(commandList[commandCount].command, command, SMARTFRONTEND_MAXCOMMANDLENGTH);
  commandList[commandCount].function = function;
  commandCount++;
}
/**
    \brief prints a list of available commands
    \example sCmd.printlist;
 */
void SmartFrontend::printlist(void) {
  for (int i = 0; i < commandCount; i++) {
    Serial.print(commandList[i].command);
    Serial.print(F(" "));
  }
  Serial.println();
}


/**
    \brief sets up a default handler if command string is not defined
    \param  function to call
    \example sCmd.setDefaultHandler(defaultFunction);
 */
void SmartFrontend::setDefaultHandler(void (*function)(const char *)) {  //void SmartFrontend::setDefaultHandler(void (*function)()) {
  defaultHandler = function;
}


/**
    \brief sets up a default handler if command string is not defined
    \param (*function)() - function to call
    \example sCmd.addDefaultHandler(function);
 */
void SmartFrontend::addDefaultHandler(void (*function)()) {
  defaultHandler = (void (*)(const char *))function;
}


/**
    \brief asssembles serial stream to buffer starts parsing after line feed and calls handler
    \example sCmd.readSerial();
    \warning needs to be integrated in a loop to work
 */
void SmartFrontend::readSerial() {
  while (Serial.available() > 0) {
    char inChar = Serial.read();  // Read single available character, there may be more waiting
#ifdef SMARTFRONTEND_DEBUG
    Serial.print(inChar);  // Echo back to serial stream
#endif

    if (inChar == term) {  // Check for the terminator (default '\r') meaning end of command
#ifdef SMARTFRONTEND_DEBUG
      Serial.print("Received: ");
      Serial.println(buffer);
#endif
      char *command = strtok_r(buffer, delim, &last);  // Search for command at start of buffer
      if (command != NULL) {
        boolean matched = false;
        for (int i = 0; i < commandCount; i++) {
#ifdef SMARTFRONTEND_DEBUG
          Serial.print("Comparing [");
          Serial.print(command);
          Serial.print("] to [");
          Serial.print(commandList[i].command);
          Serial.println("]");
#endif
          // Compare the found command against the list of known commands for a match
          if (strncmp(command, commandList[i].command, SMARTFRONTEND_MAXCOMMANDLENGTH) == 0) {
#ifdef SMARTFRONTEND_DEBUG
            Serial.print("Matched Command: ");
            Serial.println(command);
#endif
            // Execute the stored handler function for the command
            (*commandList[i].function)();
            matched = true;
            break;
          }
        }
        if (!matched && (defaultHandler != NULL)) {
          (*defaultHandler)(command);
        }
      }
      clearBuffer();
    } else if (isprint(inChar)) {  // Only printable characters into the buffer
      if (bufPos < SMARTFRONTEND_BUFFER) {
        buffer[bufPos++] = inChar;  // Put character into buffer
        buffer[bufPos]   = '\0';    // Null terminate
      } else {
#ifdef SMARTFRONTEND_DEBUG
        Serial.println("Line buffer is full - increase SMARTFRONTEND_BUFFER");
#endif
      }
    }
  }
}


/**
    \brief clears input buffer
    \example sCmd.clearBuffer();
 */
void SmartFrontend::clearBuffer() {
  buffer[0] = '\0';
  bufPos    = 0;
}


/**
    \brief get next token/argument from buffer as string
    \example  result = sCmd.next();
    \return  a cstring containing the argument
 */
char *SmartFrontend::next() {
  return strtok_r(NULL, delim, &last);
}


/**
    \brief get next token/argument from buffer as integer
    \param valid - shows if result is a valid integer
    \example resultInt = sCmd.next_int(valid);
    \return - integer value of argument [int]
 */
int SmartFrontend::next_int(bool *valid) {
  int   parm;
  char *arg;
  char *e;
  arg = strtok_r(NULL, delim, &last);
  if (arg != NULL) {
    parm = (int)strtol(arg, &e, 0);
    if (*e != '\0') {
      Serial.println(F("ERR2"));  // invalid number
      *valid = false;
      return (0);
    }
  } else {
    Serial.println(F("ERR1"));  // missing arg
    *valid = false;
    return (0);
  }
  *valid = true;
  return (parm);
}


/**
    \brief get next token/argument from buffer as unsigned long
    \param valid - shows if result is a valid integer
    \example resultLong = sCmd.next_ulong(valid);
    \return - unsigned long value of argument [int]
 */
unsigned long SmartFrontend::next_ulong(bool *valid) {
  unsigned long parm;
  char         *arg;
  char         *e;
  arg = strtok_r(NULL, delim, &last);
  if (arg != NULL) {
    parm = strtoul(arg, &e, 0);
    if (*e != '\0') {
      Serial.println(F("ERR2"));  // invalid number
      *valid = false;
      return (0);
    }
  } else {
    Serial.println(F("ERR1"));  // missing arg
    *valid = false;
    return (0);
  }
  *valid = true;
  return (parm);
}


/**
    \brief get next token/argument from buffer as float
    \param valid - shows if result is a valid integer
    \example resultFloat = sCmd.next_float(valid);
    \return - float value of argument [float]
 */
float SmartFrontend::next_float(bool *valid) {
  float parm;
  char *arg;
  char *e;
  arg = strtok_r(NULL, delim, &last);
  if (arg != NULL) {
    parm = strtod(arg, &e);
    if (*e != '\0') {
      Serial.println(F("ERR2"));  // invalid number
      *valid = false;
      return (0);
    }
  } else {
    Serial.println(F("ERR1"));  // missing arg
    *valid = false;
    return (0);
  }
  *valid = true;
  return (parm);
}


/**
    \brief get next token/argument from buffer as bool, acts on "ON", "OFF"
    \param valid - shows if result is a valid bool
    \example resultBool = sCmd.next_onoff(valid);
    \return - bool value of argument [bool]
 */
bool SmartFrontend::next_onoff(bool *valid) {
  char *arg;
  arg = strtok_r(NULL, delim, &last);
  if (arg != NULL) {
    if (strcmp(arg, "ON") == 0) {
      *valid = true;
      return (true);
    } else if (strcmp(arg, "OFF") == 0) {
      *valid = true;
      return (false);
    } else {
      *valid = false;
      Serial.println(F("ERR0"));  // wrong or missing argument
      return (false);
    }
  } else {
    Serial.println(F("ERR1"));  // missing arg
    *valid = false;
    return (false);
  }
}
