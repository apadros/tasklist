#include <stdio.h>
#include "apad_error.h"
#include "helpers.h"

#include <time.h>
#include "apad_time.h"
program_local tm ConvertDateToCSLTime(date& d) {
	tm ret = {};
	ret.tm_wday = d.dayOfTheWeek == 7 ? 0 : d.dayOfTheWeek;
	ret.tm_mday = d.day;
	ret.tm_mon = d.month - 1;
	ret.tm_year = d.year - 1900;
	return ret;
}

#include "apad_string.h"
bool IsValidChar(char c) {
  return IsLetter(c) == true || IsNumber(c) == true || c == '\"' || c == '/' || c == '-' || c == '?' || c == '!' || c == '#';
}

bool TagIsValid(const char* tag) {
	return tag != Null;
}

#include "apad_string.h"
bool DateDueIsUnspecified(const char* dateDue) {
	return GetStringLength(dateDue) == 1 && dateDue[0] == '-';
}

bool AnyTagsPresent(char** tags) {
	AssertRetType(tags != Null, false);
	ForAll(MaxTags) {
		if(TagIsValid(tags[it]) == true)
			return true;
	}
	return false;
}

#include "apad_file.h"
void UpdateLogFile(const char* string, memory_stack& logFile, const char* path) {
	Assert(string != Null);
	Assert(path != Null);
	PushString(string, false, logFile);
	SaveFile(logFile.memory, logFile.size, path);
}

todoListEntry* FindEntry(const char* id, memory_stack& todoList) {
	auto ID = StringToInt(id, Null);
	TodoEntriesLoop(todoList) {
		auto* entry = GetTodosEntry(todoList, it);
		if(entry->ID == ID)
			return entry;
	}
	return Null;
}

#include "apad_file.h"
#include "apad_string.h"
char* GetBackupTodosFilePath(const char* filePath) {
	AssertRetType(filePath != Null, Null);
	char* backupPath = AllocateString(filePath, Null);
	*((char*)(GetFileExtension(backupPath) - 1)) = '\0'; // Remove the . and extension
	return Concatenate(3, backupPath, "_backup.", GetFileExtension(filePath)); // Append "_backup.extension"
}

