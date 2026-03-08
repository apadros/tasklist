#ifndef APAD_FILE_H
#define APAD_FILE_H

#include "apad_base_types.h"
#include "apad_intrinsics.h"
#include "apad_memory.h"
#include "apad_win32.h"

typedef memory_block file;

// A macro can be added here in case of porting to another OS

program_unique bool (*FileExists)(const char* path) = Win32FileExists;
										// Calls FileExists() first, returns if false
program_unique file (*LoadFile)(const char* path) = Win32LoadFile;
										// Will create a new file if it doesn't exist. 
										// If it does it'll get replaced.
program_unique void (*SaveFile)(void* data, ui32 dataSize, const char* path) = Win32SaveFile;
program_unique void (*FreeFile)(file& f) = FreeMemory;
							 		  // Defined in apad_memory.cpp
							 bool IsValid(file f);

#endif