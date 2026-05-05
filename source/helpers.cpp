#include <stdio.h>
#include "helpers.h"

bool IsValidChar(char c) {
  return IsLetter(c) == true || IsNumber(c) == true || c == '\"' || c == '/' || c == '-' || c == '?' || c == '!' || c == '#';
}

bool TagIsValid(const char* tag) {
	return tag != Null;
}

void DisplayCommandOptions(bool id, bool taskString, bool dateAdded, bool dateDue, bool tags) {
	printf("\n  Options\n");
	if(id == true)
		printf("    %s  [id]                            ID\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
	if(taskString == true)
		printf("    %s  [<text string>]                 task text\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
	if(dateAdded == true)
		printf("    %s [dd/mm | dd/mm/yyyy]            date added\n", (const char*)ValidArguments[ValidArgumentsIndex::DateAdded]);
	if(dateDue == true)
		printf("    %s [dd/mm | dd/mm/yyyy | +ddd[w]]  date due\n", (const char*)ValidArguments[ValidArgumentsIndex::DateDue]);
	if(tags == true)
		printf("    [[%s <tags>] | [%s<1-5>]]           string tags (up to 5) or %s(number 1 to 5) to set a specific tag\n", (const char*)ValidArguments[ValidArgumentsIndex::TagsGeneric], (const char*)ValidArguments[ValidArgumentsIndex::TagsGeneric], (const char*)ValidArguments[ValidArgumentsIndex::TagsGeneric]);
}

#include "apad_file.h"
#include "apad_string.h"
void SaveChangesToTodosFile(memory_stack& todoList, const char* dataPath) {
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

void PrintDetailedTask(ui16 id, const char* task, const char* dateAdded, const char* dateDue, const char** tags) {
  // @TODO - Add assertions once program takes shape
	// AssertRet(id != Null);
	// AssertRet(task != Null);
	// AssertRet(dateAdded != Null);
	// AssertRet(dateDue != Null);
	// AssertRet(tags!= Null);
	
	printf("\n  ID:         %u\n", id);
	printf("  String:     %s\n", task);
	printf("  Date added: %s\n", dateAdded);
	printf("  Date due:   %s\n", dateDue);
	printf("  Tags:       ");
	if(TagIsValid(tags[0]) == false) // If no tags supplied
		printf("-\n");
	else { 
		printf("%s\n", (char*)tags[0]);
		FromTo(1, MaxTags) {
			if(TagIsValid(tags[it]) == true)
				printf("              %s\n", (char*)tags[it]);
		}
	}
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