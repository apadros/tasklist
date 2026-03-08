#ifndef APAD_ERROR
#define APAD_ERROR

#include "apad_intrinsics.h"

// ******************** Generic ******************** //

dll_import void ExitProgram(bool error); // This will not work within DLL functions

// ******************** Error checking, setting and displaying ******************** //

// Use these to create an error message which is then logged by outside code (e.g. error within a function, need it available when returning)
dll_import bool 				IsExitIfErrorSet();
dll_import void 				SetExitIfError(bool b); // False by default

dll_import void 				ClearError();
dll_import bool 				ErrorIsSet();
dll_import const char* GetError();
dll_import void 				SetError(const char* string);

dll_import void 				DisplayErrorGUI(const char* string);

/******************** Assertions ******************** 

#define APAD_DEBUGGER_ASSERTIONS for use in a debugger, do NOT otherwise as they will have no effect.
If the macro is not defined, assertions set the global error and record the failed condition in a global string.
Call SetExitIfError(true) to have assertions stop program execution, otherwise will continue by default to allow client code to handle as seen fit.
Assertions will be printed in command line programs and displayed in a message box in GUI programs.

*****************************************************/

// Assert()
#include <intrin.h> // For __debugbreak()
#ifdef APAD_DEBUGGER_ASSERTIONS

// Will break into the debugger in debug mode and stop and exit program execution in release mode
#define Assert(_condition) { \
  if(!(_condition)) \
		__debugbreak(); \
}

#else

// If IsExitIfErrorSet() == true, will exit program execution. Otherwise, need to 
// manually check afterwards for errors and manually decide execution from there.
// IsExitIfErrorSet() == false by default.
#include <stdio.h> // For sprintf
#define Assert(_condition) { \
  ClearError(); \
  if(!(_condition)) { \
	  char buffer[256] = {}; \
		sprintf(buffer, "Assertion failed \
										 \n[Condition] %s \
										 \n[File]      %s \
										 \n[Line]      %lu", #_condition, __FILE__, __LINE__); \
		SetError((const char*)buffer); \
		dll_import program_external bool GUIApp; \
		if(GUIApp == false) \
		  printf("\n%s\n", GetError()); \
		else \
		  DisplayErrorGUI(GetError()); \
		if(IsExitIfErrorSet() == true) \
		  ExitProgram(true); \
	} \
}

#endif

// AssertRet()
#ifdef APAD_DEBUGGER_ASSERTIONS

#define AssertRet(_condition) \
	Assert(_condition)

#else
	
#define AssertRet(_condition) { \
	Assert(_condition); \
	if(ErrorIsSet() == true) \
	  return; \
}

#endif

// AssertRetType()
#ifdef APAD_DEBUGGER_ASSERTIONS

#define AssertRetType(_condition, _retValue) \
	Assert(_condition)

#else
	
#define AssertRetType(_condition, _retValue) { \
	Assert(_condition); \
	if(ErrorIsSet() == true) \
	  return (_retValue); \
}

#endif

// InvalidCodePath
#define InvalidCodePath Assert(false)

#endif