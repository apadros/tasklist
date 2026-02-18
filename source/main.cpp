#define APAD_DEBUG

#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#include "apad_array.h"
#include "apad_base_types.h"
#include "apad_error.h"
#include "apad_file.h"
#include "apad_intrinsics.h"
#include "apad_maths.h"
#include "apad_string.h"
#include "apad_time.h"
#include "apad_win32.h"

			string DateFormatShort  = "dd/mm";
			string DateFormatMedium = "dd/mm/yy";
			string DateFormatLong   = "dd/mm/yyyy";
const ui8 	 MaxTags = 5;

#include "helpers.cpp"

string 		ValidCommands[] = 	{ "add", "list", "del", "mod", "undo", "redo" };
BeginEnum(ValidCommandsIndex) { Add, List, Delete, Modify, Undo, Redo, Length } EndEnum(ValidCommandsIndex);

string    ValidOptions[] =   { "-s", "-da", "-dd", "-t" };
BeginEnum(ValidOptionsIndex) { TaskString, DateAdded, DateDue, Tags, Length } EndEnum(ValidOptionsIndex);


struct taskListEntry {
	string task;
	string dateAdded;
	string dateDue;
	char   flag;
	string tags[MaxTags];
};

#include <stdio.h>
#define PrintErrorExit(_string) { \
	printf("ERROR: %s\n", _string); \
	goto program_exit; \
}

