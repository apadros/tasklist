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
		printf("    %s [< | <= | > | >=][dd/mm | dd/mm/yyyy][[+ | -]<days>] date due\n", (const char*)ValidArguments[ValidArgumentsIndex::DateDue]);
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

// Any changes to this function must be reflected in SaveTodosFile()
void SaveTodosFileBackup(memory_stack& todoList, const char* dataPath) {
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
void PrintDetailedTask(ui16 id, char* task, char* dateAdded, char* dateDue, char** tags) {
  // @TODO - Add assertions once program takes shape
	// AssertRet(id != Null);
	// AssertRet(task != Null);
	// AssertRet(dateAdded != Null);
	// AssertRet(dateDue != Null);
	// AssertRet(tags!= Null);

	printf("\n  ID:         %u\n", id);
	printf("  String:     %s\n", task);
	printf("  Date added: %s\n", dateAdded);
	printf("  Date due:   %s", dateDue == Null ? "-\n" : dateDue);
	if(dateDue != Null) {
		si32 diffDays = GetDaysOffsetFromToday(dateDue);
		
		 if(diffDays > 0)
		   printf(" (+%i)\n", diffDays);
		 else
		   printf(" (%i)\n", diffDays);
	}
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

// @TODO - PrintTaskWide() - add support for a batch to to ascertain each colum length
void PrintTaskWide(const char* id, const char* task, const char* dateAdded, const char* dateDue, const char** tags) {
	#if 0
	AssertRet(task != Null);

	// @TODO - Add assertions once program takes shape
	// AssertRet(id != Null);
	// AssertRet(task != Null);
	// AssertRet(dateAdded != Null);
	// AssertRet(dateDue != Null);
	// AssertRet(reschedulePeriod	!= Null);
	// AssertRet(flag != Null);
	// AssertRet(groups != Null);

	// @TODO - Make it so dates which coincide with current year are displayed in dd/mm format

	id = Null; // @TODO - Update once IDs are implemented

	const char* headers[] = { "ID", "String", "Date added", "Date due", "Tags" }; //
	const ui8   headersCount = GetArrayLength(headers);
	const char* contents[] = { id, task, dateAdded, dateDue, reschedulePeriod, flag };
	ui16        lengths[headersCount] = { };
	ForAll(headersCount) {
		const char* header = headers[it];
		const char* content = "-";
		if(contents[it] != Null)
			content = contents[it];

		auto headerLength = GetStringLength(header);
	  auto contentLength = GetStringLength(content);

	  lengths[it] = Max(headerLength, contentLength) + 2;
	}

	// Print headers
	ui16 totalHeadersLength = 0;
	ForAll(headersCount) {
		const char* header = headers[it];
		ui16 finalLength = lengths[it];

		printf(" %s ", header);
		ui16 headerLength = GetStringLength(header);
		totalHeadersLength += 1 + headerLength + 1;
		si16 printLength = finalLength - (headerLength + 2);
		if(printLength > 0) {
			ForAll(printLength) {
				printf(" ");
				totalHeadersLength += 1;
			}
		}
		printf("|");
		totalHeadersLength += 1;
	}

	// Groups
	{
		const char* string = " Groups ";
	  printf(string);
	  totalHeadersLength += GetStringLength(string);
	}
	printf("\n");

	// Print separator
	ForAll(totalHeadersLength)
	  printf("=");
	printf("\n");

	// Print content
	ForAll(headersCount) {
		const char* content = "-";
		if(contents[it] != Null)
			content = contents[it];
		ui16 finalLength = lengths[it];

		printf(" %s ", content);
		si16 printLength = finalLength - (GetStringLength(content) + 2);
		if(printLength > 0) {
			ForAll(printLength)
				printf(" ");
		}
		printf("|");
	}

	// Groups
	ForAll(MaxTags) {
		const char* group = groups[it];
		if(group == Null)
			continue;
		if(it > 0)
			printf(",");
		printf(" %s", groups[it]);
	}
	#endif
}