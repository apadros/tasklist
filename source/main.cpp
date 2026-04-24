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

const char* DateFormatShort  = "dd/mm";
const char* DateFormatMedium = "dd/mm/yy";
const char* DateFormatLong   = "dd/mm/yyyy";
const ui8 	MaxTags = 5;

#include "helpers.cpp"

const char* ValidCommands[] = 	{ "add", "list", "del", "mod", "undo", "redo" };
BeginEnum(ValidCommandsIndex) { Add, List, Delete, Modify, Undo, Redo, Length } EndEnum(ValidCommandsIndex);

const char* ValidArguments[] =   { "-s", "-da", "-dd", "-t" };
BeginEnum(ValidArgumentsIndex) { TaskString, DateAdded, DateDue, Tags, Length } EndEnum(ValidArgumentsIndex);

struct todoListEntry {
	const char* task;
	const char* dateAdded;
	const char* dateDue;
				char  flag;
	const char* tags[MaxTags];
};

#include <stdio.h>
#define PrintErrorExit(_string) { \
	printf("ERROR: %s\n", _string); \
	goto program_exit; \
}

void ExitFunction() {
	// @TODO
}

ConsoleAppEntryPoint(args, argsCount) {
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(false);
	SetExitIfAssertionHit(false);
	
	#ifdef APAD_DEBUG
		#if 1
		char* debugArgs[] = { args[0], "add", "sample task" };
		args = debugArgs;
		argsCount = GetArrayLength(debugArgs);	
		#endif
	#endif
	
	if(argsCount == 1) {
		printf("Usage: %s [add] [list] [del | delete] [mod | modify] [undo] [redo]\n\n", args[0]);
		printf("Options\n");
		printf("	%s  [<text string>]                 task text\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
		printf("	%s [dd/mm | dd/mm/yyyy]            date added\n", (const char*)ValidArguments[ValidArgumentsIndex::DateAdded]);
		printf("	%s [dd/mm | dd/mm/yyyy | +ddd[w]]  date due\n", (const char*)ValidArguments[ValidArgumentsIndex::DateDue]);
		printf("	%s  [<tags>...]                     string tags (up to 5)\n", (const char*)ValidArguments[ValidArgumentsIndex::Tags]);
		// @TODO - Add what options can be specified with every command, figure out a simple way
		goto program_exit;
	}
	
	// Data to be parsed from arguments
	// Arguments not needed for the required command will be ignored
	const char* command;
	const char* taskString;
	const char* dateAdded;
	const char* dateDue;
	const char* tags[MaxTags] = { Null };

	// Parse and check command
	command = args[1];
	{
		bool found = false;
		ForAll(ValidCommandsIndex::Length) {
			if(StringsAreEqual(command, ValidCommands[it]) == true) {
				found = true;
				break;
			}
		}
		
		if(found == false)
			PrintErrorExit("Invalid command\n");
	}
	
	#define CheckArgsExit() { if(it >= argsCount) \
															PrintErrorExit("Not enough arguments supplied."); }
			
	// Parse arguments
	FromTo(2, argsCount) {
		const char* arg = args[it];
		
		if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::TaskString]) == true) {
			it += 1;
			CheckArgsExit();
			taskString = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::DateAdded]) == true) {
			it += 1;
			CheckArgsExit();
			dateAdded = args[it];
			if(IsDate(dateAdded) == false)
				PrintErrorExit("Invalid date added specified\n");
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::DateDue]) == true) {
			it += 1;
			CheckArgsExit();
			dateDue = args[it];
			
			if(IsDate(dateAdded) == false)
				PrintErrorExit("Invalid date due specified\n");
			
			// Convert to long date format
			dateDue = DateToString(StringToDate(dateDue)); // @TODO - Simplify this?
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::Tags]) == true) {
			// Scan arguments and store up to MaxTags or end of arguments so long as none are valid options
			ui8 count = 0;
			while(count < MaxTags) {
				it += 1;
				
				if(count == 0) {
					CheckArgsExit();
				}
				else if(it >= argsCount) // Reached the end of the arguments, not necessarily an error
					goto program_exit;
				
				const char* s = args[it];
				bool arg = FindSubstring("-", s);
				if(arg == false)
					tags[count++] = s;
				else {
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
				const char* daysString = arg.chars + 1;
				
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
		else if(StringsAreEqual(arg, "-t") == true) { // Tags
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
	const char* dataPath = "..\\..\\data\\todos.txt";
	#else
	const char* dataPath = "data/todos.txt";
	#endif
	
	// Open the todos file and generate task list
	file todosFile = {};
	{
		if(FileExists(dataPath) == false)
			PrintErrorExit("Couldn't find data/todos.txt\n");
		
		todosFile = LoadFile(dataPath);
		if(AssertionWasHit() == true)
			PrintErrorExit("Couldn't load data/todos.txt");
		
		#if 0
		
		// Parse tasks - see data/format.txt
		
		// Extract data
		auto  todoList = AllocateStack(128);
		auto  line = AllocateStack(128);
		char* data = Null;
		bool  readingTaskString = false;
		for(ui32 it = 0; it < todosFile.size; it += 1) {
			char* c = (char*)todosFile.memory + it;
			
			if(*c == '"') {
				if(readingTaskString == false)
					data = c + 1;
				else
					*c = ' ';
				
				Toggle(readingTaskString);
			}
			
			if(*c == '\n') { // Add task to list
				#if 0
				struct todoListEntry {
					const char* task;
					const char* dateAdded;
					const char* dateDue;
								ui8   reschedulePeriod;
								char  flag;
					const char* tags[MaxTags];
				};
				#endif
				
				Assert(line.size % sizeof(char*) == 0);
				char** lineArray = (char**)line.memory;
				auto*  task = (todoListEntry*)Push(sizeof(todoListEntry), todoList);
				ClearMemory(task, sizeof(todoListEntry));
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
	if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Add]) == true) {
		dateAdded = DateToString(GetDate(0));
				
		const char* string = Concatenate(7, "\"", taskString, "\" ", dateAdded, " ", dateDue, " ");
		if(tags[0] == Null)
			string = Concatenate(2, string, "-");
		else {
			ForAll(MaxTags) {
				if(tags[it] != Null)
					string = Concatenate(4, string, "\"", tags[it], "\" ");
			}
		}
		string = Concatenate(2, string, "\r\n"); 
		
		SaveFile((void*)string, GetStringLength(string), dataPath);
		
		printf("\nTask added\n");
		PrintDetailedTask(Null, taskString, dateAdded, dateDue, tags);
		
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true) {
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
			if(StringsAreEqual(arg, "-d") == true)
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
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Modify]) == true) { // Modify - @TODO
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
	if(IsValid(todosFile) == true)
		FreeFile(todosFile);
	
	printf("\n");
	
	return 0;
}