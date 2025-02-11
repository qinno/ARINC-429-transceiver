/**
  \file SmartFrontend.h
  \brief A Arduino library to tokenize and parse commands received over a serial port. - header
*/

#ifndef SmartFrontend_h
#define SmartFrontend_h

#if defined(WIRING) && WIRING >= 100
#include <Wiring.h>
#elif defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <string.h>

// Size of the input buffer in bytes (maximum length of one command plus arguments)
#define SMARTFRONTEND_BUFFER 40
// Maximum length of a command excluding the terminating null
#define SMARTFRONTEND_MAXCOMMANDLENGTH 16

// Uncomment the next line to run the library in debug mode (verbose messages)
//#define SMARTFRONTEND_DEBUG


class SmartFrontend {
public:
  SmartFrontend();                                                    // Constructor
  void          addCommand(const char *command, void (*function)());  // Add a command to the processing dictionary.
  void          printlist(void);                                      // print command list
  void          setDefaultHandler(void (*function)(const char *));    // A handler to call when no valid command received.
  void          addDefaultHandler(void (*function)());                // A handler to call when no valid command received.
  void          readSerial();                                         // Main entry point.
  void          clearBuffer();                                        // Clears the input buffer.
  char         *next();                                               // Returns pointer to next token found in command buffer (for getting arguments to commands).
  int           next_int(bool *valid);                                // int16_t
  unsigned long next_ulong(bool *valid);                              // uint16_t
  float         next_float(bool *valid);                              // float
  bool          next_onoff(bool *valid);                              // bool

private:
  // Command/handler dictionary
  struct SmartFrontendCallback {
    char command[SMARTFRONTEND_MAXCOMMANDLENGTH + 1];
    void (*function)();
  };                                       // Data structure to hold Command/Handler function key-value pairs
  SmartFrontendCallback *commandList;      // Actual definition for command/handler array
  byte                   commandCount;     // command counter
  void (*defaultHandler)(const char *);    // Pointer to the default handler function
  char  delim[2];                          // null-terminated list of character to be used as delimeters for tokenizing (default " ")
  char  term;                              // Character that signals end of command (default '\n')
  char  buffer[SMARTFRONTEND_BUFFER + 1];  // Buffer of stored characters while waiting for terminator character
  byte  bufPos;                            // Current position in the buffer
  char *last;                              // State variable used by strtok_r during processing
};

#endif  //SmartFrontend_h
