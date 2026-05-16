#include <stdio.h>
#include <stdlib.h> // For atexit()
#include <string.h>
#include "apad_base_types.h"
#include "apad_error_internal.h"
#include "apad_intrinsics.h"
#include "apad_maths.h"
#include "apad_memory.h"
#include "apad_string.h"

// ******************** Internal API start ******************** //

const ui16 MaxStringLength = UI16Max;

// @TODO - Replace stack functionality with pool allocation ?
// Array of memory_blocks containing dynamically-allocated memory pointers allocated on the heap.
memory_stack stringTable; 

program_local void ExitStringAPI() {
	FunctionStart(;);
	
	if(IsValid(stringTable) == true) {
		// Free all memory blocks contained within the table
		ui32  blocks = stringTable.size / sizeof(memory_block);
		auto* table = (memory_block*)stringTable.memory;
		ForAll(blocks) {
			auto* block = table + it;
			if(IsValid(*block) == true)
				FreeMemory(*block);
		}
		
		FreeStack(stringTable);
	}
	
	FunctionEnd();
}

program_local void AddToStringTable(memory_block& block) {
	FunctionStart(;);
	
  if(IsValid(stringTable) == false) { // Init table
	  stringTable = AllocateStack(KiB(1));
		atexit(ExitStringAPI); // @TODO - Returns 0 for no error. What to do if it doesn't?
	}
	
	PushInstance(block, stringTable);
	
	FunctionEnd();
}

program_local void PushNullChar(memory_stack& stack) {
	FunctionStart(;);
	
	Push("\0", 1, stack);
	
	FunctionEnd();
}

// Also used in log.cpp
dll_export char* PushString(const char* string, bool addEOS, memory_block& stack) {
  FunctionStart(Null);
	
	auto  length = GetStringLength(string);
	void* ret = (ui8*)stack.memory + stack.size;
	if(length > 0)
		ret = Push((void*)string, length, stack);
	if(addEOS == true)
		PushNullChar(stack);
	
	FunctionEnd();
	return (char*)ret;
}

// ******************** Internal API end ******************** //

#include <ctype.h>
dll_export bool IsWhitespace(char c) {
	return isspace(c) != 0;
}

dll_export void ConvertStringToLowerCase(const char* s) {
	FunctionStart(;);
	AssertInternal(s != Null);
	
	auto length = GetStringLength(s);
	ForAll(length) {
    if(s[it] >= 'A' && s[it] <= 'Z')
			((char*)s)[it] += 'a' - 'A';
	}
	
	FunctionEnd();
}

#include <stdarg.h>
// Unfortunately variadic arguments cannot intrinsically infer the number of parameters passed to them
dll_export char* Concatenate(ui8 count, ...) {
	FunctionStart(Null);
	AssertInternal(count >= 2);
	
	va_list list;
	va_start(list, count);
	
	auto stack = AllocateStack(Null);
	
	ForAll(count) {
		char* string = va_arg(list, char*);
		if(string != Null) // Just to avoid having to check for Null when concatenating several strings
			PushString(string, false, stack);
	}
	
	PushNullChar(stack);
	
	va_end(list);
	
	AddToStringTable(stack);
	
	FunctionEnd();
	return (char*)stack.memory;
}

dll_export char* AllocateString(const char* s, ui16 length) {
	FunctionStart(Null);
	AssertInternal(s != Null);
	
	auto sLength = GetStringLength(s);
	
	ui16 copyLength = 0;
	if(length == Null)
		copyLength = sLength;
	else
		copyLength = Min(length, sLength);
	
	auto stack = AllocateStack(copyLength + 1);
	Push((void*)s, copyLength, stack);
	PushNullChar(stack);
	
	AddToStringTable(stack);
	
	FunctionEnd();
	return (char*)stack.memory;
}

dll_export ui16 GetStringLength(const char* s) {
  FunctionStart(0);
	AssertInternal(s != Null);
  
	auto length = strlen(s);
	AssertInternal(length <= UI16Max);
  
	FunctionEnd();
  return length;
};

