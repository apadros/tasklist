#ifndef HELPERS_H
#define HELPERS_H

#include "apad_base_types.h"

typedef ui16 guid;
const   ui8  MaxTags = 5;

struct todoListEntry {
	guid  ID;
	char* task; 		 		 // Must have
	char* dateAdded; 		 // Must have
	char* dateDue; 	 		 // Can be Null
	char* tags[MaxTags]; // Can all be Null
};

#include <stdio.h>
#define PrintError(_string) \
	printf("\nERROR: %s\n", _string)
	
#define PrintErrorExit(_string) { \
	PrintError(_string); \
	goto program_exit; \
}

#define TodoEntriesLoop(_todoList)    ForAll((_todoList).size / sizeof(todoListEntry))
#define GetTodosEntry(_todoList, _it) (((todoListEntry*)(_todoList).memory) + it)

bool IsValidChar(char c);
bool TagIsValid(const char* tag);
void PrintDetailedTask(ui16 id, const char* task, const char* dateAdded, const char* dateDue, const char** tags);
void DisplayCommandOptions(bool id, bool taskString, bool dateAdded, bool dateDue, bool tags);
void SaveChangesToTodosFile(memory_stack& todoList, const char* dataPath);

#endif