#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef BPBASE_H
#define BPBASE_H
typedef int RC;

const int RC_FILE_OPEN_FAILED    = -1001;
const int RC_FILE_CLOSE_FAILED   = -1002;
const int RC_FILE_SEEK_FAILED    = -1003;
const int RC_FILE_READ_FAILED    = -1004;
const int RC_FILE_WRITE_FAILED   = -1005;
const int RC_INVALID_FILE_MODE   = -1006;
const int RC_INVALID_PID         = -1007;
const int RC_INVALID_RID         = -1008;
const int RC_INVALID_FILE_FORMAT = -1009;
const int RC_NODE_FULL           = -1010;
const int RC_INVALID_CURSOR      = -1011;
const int RC_NO_SUCH_RECORD      = -1012;
const int RC_END_OF_TREE         = -1013;
const int RC_INVALID_ATTRIBUTE   = -1014;

const int RC_FILE_READ_ONLY = -1015;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//  Miscellaneous useful definitions, including debugging routines.
//  //
//  //  The debugging routines allow the user to turn on selected
//  //  debugging messages, controllable from the command line arguments
//  //  passed to bpbase (-d).  You are encouraged to add your own
//  //  debugging flags. The pre-defined debugging flags are:
//  //
//  //  '+' -- turn on all debug messages
//  //      'c' -- page cache
//  //      'i' -- b plus tree insertion
//  //      's' -- b plus tree search
//  //


// Interface to debugging routines.

extern void DebugInit(char* flags); // enable printing debug messages

extern bool DebugIsEnabled(char flag);  // Is this debug flag enabled?

extern void DEBUG (char flag, char* format, ...);   // Print debug message
                            // if flag is enabled

//----------------------------------------------------------------------
// ASSERT
//      If condition is false,  print a message and dump core.
//  Useful for documenting assumptions in the code.
//
//  NOTE: needs to be a #define, to be able to print the location
//  where the error occurred.
//----------------------------------------------------------------------
#define ASSERT(condition)                                                     \
    if (!(condition)) {                                                       \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n",           \
                __LINE__, __FILE__);                                          \
    fflush(stderr);                               \
        Abort();                                                              \
    }



#endif 