dll_export char* ToString(si8 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%hhi", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(ui8 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%hhu", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(si16 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%hi", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(ui16 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%hu", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(si32 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%li", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(ui32 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%lu", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(si64 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%lli", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(ui64 i) {
  FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%llu", i);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(f32 f) {
	FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%.2f", f);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export char* ToString(f64 f) {
	FunctionStart(Null);
	
	char buffer[32] = { '\0' };
  sprintf(buffer, "%.2Lf", f);
	auto ret = AllocateString((char*)buffer, Null);
	
	FunctionEnd();
	return ret;
}

dll_export bool StringsAreEqual(const char* s1, const char* s2) {
  FunctionStart(false);
	
	AssertInternal(s1 != Null);
	AssertInternal(s2 != Null);
	
	FunctionEnd();
	return strcmp(s1, s2) == 0;
}

dll_export const char* FindSubstring(const char* sub, const char* string) {
  FunctionStart(Null);
	
	AssertInternal(string != Null);
  AssertInternal(sub != Null);
  auto ret = strstr(string, sub);
	
	FunctionEnd();
	return ret;
}

dll_export bool ContainsAnySubstring(const char* string, const char** substrings, ui8 length) {
  FunctionStart(false);
	
	ForAll(length) {
    auto* sub = substrings[it];
		AssertInternal(sub != Null);
		if(FindSubstring(sub, string) != Null) {
			FunctionEnd();
			return true;
		}
	}
	
	FunctionEnd();
  return false;
}

dll_export void CopyString(const char* source, si16 srcLength, const char* destination, ui16 destLength) {
	FunctionStart(;);
	
	AssertInternal(source != Null);
	AssertInternal(srcLength > 0 || srcLength == -1);
	AssertInternal(destination != Null);
	AssertInternal(destLength > 0);
	
	auto copyLength = srcLength == -1 ? (GetStringLength(source) + 1) : srcLength;
	AssertInternal(copyLength <= destLength);
	
	CopyMemory((void*)source, copyLength, (void*)destination);
	
	FunctionEnd();
}

dll_export bool IsLetter(char c) {
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

dll_export bool IsWord(char* string) {
	FunctionStart(false);
	AssertInternal(string != Null);
	
	auto length = GetStringLength(string);
	ForAll(length) {
		if(IsLetter(string[it]) == false)
			return false;
	}
	
	FunctionEnd();
	return true;
}

dll_export bool IsNumber(char c) {
	return c >= '0' && c <= '9';
}

dll_export bool IsNumber(char* string) {
	FunctionStart(false);
	AssertInternal(string != Null);
	
	auto length = GetStringLength(string);
	ForAll(length) {
		if(IsNumber(string[it]) == false)
			return false;
	}
		
	FunctionEnd();
	return true;
}

#include <stdlib.h>
dll_export si32 StringToInt(const char* string, ui16 length) {
  FunctionStart(0);
	AssertInternal(string != Null);
	
	si32 ret = Null;
	
	if(length == Null)
		ret = atoi(string);
	else { // Want to convert only part of the string or string is an array without a null char
		auto copy = AllocateString(string, length);
		ret = atoi(copy);
		FreeString(copy);
	}
	
	FunctionEnd();
	return ret;
}

dll_export char* ExtractSubstring(const char* s, ui16 length) {
	FunctionStart(Null);
	AssertInternal(s != Null);
	
	auto sLength = GetStringLength(s);
	AssertInternal(sLength > 0);
	
	ui8  copyLength = 0;
	if(length == Null || length > sLength)
		copyLength = sLength;
	else
		copyLength = length;
	
	auto stack = AllocateStack(copyLength + 1);
	// Use this instead of PushString() since the latter will push the entire string
	// instead of just a section if that is what's wanted.
	void* mem = Push((void*)s, copyLength, stack); 
	PushNullChar(stack);
	
	AddToStringTable(stack);
	
	FunctionEnd();
	return (char*)stack.memory;
}

dll_export void FreeString(char* string) {
	FunctionStart(;);
	AssertInternal(string != Null);
	
	ui32  blocks = stringTable.size / sizeof(memory_block);
	auto* table = (memory_block*)stringTable.memory;
	ForAll(blocks) {
		auto* block = table + it;
		if(block->memory == string) {
			FreeMemory(*block);
			// @TODO - Reset the block when pool allocation functionality implemented
			break;
		}
	}
	
	FunctionEnd();
}

dll_export bool StringIsEqualToAny(const char* string, const char** strings, ui8 count) {
	FunctionStart(false);
	AssertInternal(string != Null);
	AssertInternal(strings != Null);
	AssertInternal(count > 0);
	
	ForAll(count) {
		auto s = strings[it];
		AssertInternal(s != Null);
		if(StringsAreEqual(string, s) == true) {
			FunctionEnd();
			return true;
		}
	}
	
	FunctionEnd();
	return false;
}