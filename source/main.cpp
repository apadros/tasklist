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

#include "helpers.h"

const char* ValidCommands[] = 	{ "add", "list", "del", "mod", "undo", "redo" };
BeginEnum(ValidCommandsIndex) { Add, List, Delete, Modify, Undo, Redo, Length } EndEnum(ValidCommandsIndex);

const char* ValidArguments[] =   { "-id", "-s", "-da", "-dd", "-t", "-t1", "-t2", "-t3", "-t4", "-t5" };
BeginEnum(ValidArgumentsIndex) { ID, TaskString, DateAdded, DateDue, TagsGeneric, Tag1, Tag2, Tag3, Tag4, Tag5, Length } EndEnum(ValidArgumentsIndex);

#include "helpers.cpp"

file todosFile;
void ExitFunction() {
	if(IsValid(todosFile) == true)
		FreeFile(todosFile);
}

ConsoleAppEntryPoint(args, argsCount) {
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(true);
	SetExitIfAssertionHit(true);
	
	RegisterExitFunction(ExitFunction);
	
	#ifdef APAD_DEBUG
		#if 0
		char* debugArgs[] = { args[0], "add", "-s", "task99", "-t", "tag 1", "tag0", "tag 1", "tag", "tag0" };
		args = debugArgs;
		argsCount = GetArrayLength(debugArgs);	
		#endif
	#endif
	
	// Initial help message
	if(argsCount == 1) {
		printf("\nUsage: %s [<command>] [<options>]\n", args[0]);
		printf("\n  Commands\n");
		printf("      Add\n");
		printf("      List\n");
		goto program_exit;
	}
	
	// Data to be parsed from arguments
	// Arguments not needed for the required command will be ignored
	const char* command = Null;
	const char* id = Null;
	const char* taskString = Null;
	const char* dateAdded = Null;
	const char* dateDue = Null;
	const char* tags[MaxTags] = { Null };
	const char* specialCommand = Null;

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
		
		if(it == 2 && StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true && (StringsAreEqual(arg, "all") == true || StringsAreEqual(arg, "alltags") == true)) {
			specialCommand = arg;
			break;
		}
		else if(it == 2 && StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Delete]) == true && IsNumber((char*)arg) == true) { // Skip the -id when deleting a todo
			id = arg;
			break;
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::ID]) == true) {
			it += 1;
			CheckArgsExit();
			id = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::TaskString]) == true) {
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
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::TagsGeneric]) == true) {
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
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::Tag1]) == true) {
			it += 1;
			CheckArgsExit();
			tags[0] = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::Tag2]) == true) {
			it += 1;
			CheckArgsExit();
			tags[1] = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::Tag3]) == true) {
			it += 1;
			CheckArgsExit();
			tags[2] = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::Tag4]) == true) {
			it += 1;
			CheckArgsExit();
			tags[3] = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::Tag5]) == true) {
			it += 1;
			CheckArgsExit();
			tags[4] = args[it];
		}
		else
			PrintErrorExit("Invalid argument supplied");
		
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
	
	// Check command arguments and possibly display help message
	if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Add]) == true && taskString == Null) {
		printf("\nUsage: %s %s -s [task string] [<options>]\n", args[0], command);
		DisplayCommandOptions(false, false, false, true, true);
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true && argsCount < 4 && specialCommand == Null) {
		printf("\nUsage: %s %s [<options>]\n", args[0], command);
		DisplayCommandOptions(true, true, true, true, true);
		printf("    all                                          list all todos\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
		printf("    alltags                                      list all existing tags\n", (const char*)ValidArguments[ValidArgumentsIndex::TaskString]);
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Modify]) == true && (id == Null || argsCount < 6)) {
		printf("\nUsage: %s %s -id [id] [<options>]\n", args[0], command);
		DisplayCommandOptions(false, true, false, true, true);
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Delete]) == true && id == Null) {
		printf("\nUsage: %s %s [id]\n", args[0], command);
		goto program_exit;
	}
	
	#ifdef APAD_DEBUG
	const char* dataPath = "..\\..\\data\\todos.txt";
	#else
	const char* dataPath = "data/todos.txt";
	#endif
	
	// Open the todos file and generate task list
	guid guidCounter = 0;
	
	memory_stack todoList = AllocateStack();
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
			entry->ID = ++guidCounter;
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
		entry->ID = ++guidCounter;
		entry->task = (char*)taskString; 
		entry->dateAdded = (char*)dateAdded;
		entry->dateDue = (char*)dateDue;
		
		Assert(sizeof(tags) == sizeof(entry->tags));
		FromTo(0, MaxTags - 1) {
			auto current = it;
			if(TagIsValid(tags[current]) == true) {
				FromTo(current + 1, MaxTags) {
					if(TagIsValid(tags[it]) && StringsAreEqual(tags[current], tags[it]) == true)
						tags[it] = Null;
				}
			}
		}
		CopyMemory(tags, sizeof(tags), entry->tags);
		printf("\nTask added\n");
		PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
		
		SaveChangesToTodosFile(todoList, dataPath);
		
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true) {
		if(specialCommand != Null && StringsAreEqual(specialCommand, "all") == true) { // Print all
			TodoEntriesLoop(todoList) {
				auto* entry = GetTodosEntry(todoList, it);
				PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
			}
		}
		else if(specialCommand != Null && StringsAreEqual(specialCommand, "alltags") == true) { // Print all tags
			auto printedTags = AllocateStack();
			TodoEntriesLoop(todoList) {
				auto* entry = GetTodosEntry(todoList, it);
				ForAll(MaxTags) {
					auto* tag = entry->tags[it];
					if(TagIsValid(tag) == true) {
						// Check if already printed
						bool printed = false;
						ForAll(printedTags.size / sizeof(char*)) {
							char* t = ((char**)printedTags.memory)[it];
							printed = StringsAreEqual(t, tag);
							if(printed == true)
								break;
						}
						
						if(printed == false) {
							if(printedTags.size < sizeof(char*)) // Print first tag
								printf("  Tags: %s\n", (char*)tag);
							else
								printf("        %s\n", (char*)tag);
							Push(&tag, sizeof(char*), printedTags);
						}
					}
				}
			}
			FreeStack(printedTags);
		}
		else {		
			guid ID = 0;
			if(id != Null)
				ID = StringToInt(id, Null);
			
			TodoEntriesLoop(todoList) {
				auto* entry = GetTodosEntry(todoList, it);
				bool printed = false;
				
				if(id != Null && ID == entry->ID) {
					PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
					printed = true;
				}
				
				if(printed == false && taskString != Null) {
					ConvertStringToLowerCase(taskString);
					
					auto entryTaskString = AllocateString(entry->task, Null);
					ConvertStringToLowerCase(entryTaskString);
					
					if(FindSubstring(taskString, entryTaskString) != Null) {
						PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
						printed = true;
					}
				}
				
				if(printed == false && dateAdded != Null) {
					if(FindSubstring(dateAdded, entry->dateAdded) != Null) {
						PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
						printed = true;
					}
				}
				
				if(printed == false && dateDue != Null) {
					if(FindSubstring(dateDue, entry->dateDue) != Null) {
						PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
						printed = true;
					}
				}
				
				if(printed == false) { 
					ForAll(MaxTags) {
						const char* tag = tags[it];
						if(TagIsValid(tag) == true) {
							ForAll(MaxTags) {
								if(TagIsValid(entry->tags[it]) == true && StringsAreEqual(tag, entry->tags[it]) == true)
									PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
							}
						}
					}
				}
			}
		}
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Modify]) == true) {
		// @TODO
		// When making any changes to an entry, display the updated portion of info 
		// before and after, then display the updated entry will all info
		
		Assert(id != Null);
		guid ID = StringToInt(id, Null);
		bool modded = false;
		TodoEntriesLoop(todoList) {
			auto* entry = GetTodosEntry(todoList, it);
			if(entry->ID == ID) {
				if(taskString != Null) {
					entry->task = (char*)taskString;
					printf("\nUpdated task text\n");
					PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
					modded = true;
				}
				
				if(dateDue != Null) {
					entry->dateDue = (char*)dateDue;
					printf("\nUpdated date due\n");
					PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
					modded = true;
				}
				
				if(AnyTagsPresent((char**)tags) == true) {
					ForAll(MaxTags) {
						if(TagIsValid(tags[it]) == true) {
							if(StringsAreEqual(tags[it], "") == true)
								entry->tags[it] = Null;
							else
								entry->tags[it] = (char*)tags[it];
						}
					}
					printf("\nUpdated tags\n");
					PrintDetailedTask(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
					modded = true;
				}
				
				break;
			}
		}
		
		if(modded == true)
			SaveChangesToTodosFile(todoList, dataPath);		
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Delete]) == true) {
		Assert(id != Null);
		guid ID = StringToInt(id, Null);
		TodoEntriesLoop(todoList) {
			auto* entry = GetTodosEntry(todoList, it);
			if(entry->ID == ID) {
				ClearMemory(entry, sizeof(todoListEntry));
				void* dataStart = entry + 1;
				void* dataEnd = (ui8*)todoList.memory + todoList.size;
				if(dataStart != dataEnd) { // Would be the case for the very last entry
					CopyMemory(dataStart, (ui32)((ui8*)dataEnd - (ui8*)dataStart), (void*)entry);
					ClearMemory((void*)((ui8*)dataEnd - sizeof(todoListEntry)), sizeof(todoListEntry));
				}
				todoList.size -= sizeof(todoListEntry);
				
				printf("\nTodo no. %u deleted\n", ID);
				SaveChangesToTodosFile(todoList, dataPath);
				
				break;
			}
		}		
	}
	else
		PrintErrorExit("Invalid command supplied.");
	
	program_exit:	
	printf("\n");
	
	return 0;
}