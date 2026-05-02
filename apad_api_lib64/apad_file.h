#ifndef APAD_FILE_H
#define APAD_FILE_H

#include "apad_base_types.h"
#include "apad_intrinsics.h"
#include "apad_memory.h"
#include "apad_win32.h"

typedef memory_block file;

// A macro can be added here in case of porting to another OS

// ******************** Loading and saving ******************** //

// For some reason can't declase these function pointers as dll_import
program_unique bool 			(*FileExists)(const char* path) = Win32FileExists;
program_unique file 			(*LoadFile)(const char* path) = Win32LoadFile; // FileExists() must be called first
program_unique void 			(*SaveFile)(void* data, ui32 dataSize, const char* path) = Win32SaveFile; // Will create a new file if it doesn't exist; if it does it'll get replaced.
program_unique void 			(*FreeFile)(file& f) = FreeMemory;
dll_import 		 bool   			IsValid(file f); // Defined in apad_memory.cpp
dll_import 		 const char*  GetFileNameAndExtension(const char* path);

// ******************** Reading ******************** //

struct file_line {
	memory_stack data;
	ui8          count;
};

#define              LineReadLoopHeader(_readIndex, _file) for(ui32 _readIndex = 0; _readIndex < (_file).size; )
dll_import file_line ReadLine(file& f, ui32& readIndex); // Will treat any data between quotation marks as a single string
dll_import bool      LineIsValid(file_line& f);
dll_import char* 		 GetLineDataElement(file_line& line, ui8 index);
dll_import void 		 FreeLine(file_line& line); // Must be called after every call to ReadLine() once data is not needed

// ******************** Writing ******************** //

// Cause Windows
#ifdef CreateFile
#undef CreateFile
#endif

dll_import file CreateFile(); // Needs to be freed afterwards
dll_import void WriteToFile(void* data, ui32 size, file& f);
dll_import void WriteToFile(char* string, file& f);

#endif