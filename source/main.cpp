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

#include <stdio.h>
#define PrintErrorExit(_string) { \
	printf("\nERROR: %s\n", _string); \
	goto program_exit; \
}

file todosFile;
void ExitFunction() {
	if(IsValid(todosFile) == true)
		FreeFile(todosFile);
}

ConsoleAppEntryPoint(args, argsCount) {
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(true);
	SetExitIfAssertionHit(true);
	
	RegisterExitFunction(ExitFunction); // @TODO - Test this works
	
	#ifdef APAD_DEBUG
		#if 1
		char* debugArgs[] = { args[0], "add", "-s", "task x", "-t", "tag1", "tag2", "tag3", "-dd", "05/07" };
		args = debugArgs;
		argsCount = GetArrayLength(debugArgs);	
		#endif
	#endif
	
	if(argsCount == 1) {
		printf("\nUsage: %s [<command>] [<options>]\n", args[0]);
		printf("\n  Commands\n");
		printf("      Add\n");
		printf("      List\n");
		
		// @TODO - Have a custom message for each command and which options can be used with it
		
		printf("\n  Options\n");
		printf("      %s  [<text string>]                 task text\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
		printf("      %s [dd/mm | dd/mm/yyyy]            date added\n", (const char*)ValidArguments[ValidArgumentsIndex::DateAdded]);
		printf("      %s [dd/mm | dd/mm/yyyy | +ddd[w]]  date due\n", (const char*)ValidArguments[ValidArgumentsIndex::DateDue]);
		printf("      %s  [<tags>...]                     string tags (up to 5)\n", (const char*)ValidArguments[ValidArgumentsIndex::Tags]);
		
		goto program_exit;
	}
	
	// Data to be parsed from arguments
	// Arguments not needed for the required command will be ignored
	const char* command;
	const char* taskString;
	const char* dateAdded;
	const char* dateDue = Null;
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
			PrintErrorExit("Invalid command");
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
				PrintErrorExit("Invalid date added specified");
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::DateDue]) == true) {
			it += 1;
			CheckArgsExit();
			dateDue = args[it];
			
			if(IsDate(dateDue) == false)
				PrintErrorExit("Invalid date due specified");
			
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
					break;
				
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
		else {
			PrintErrorExit("Invalid command option"); // @TODO - Go to specific help message
		}
		
		#if 0
		else if(IsDate(arg) == true || (arg.length == 1 && arg[0] == '.') || (arg.length >= 2 && arg.length <= 4 && arg[0] == '+')) { // Date
      if(arg[0] == '.') {
				if(dateDue.length == 0)
					dateDue = DateToString(GetDate(0));
				else {
					PrintErrorExit("Target date already supplied: %s", (char*)arg);
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
					PrintErrorExit("Invalid day offset (max length allowed is 3)");
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
					PrintErrorExit("Reschedule period already supplied", (char*)arg);
					goto program_exit;
				}						
			}
			else {
				if(dateDue.length == 0)
					dateDue = DateToString(StringToDate(arg)); // Conversion back and forth to set the standard date format dd/mm/yyyy
				else {
					PrintErrorExit("Target date already supplied", arg);
					goto program_exit;
				}
			}
		}
		else if(arg.length == 1) { // Flag
			if(arg[0] == '!' || arg[0] == '?' || arg[0] == '@')
				flag = arg;
			else {
				PrintErrorExit("Invalid flag supplied: %s", (char*)arg);
				goto program_exit;
			}
		}
		else if(StringsAreEqual(arg, "-t") == true) { // Tags
			it += 1;
			
			if(it == argsCount) {
				PrintErrorExit("No tags specified");
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
					PrintErrorExit("Number of tags exceeded, max 5");
					goto program_exit;
				}
				
				it += 1;
			}
			while(it < argsCount);
		}
		else { // Invalid argument
			PrintErrorExit("Invalid argument: %s", (char*)arg);
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
	struct todoListEntry {
		char* task; 		 		 // Must have
		char* dateAdded; 		 // Must have
		char* dateDue; 	 		 // Can be Null
		char* tags[MaxTags]; // Can all be Null
	};
	memory_stack todoList = AllocateStack();
	ui16 				 todosCount = 0;
	{
		if(FileExists(dataPath) == false)
			PrintErrorExit("Couldn't find data/todos.txt");
		
		todosFile = LoadFile(dataPath);
		if(AssertionWasHit() == true)
			PrintErrorExit("Couldn't load data/todos.txt");
		
		// Extract line data
		LineReadLoopHeader(readIndex, todosFile) {
			auto line = ReadLine(todosFile, readIndex);
			Assert(LineIsValid(line));
			Assert(line.count >= 4);
			Assert(line.count <= 3 + MaxTags);
			
			auto* entry = PushStruct(todoListEntry, todoList);
			entry->task = GetLineDataElement(line, 0);
			entry->dateAdded = GetLineDataElement(line, 1);
			entry->dateDue = GetLineDataElement(line, 2);
			FromTo(3, line.count) {
				char* tag = GetLineDataElement(line, it);
				if(it > 3 || tag[0] != '-')
					entry->tags[it - 3] = tag;
			}
			
			FreeLine(line);
		}
	}
	
	// Parse command, output error message if invalid
	if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Add]) == true) {
		dateAdded = DateToString(GetDate(0));
		
		// Create new entry
		auto* entry = (todoListEntry*)Push(sizeof(todoListEntry), todoList);
		todosCount += 1;
		entry->task = (char*)taskString; 
		entry->dateAdded = (char*)dateAdded;
		entry->dateDue = (char*)dateDue;
		Assert(sizeof(tags) == sizeof(entry->tags));
		CopyMemory(tags, sizeof(tags), entry->tags);
		printf("\nTask added\n");
		PrintDetailedTask(Null, taskString, dateAdded, dateDue, tags);
		
		// Save to file
		auto saveString = AllocateStack(Null);
		ForAll(todosCount) {
			auto* entry = (todoListEntry*)todoList.memory + it;	
			Assert(entry->task != Null);
			Assert(entry->dateAdded != Null);
			
			const char* string = Concatenate(7, "\"", entry->task, "\" ", entry->dateAdded, " ", entry->dateDue == Null ? "-" : entry->dateDue, " ");
			if(entry->tags[0] == Null)
				string = Concatenate(2, string, "- ");
			else {
				ForAll(MaxTags) {
					if(entry->tags[it] != Null)
						string = Concatenate(4, string, "\"", entry->tags[it], "\" ");
				}
			}
			string = Concatenate(2, string, "\r\n"); 
			
			Push((void*)string, GetStringLength(string), saveString);
		}
		SaveFile(saveString.memory, saveString.size, dataPath);
		
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true) {
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
		PrintErrorExit("Invalid command supplied.");
		goto program_exit;
	}
	
	program_exit:	
	printf("\n");
	
	return 0;
}