void DisplayCommandOptions(bool id, bool taskString, bool dateAdded, bool dateDue, bool tags) {
	printf("\n  Options\n");
	if(id == true)
		printf("    %s  [id]                                                 ID\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
	if(taskString == true)
		printf("    %s  [<text string>]                                      task text\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
	if(dateAdded == true)
		printf("    %s [< | <= | > | >=][dd/mm | dd/mm/yyyy][[+ | -]<days>] date added\n", (const char*)ValidArguments[ValidArgumentsIndex::DateAdded]);
	if(dateDue == true)
		printf("    %s [< | <= | > | >=][dd/mm | dd/mm/yyyy][[+ | -]<days>] date due, use of operators requires quotation marks around all of it (e.g. \"<date\")\n", (const char*)ValidArguments[ValidArgumentsIndex::DateDue]);
	if(tags == true)
		printf("    [[%s [<tags> | \"\"]] | [%s<1-5> [<tag> | \"\"]]]            string tags (up to 5) or %s(number 1 to 5) to set a specific tag or \"\" to remove a tag\n", (const char*)ValidArguments[ValidArgumentsIndex::TagsGeneric], (const char*)ValidArguments[ValidArgumentsIndex::TagsGeneric], (const char*)ValidArguments[ValidArgumentsIndex::TagsGeneric]);
}

#include "apad_file.h"
#include "apad_string.h"
// Any changes to this function must be reflected in SaveTodosFileBackup()
void SaveTodosFile(memory_stack& todoList, const char* dataPath) {
	AssertRet(dataPath != Null);
	
	auto file = CreateFile();
	TodoEntriesLoop(todoList) {
		auto* entry = GetTodosEntry(todoList, it);
		Assert(entry->task != Null);
		Assert(entry->dateAdded != Null);

		char* string = Concatenate(7, "\"", entry->task, "\" ", entry->dateAdded, " ", entry->dateDue == Null ? "-" : entry->dateDue, " ");
		WriteToFile(string, file);

		bool tagsFound = false;
		ForAll(MaxTags) {
			if(TagIsValid(entry->tags[it]) == true) {
				WriteToFile(Concatenate(3, "\"", entry->tags[it], "\" "), file);
				tagsFound = true;
			}
		}
		if(tagsFound == false)
			WriteToFile("- ", file);

		WriteToFile("\r\n", file);
	}
	
	SaveFile(file.memory, file.size, dataPath);
	
	FreeFile(file);
}

void PrintLogMessage(const char* string) {
	printf("\n[LOG] %s\n", string);
}

// Any changes to this function must be reflected in SaveTodosFile()
void SaveTodosFileBackup(memory_stack& todoList, const char* dataPath) {
	if(todoList.size == 0) // Not an error in case of new-created data/todos.txt
		return;
	
	AssertRet(dataPath);
	
	auto file = CreateFile();
	TodoEntriesLoop(todoList) {
		auto* entry = GetTodosEntry(todoList, it);
		Assert(entry->task != Null);
		Assert(entry->dateAdded != Null);

		char* string = Concatenate(7, "\"", entry->task, "\" ", entry->dateAdded, " ", entry->dateDue == Null ? "-" : entry->dateDue, " ");
		WriteToFile(string, file);

		bool tagsFound = false;
		ForAll(MaxTags) {
			if(TagIsValid(entry->tags[it]) == true) {
				WriteToFile(Concatenate(3, "\"", entry->tags[it], "\" "), file);
				tagsFound = true;
			}
		}
		if(tagsFound == false)
			WriteToFile("- ", file);

		WriteToFile("\r\n", file);
	}
	
	char* backupPath = GetBackupTodosFilePath(dataPath);
	SaveFile(file.memory, file.size, backupPath);
}

#include <time.h>
#include "apad_time.h"
si32 GetDaysOffsetFromToday(const char* targetDate) {
	Assert(targetDate != Null);
	
	auto todayDate = GetDate(0);
	auto todayCSL = ConvertDateToCSLTime(todayDate);
	auto todayTime = mktime(&todayCSL);
	
	auto targetDateDate = StringToDate(targetDate);
	auto targetDateCSL = ConvertDateToCSLTime(targetDateDate);
	auto targetDueTime = mktime(&targetDateCSL);
		
	auto diffSecs = difftime(targetDueTime, todayTime);
	si32 diffDays = diffSecs / 60 / 60 / 24;
	return diffDays;	
}

#include "apad_string.h"
void PrintTaskVertical(ui16 id, char* task, char* dateAdded, char* dateDue, char** tags, ui8 maxTaskColumWidth) {
	Assert(task != Null);
	Assert(dateAdded != Null);
	Assert(tags != Null);
	Assert(maxTaskColumWidth > 0);
	
	printf("\n");
	printf("  ID:         %u\n", id);
	
	printf("  String:     ");
	auto length = GetStringLength(task);
	ForAll(length) {
		printf("%c", task[it]);
		if(it > 0 && it % maxTaskColumWidth == 0)
			printf("\n              ");
		else if(it == length - 1)
			printf("\n");
	}
	
	printf("  Date added: %s\n", dateAdded);
	printf("  Date due:   %s", dateDue == Null ? "-\n" : dateDue);
	if(dateDue != Null)
		printf(" (%+i)\n", GetDaysOffsetFromToday(dateDue));
	printf("  Tags:       ");

	if(AnyTagsPresent(tags) == true) {
		bool firstTagPrinted = false;
		ForAll(MaxTags) {
			if(TagIsValid(tags[it]) == true) {
				if(firstTagPrinted == false) {
					printf("%s\n", (char*)tags[it]);
					firstTagPrinted = true;
				}
				else
					printf("              %s\n", (char*)tags[it]);
			}
		}
	}
	else
		printf("-\n");
}