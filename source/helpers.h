#ifndef HELPERS_H
#define HELPERS_H

#include "apad_base_types.h"

bool IsValidChar(char c);
bool TagIsValid(const char* tag);
void PrintDetailedTask(const char* id, const char* task, const char* dateAdded, const char* dateDue, const char** tags);

void DisplayTaskStringHelpMessage();
void DisplayDateAddedHelpMessage();
void DisplayDateDueHelpMessage();
void DisplayTagsHelpMessage();

#endif