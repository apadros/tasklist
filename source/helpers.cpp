#include <stdio.h>

bool IsValidChar(char c) {
  return IsLetter(c) == true || IsNumber(c) == true || c == '\"' || c == '/' || c == '-' || c == '?' || c == '!' || c == '#';
}

bool TagIsValid(const char* tag) {
	return tag != Null;
}

void DisplayTaskStringHelpMessage() {
	printf("   %s  [<text string>]                 task text\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
}

void DisplayDateAddedHelpMessage() {
	printf("   %s [dd/mm | dd/mm/yyyy]            date added\n", (const char*)ValidArguments[ValidArgumentsIndex::DateAdded]);
}

void DisplayDateDueHelpMessage() {
	printf("   %s [dd/mm | dd/mm/yyyy | +ddd[w]]  date due\n", (const char*)ValidArguments[ValidArgumentsIndex::DateDue]);
}

void DisplayTagsHelpMessage() {
	printf("   %s  [<tags>...]                     string tags (up to 5)\n", (const char*)ValidArguments[ValidArgumentsIndex::Tags]);
}

void PrintDetailedTask(const char* id, const char* task, const char* dateAdded, const char* dateDue, const char** tags) {
  // @TODO - Add assertions once program takes shape
	// AssertRet(id != Null);
	// AssertRet(task != Null);
	AssertRet(dateAdded != Null);
	// AssertRet(dateDue != Null);
	AssertRet(tags!= Null);
	
	id = "-"; // @TODO - Update once IDs are implemented
	
	printf("  ID:         %s\n", id);
	printf("  String:     %s\n", task);
	printf("  Date added: %s\n", dateAdded);
	printf("  Date due:   %s\n", dateDue);
	printf("  Tags:       ");
	if(tags == Null) // If no tags supplied
		printf("-\n");
	else { 
		printf("%s\n", (char*)tags[0]);
		FromTo(1, MaxTags) {
			if(tags[it] != Null)
				printf("              %s\n", (char*)tags[it]);
		}
	}
}

// @TODO - PrintTaskWide() - add support for a batch to to ascertain each colum length
void PrintTaskWide(const char* id, const char* task, const char* dateAdded, const char* dateDue, const char* reschedulePeriod, const char* flag, const char** groups) {
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
	
	const char* headers[] = { "ID", "String", "Added", "Due", "Reschedule", "Flag" }; // Groups implemented separately
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
}