ConsoleAppEntryPoint(args, argsCount) {
	#ifdef APAD_DEBUG
		#if 1
		char* debugArgs[] = { args[0], "add", "-s", "task text", "-t", "tag1", "tag2", "-dd", "20/02" };
		args = debugArgs;
		argsCount = GetArrayLength(debugArgs);	
		#endif
	#endif
	
	if(argsCount == 1) {
		printf("Usage: %s [add] [list] [del | delete] [mod | modify] [undo] [redo]\n\n", args[0]);
		printf("Options\n");
		printf("	%s  [<text string>]                 task text\n", (const char*)ValidOptions[ValidOptionsIndex::TaskString]);
		printf("	%s [dd/mm | dd/mm/yyyy]            date added\n", (const char*)ValidOptions[ValidOptionsIndex::DateAdded]);
		printf("	%s [dd/mm | dd/mm/yyyy | +ddd[w]]  date due\n", (const char*)ValidOptions[ValidOptionsIndex::DateDue]);
		printf("	%s  [<tags>...]                     string tags (up to 5)\n", (const char*)ValidOptions[ValidOptionsIndex::Tags]);
		// @TODO - Add what options can be specified with every command, figure out a simple way
		goto program_exit;
	}
	
	// Data to be parsed from arguments
	// Arguments not needed for the required command will be ignored
	string command;
	string taskString;
	string dateAdded;
	string dateDue;
	string tags[MaxTags];
	ui8    tagsPresent = 0;

	// Parse and check command
	command = args[1];
	{
		bool found = false;
		ForAll(ValidCommandsIndex::Length) {
			if(command == ValidCommands[it]) {
				found = true;
				break;
			}
		}
		
		if(found == false)
			PrintErrorExit("Invalid command\n");
	}
	
	#define CheckArgsExit() if(it >= argsCount) \
														PrintErrorExit("Not enough arguments supplied.");
			
	
	// Parse options
	FromTo(2, argsCount) {
		string arg = args[it];
		
		if(arg == ValidOptions[ValidOptionsIndex::TaskString]) {
			it += 1;
			CheckArgsExit();
			taskString = args[it]; // @TODO - Check correctness
		}
		else if(arg == ValidOptions[ValidOptionsIndex::DateAdded]) {
			it += 1;
			CheckArgsExit();
			dateAdded = args[it]; // @TODO - Check correctness
		}
		else if(arg == ValidOptions[ValidOptionsIndex::DateDue]) {
			it += 1;
			CheckArgsExit();
			dateDue = args[it]; // @TODO - Check correctness
			
			// Convert to long date format
			if(IsDate(dateDue) == true)
				dateDue = DateToString(StringToDate(dateDue)); // @TODO - Simplify this?
			else
				PrintErrorExit("Incorrect date due format");
		}
		else if(arg == ValidOptions[ValidOptionsIndex::Tags]) {
			// Scan arguments and store up to MaxTags or end of arguments so long as none are valid options
			ui8 count = 0;
			while(count < MaxTags) {
				it += 1;
				CheckArgsExit();
				
				string s = args[it];
				bool option = FindSubstring("-", s);
				if(option == false)
					tags[count++] = s;
				else {
					tagsPresent = count;
					it -= 1;
					break;
				}
			}
		}
		
		#if 0
		else if(IsDate(arg) == true || (arg.length == 1 && arg[0] == '.') || (arg.length >= 2 && arg.length <= 4 && arg[0] == '+')) { // Date
      if(arg[0] == '.') {
				if(dateDue.length == 0)
					dateDue = DateToString(GetDate(0));
				else {
					PrintErrorExit("Target date already supplied: %s\n", (char*)arg);
					goto program_exit;
				}
			}
			else if(arg[0] == '+') {
				string daysString = arg.chars + 1;
				
				// Determine validity and whether work days have been specified
				bool isValid = true;
				bool workDays = false;
				{
					const ui8 MaxDigits = 3;
					
					const char* workDaysSub = FindSubstring("w", daysString);
					if(workDaysSub != Null) {
						workDays = true;
						workDaysSub = '\0';
					}
					
					if(daysString.length == 0 || daysString.length > MaxDigits)
						isValid = false;
					
					ForAll(daysString.length) {
						if(IsNumber(daysString[it]) == false)
							isValid = false;
					}
				}
				if(isValid == false) {
					PrintErrorExit("Invalid day offset (max length allowed is 3)\n");
					goto program_exit;
				}
				
				if(dateDue.length == 0) {
					ui16 calendarDays = 0;
					{
						si32 days = StringToInt(daysString);
						if(workDays == true) {
							ForAll(days) {
								calendarDays += 1;
								while(GetDate(calendarDays).dayOfTheWeek >= 6) // Weekend
									calendarDays += 1;
							}
						}
						else
							calendarDays = days;
					}
				
					dateDue = DateToString(GetDate(calendarDays));
				}
				else if(reschedulePeriod.length == 0)
					reschedulePeriod = arg.chars + 1;
				else {
					PrintErrorExit("Reschedule period already supplied\n", (char*)arg);
					goto program_exit;
				}						
			}
			else {
				if(dateDue.length == 0)
					dateDue = DateToString(StringToDate(arg)); // Conversion back and forth to set the standard date format dd/mm/yyyy
				else {
					PrintErrorExit("Target date already supplied\n", arg);
					goto program_exit;
				}
			}
		}
		else if(arg.length == 1) { // Flag
			if(arg[0] == '!' || arg[0] == '?' || arg[0] == '@')
				flag = arg;
			else {
				PrintErrorExit("Invalid flag supplied: %s\n", (char*)arg);
				goto program_exit;
			}
		}
		else if(arg == "-t") { // Tags
			it += 1;
			
			if(it == argsCount) {
				PrintErrorExit("No tags specified\n");
				goto program_exit;
			}
			
			// Everything following this switch is considered to be a tag
			do {
				arg = args[it];
				
				bool added = false;
				ForAll(MaxTags) {
					if(tags[it].length == 0) {
						tags[it] = arg;
						added = true;
						break;
					}
				}
				
				if(added == false) {
					PrintErrorExit("Number of tags exceeded, max 5\n");
					goto program_exit;
				}
				
				it += 1;
			}
			while(it < argsCount);
		}
		else { // Invalid argument
			PrintErrorExit("Invalid argument: %s\n", (char*)arg);
			goto program_exit;
		}
		#endif
	}	
	
	#ifdef APAD_DEBUG
	string dataPath = "..\\..\\data\\tasklist.txt";
	#else
	string dataPath = "data/tasklist.txt";
	#endif
	
	// Open the tasks file and generate task list
	file tasksFile = {};
	{
		 
				
		if(Win32FileExists(dataPath) == false) // @TODO - Replace with File API function once bug is fixed
			PrintErrorExit("Couldn't find data/tasklist.txt\n");
		
		tasksFile = Win32LoadFile(dataPath); // @TODO - Replace with File API function once bug is fixed
		if(ErrorIsSet() == true)
			PrintErrorExit("Couldn't load data/tasklist.txt");
		
		#if 0
		
		// Parse tasks - see data/format.txt
		
		// Extract data
		auto  taskList = AllocateStack(128);
		auto  line = AllocateStack(128);
		char* data = Null;
		bool  readingTaskString = false;
		for(ui32 it = 0; it < tasksFile.size; it += 1) {
			char* c = (char*)tasksFile.memory + it;
			
			if(*c == '"') {
				if(readingTaskString == false)
					data = c + 1;
				else
					*c = ' ';
				
				Toggle(readingTaskString);
			}
			
			if(*c == '\n') { // Add task to list
				#if 0
				struct taskListEntry {
					string task;
					string dateAdded;
					string dateDue;
					ui8    reschedulePeriod;
					char   flag;
					string tags[MaxTags];
				};
				#endif
				
				Assert(line.size % sizeof(char*) == 0);
				char** lineArray = (char**)line.memory;
				auto*  task = (taskListEntry*)Push(sizeof(taskListEntry), taskList);
				ClearMemory(task, sizeof(taskListEntry));
				task->task = lineArray[0];
				task->dateAdded = lineArray[1]; // @TODO - We're getting an access violation here for some reason.
				PreventCompilation;
				task->dateDue = lineArray[2];
				
				string resc = lineArray[3];
				if(resc.length == 1 && resc[0] == '-')
					task->reschedulePeriod = 0;
				else
					task->reschedulePeriod = StringToInt(resc);
				task->flag = *(lineArray[4]);
				ui8 totalEntries = line.size / sizeof(char*);
				Assert(totalEntries - 5 <= MaxTags);
				FromTo(5, totalEntries)
					task->tags[it - 5] = lineArray[it];
					
				ResetStack(line);
			}
			
			if(data == Null && IsWhitespace(*c) == false) // Start of new data
				data = c;
			else if(readingTaskString == false && IsWhitespace(*c) == true && data != Null) { // End of current data
				PushInstance(data, line);
				*c = '\0';
				data = Null;
			}
		}
		
		#endif
	}
	
	// Parse command, output error message if invalid
	if(command == ValidCommands[ValidCommandsIndex::Add]) {
		// @CURRENT @BUG - Too much encapsulation in API time -> string -> memory, string data is overwritten without meaning to, leads to the bug underneath.
		dateAdded = DateToString(GetDate(0)); // @BUG - Overwrites both dateDue and dateAdded
				
		auto stack = AllocateStack(128);
		PushString((string)"\"" + taskString + "\" ", false, stack);
		PushString(dateAdded + " ", false, stack);
		PushString(dateDue + " ", false, stack);
		if(tagsPresent == 0)
			PushString("-", false, stack);
		else {
			ForAll(tagsPresent)
				PushString((string)"\"" + tags[it] + "\" ", false, stack);
		}
				
		PushString("\r\n", false, stack);
		
		Win32SaveFile(stack.memory, stack.size, dataPath);
		
		FreeStack(stack);
		
		printf("\nTask added\n");
		PrintDetailedTask(Null, taskString, dateAdded, dateDue, tags);
		
		goto program_exit;
	}
	else if(command == ValidCommands[ValidCommandsIndex::List]) {
		#if 0
	  // @TODO
		// By string, ID, flags & tags, - means not
		// <60 days & >60 days
		// Automatically list preempt tasks with a date <30 days when listing priority tasks?
		
		// @TODO - Parse calendar entries and display
		// id(8 bit unsigned) task_text(string) date_added(dd/mm/yyyy) date_due_by(dd/mm/yyyy | -) reschedule_data(days | -) flag(! | ? | @) tags(#id1 #id2 ... #id5)
		
		// @TODO - search / list filters - view only task, id, all tasks from #group, etc
		// 			 	- E.g. view only the tags of tasks due by x date
		
		// @TODO - Specify which info columns are wanted - -da (date added) -dd (date due) etc ?
		
		// Scan through remaining arguments
		bool printDetailed = false;
		FromTo(2, argsCount) {
			const char* arg = args[it];
			if(arg == "-d")
				printDetailed = true;
		}
		
		char*       data = (char*)calendar.memory;
		const char* id = Null;
		const char* task = Null;
		const char* dateAdded = Null;
		const char* dateDue = Null;
		const char* reschedulePeriod = Null;
		const char* flag = Null;
		const char* tags[MaxTags] = { Null };
		
		const char* toStore = Null; // Temp string
		bool        scanningTaskString = false;
		ForAll(calendar.size) {
			char c = data[it];
			
			if(c == '\n') { // Reset task data
				if(printDetailed == true)
					PrintDetailedTask(id, task, dateAdded, dateDue, reschedulePeriod, flag, tags);
				else
					PrintTaskWide(id, task, dateAdded, dateDue, reschedulePeriod, flag, tags);
				
				id = Null;
				task = Null;
				dateAdded = Null;
				dateDue = Null;
				reschedulePeriod = Null;
				flag = Null;
				
				ForAll(MaxTags)
					tags[it] = Null;
					
				printf("\n");
			}
			else if(toStore == Null && IsValidChar(c) == true) {
				if(c == '\"') { // Beginning of the task string
					toStore = data + it + 1;
					scanningTaskString = true;
				}
				else
					toStore = data + it;
			}
			else if(scanningTaskString == true && c == '\"') { // Finish scanning task string
			  data[it] = '\0';
				task = toStore;
				toStore = Null;
				scanningTaskString = false;
			}
			else if(scanningTaskString == false && toStore != Null && IsValidChar(c) == false) { // Finish scanning other data
				data[it] = '\0';
				
				if(dateAdded == Null)
					dateAdded = toStore;
				else if(dateDue == Null)
					dateDue = toStore;
				else if(reschedulePeriod == Null)
					reschedulePeriod = toStore;
				else if(flag == Null)
					flag = toStore;
				else if(task != Null) {
					ForAll(MaxTags) {
						if(tags[it] == Null) {
							tags[it] = toStore;
							break;
						}
					}
				}
				
				toStore = Null;
			}
		}
		#endif
	}
	else if(command == ValidCommands[ValidCommandsIndex::Modify]) { // Modify - @TODO
		// @TODO
		// Print the details which have been updated, followed by the entire task data
		// - E.g. Task ID / flag / group updated
		// - Keep it simple
		// When making any changes to an entry, display the updated portion of info 
		// before and after, then display the updated entry will all info

	}
	else {
		PrintErrorExit("Invalid command supplied.\n");
		goto program_exit;
	}
	
	program_exit:
	
	// @TODO - Renable
	#if 0
	if(IsValid(tasksFile) == true)
		FreeMemory(tasksFile); // @TODO - Replace with File API function once bug is fixed
	#endif
	
	printf("\n");
	
	return 0;
}