#ifndef HELPERS_H
#define HELPERS_H

#include "apad_base_types.h"
#include "apad_intrinsics.h"

typedef ui16 guid;
const   ui8  MaxTags = 5;

struct todoListEntry {
	guid  ID;
	char* task; 		 		 // Must have
	char* dateAdded; 		 // Must have
	char* dateDue; 	 		 // Can be Null
	char* tags[MaxTags]; // Can all be Null
};

program_external const ui8 MaxTaskPrintLength;

#include <stdio.h>
#define PrintError(_string) \
	printf("\nERROR: %s\n", _string)
	
#define PrintErrorExit(_string) { \
	PrintError(_string); \
	goto program_exit; \
}

#define TodoEntriesLoop(_todoList)    ForAll((_todoList).size / sizeof(todoListEntry))
#define GetTodosEntry(_todoList, _it) (((todoListEntry*)(_todoList).memory) + it)

program_unique const char* ValidCommands[] = 	{ "add", "list", "del", "mod", "undo" };
program_unique BeginEnum(ValidCommandsIndex) { Add, List, Delete, Modify, Undo, Length } EndEnum(ValidCommandsIndex);

program_unique const char* ValidArguments[] =   { "-id", "-s", "-da", "-dd", "-t", "-t1", "-t2", "-t3", "-t4", "-t5" };
program_unique BeginEnum(ValidArgumentsIndex) { ID, TaskString, DateAdded, DateDue, TagsGeneric, Tag1, Tag2, Tag3, Tag4, Tag5, Length } EndEnum(ValidArgumentsIndex);

bool DateDueIsUnspecified(const char* dateDue);
si32 GetDaysOffsetFromToday(const char* targetDate);

#include "apad_memory.h"
todoListEntry* FindEntry(const char* id, memory_stack& todoList);

bool IsValidChar(char c);
bool AnyTagsPresent(char** tags);
bool TagIsValid(const char* tag);

void PrintTaskVertical(ui16 id, char* task, char* dateAdded, char* dateDue, char** tags);

void DisplayCommandOptions(bool id, bool taskString, bool dateAdded, bool dateDue, bool tags);
void PrintLogMessage(const char* string);

#include "apad_memory.h"
char* GetBackupTodosFilePath(const char* filePath);
void  SaveTodosFile(memory_stack& todoList, const char* dataPath);
void  SaveTodosFileBackup(memory_stack& todoList, const char* dataPath);
void 	UpdateLogFile(const char* string, memory_stack& logFile, const char* path);

#endif