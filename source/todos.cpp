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

#include <stdlib.h> // For qsort and system
int CompareEntriesByDateDue(const void* p1, const void* p2) {
	auto* e1 = *((todoListEntry**)p1);
	auto* e2 = *((todoListEntry**)p2);
	
	if(e1->dateDue == Null)
		return 1;
	else if(e2->dateDue == Null)
		return -1;
	
	auto d1 = StringToDate(e1->dateDue);
	Assert(sizeof(d1.year) == sizeof(ui16));
	Assert(sizeof(d1.month) == sizeof(ui8));
	Assert(sizeof(d1.day) == sizeof(ui8));
	ui32 d1p = d1.year << 16 | d1.month << 8 | d1.day;
	
	auto d2 = StringToDate(e2->dateDue);
	ui32 d2p = d2.year << 16 | d2.month << 8 | d2.day;
	
	if(d1p < d2p)
		return -1;
	else if(d1p == d2p)
		return 0;
	else
		return 1;
}

file 				 todosFile;
memory_stack todoList;
memory_stack logFile;
void ExitFunction() {
	if(IsValid(todosFile) == true)
		FreeFile(todosFile);
	if(IsValid(todoList) == true)
		FreeStack(todoList);
	if(IsValid(logFile) == true)
		FreeFile(logFile);
}

ConsoleAppEntryPoint(args, argsCount) {
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(true);
	SetExitIfAssertionHit(true);

	RegisterExitFunction(ExitFunction);

	#ifdef APAD_DEBUG_COMMANDS
		#if 0
		char* debugArgs[] = { args[0], "list", "all"};
		args = debugArgs;
		argsCount = GetArrayLength(debugArgs);
		#endif
	#endif

	// Initial help message
	if(argsCount == 1) {
		printf("\nUsage: %s [<command>] [<options>]\n", args[0]);
		printf("\n  Commands\n");
		printf("      add\n");
		printf("      list\n");
		printf("      del   deletes entire task\n");
		printf("      mod   modifies task, including deletion of data\n");
		printf("      undo  undoes latest change only (restores todos file with a backup)\n");
		goto program_exit;
	}

	// Data to be parsed from arguments
	// Arguments not needed for the required command will be ignored
	enum date_logic : ui8 { None, LT, LTE, GT, GTE };
	const char* 	   command = Null;
	const char* 	   id = Null;
	const char* 	   taskString = Null;
			  date_logic dateAddedLogic = date_logic::None;
	const char* 	   dateAdded = Null;
				date_logic dateDueLogic = date_logic::None;
	const char* 	   dateDue = Null;
	const char* 	   tags[MaxTags] = { Null };
	const char* 	   specialOption = Null;
	bool 						 printHorizontal = false;
	bool             sortByDateDue = false;

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

	// Open the todos file and generate task list
	#ifdef APAD_DEBUG_DATA_PATH
	const char* dataPath = "../../data/todos.txt";
	#else
	const char* dataPath = "data/todos.txt";
	#endif

	guid guidCounter = 0;
			 todoList = AllocateStack();
	{
		if(FileExists(dataPath) == false) {
			todosFile = CreateFile();
			PrintLogMessage("data/todos.txt wasn't found, a new one will be created with the addition of the first task");
			system("if not exist data ( mkdir data )");
		}
		else {
			todosFile = LoadFile(dataPath);
			if(AssertionWasHit() == true)
				PrintErrorExit("Couldn't load data/todos.txt");
		}

		// Create backup if required
		if(todosFile.size > 0) { // A recently-created file will have size == 0
			char* date = DateToString(GetDate(0));
			date[2] = '_';
			date[5] = '_';
			const char* filePath = AllocateString(dataPath, Null);
			*((char*)(GetFileExtension(filePath) - 1)) = '\0'; // Remove . and extension
			filePath = Concatenate(5, filePath, "_", date, ".", GetFileExtension(dataPath));
			if(FileExists(filePath) == false) {
				SaveFile(todosFile, filePath);
				PrintLogMessage(Concatenate(2, filePath, " backup created"));
			}
		}

		// Go back to check for backups more than 10 days old
		{
			bool deletedOne = false;
			FromToInc(-30, -11) {
				char* date = DateToString(GetDate(it));
				date[2] = '_';
				date[5] = '_';
				const char* filePath = AllocateString(dataPath, Null);
				*((char*)(GetFileExtension(filePath) - 1)) = '\0'; // Remove . and extension
				filePath = Concatenate(5, filePath, "_", date, ".", GetFileExtension(dataPath));
				if(FileExists(filePath) == true) {
					if(deletedOne == false) {
						printf("\n");
						deletedOne = true;
					}
					DeleteFile(filePath);
					PrintLogMessage(Concatenate(3, "Backup file ", filePath, " deleted\n"));
				}
			}
		}

		// Extract line data
		LineReadLoopHeader(readIndex, todosFile) {
			auto line = ParseLine(todosFile, readIndex);
			Assert(LineIsValid(line));
			Assert(line.count >= 4);
			Assert(line.count <= 3 + MaxTags);

			auto* entry = PushStruct(todoListEntry, todoList);
			entry->ID = ++guidCounter;
			entry->task = GetLineDataElement(line, 0);
			entry->dateAdded = GetLineDataElement(line, 1);
			char* dateDue = GetLineDataElement(line, 2);
			if(dateDue[0] != '-')
				entry->dateDue = dateDue;
			FromTo(3, line.count) {
				char* tag = GetLineDataElement(line, it);
				if(it > 3 || tag[0] != '-')
					entry->tags[it - 3] = tag;
			}

			FreeLine(line);
		}
	}

	// Parse arguments
	FromTo(2, argsCount) {
		const char* arg = args[it];

		if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true) { // Special cases
			if(it == 2 && (StringsAreEqual(arg, "all") == true || StringsAreEqual(arg, "alltags") == true)) {
				specialOption = arg;
				continue;
			}	
			else if(StringsAreEqual(arg, "printhor") == true) {
				printHorizontal = true;
				continue;
			}
			else if(StringsAreEqual(arg, "sortbydd") == true) {
				sortByDateDue = true;
				continue;
			}
		}
		if(it == 2 && StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Delete]) == true && IsNumber((char*)arg) == true) { // Skip the -id when deleting a todo
			id = arg;
			break;
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::ID]) == true) {
			it += 1;
			CheckArgsExit();
			id = args[it];

			if(FindEntry(id, todoList) == Null)
				PrintErrorExit("Invalid ID specified");
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::TaskString]) == true) {
			it += 1;
			CheckArgsExit();
			taskString = args[it];
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::DateAdded]) == true) {
			it += 1;
			CheckArgsExit();

			const char* temp = args[it];
			if(temp[0] == '<' || temp[0] == '>') {
				if(temp[0] == '<')
					dateAddedLogic = date_logic::LT;
				else
					dateAddedLogic = date_logic::GT;
				temp += 1;
			}
			if(temp[0] == '=') {
				if(temp == args[it]) // There wasn't a < or > before the =
					PrintErrorExit("Invalid date added specified");

				if(dateAddedLogic == date_logic::LT)
					dateAddedLogic = date_logic::LTE;
				else
					dateAddedLogic = date_logic::GTE;
				temp += 1;
			}

			if(IsDateAndValid(temp) == true)
				dateAdded = DateToString(StringToDate(temp)); // Do this to take into account any modifiers to the date (e.g. logic or offsets) and convert to long format
			else
				PrintErrorExit("Invalid date added specified");
		}
		else if(StringsAreEqual(arg, ValidArguments[ValidArgumentsIndex::DateDue]) == true) {
			it += 1;
			CheckArgsExit();

			char* temp = AllocateString(args[it], Null);
			if(DateDueIsUnspecified(temp) == true)
				dateDue = temp;
			else {
				if(temp[0] == '<' || temp[0] == '>') {
					if(temp[0] == '<')
						dateDueLogic = date_logic::LT;
					else
						dateDueLogic = date_logic::GT;
					temp += 1;
				}
				if(temp[0] == '=') {
					if(temp == args[it]) // There wasn't a < or > before the =
						PrintErrorExit("Invalid date due specified");
	
					if(dateDueLogic == date_logic::LT)
						dateDueLogic = date_logic::LTE;
					else
						dateDueLogic = date_logic::GTE;
					temp += 1;
				}
				
				bool  workDays = temp[GetStringLength(temp) - 1] == 'w';
				char* sign = (char*)FindSubstring("+", temp);
				if(sign == Null)
					sign = (char*)FindSubstring("-", temp);
				
				if(workDays == true)
					temp[GetStringLength(temp) - 1] = '\0';
				if(IsDateAndValid(temp) == true) {
					if(sign != Null && workDays == true) {
						ui16 offset = StringToInt(sign + 1, Null);
						sign[1] = '\0'; // Cut off the offset
						FromToInc(1, offset) { // Work out new offset
							const char* tempString = Concatenate(2, temp, ToString(it));
							auto 				date = StringToDate(tempString);
							if(date.dayOfTheWeek >= 6)
								offset += 1; // Increate the search range
							sign[1] = '\0';
						}
	
						temp = Concatenate(2, temp, ToString(offset)); // Update the offset
					}
	
					dateDue = DateToString(StringToDate(temp)); // Do this to take into account any modifiers to the date (e.g. logic or offsets) and convert to long format
				}
				else
					PrintErrorExit("Invalid date due specified");
			}
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
	}

	// Check command arguments and possibly display help message
	if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Add]) == true && taskString == Null) {
		printf("\nUsage: %s %s -s [task string] [<options>]\n", args[0], command);
		DisplayCommandOptions(false, false, false, true, true);
		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true && argsCount < 4 && specialOption == Null) {
		printf("\nUsage: %s %s [<options>] [printhor] [sortbydd]\n", args[0], command);
		DisplayCommandOptions(true, true, true, true, true);
		printf("    all                                                      list all todos\n");
		printf("    alltags                                                  list all existing tags\n");
		printf("  printhor                                                   print the results horizontally\n");
		printf("  sortbydd                                                   sort by ascending date due\n");
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

	// Update the log file with command and arguments, plus time stamp and trim out any entries older that 10 days
	const char* logFilePath = Null;
	if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == false) {
		logFilePath = AllocateString(dataPath, Null);
		char* fileNameStart = (char*)GetFileNameAndExtension(logFilePath);
		*(fileNameStart)= '\0';
		const char* extension = GetFileExtension(dataPath);
		logFilePath = Concatenate(3, logFilePath, "log.", extension);
		
		// Extract the ocntents of previous log entries and store in current log
		logFile = AllocateStack();
		if(FileExists(logFilePath) == true) {
			file file = LoadFile(logFilePath);
			Push(file.memory, file.size, logFile);
			FreeFile(file);
		}
		
		// Trim out unneeded entries
		{
			const char* limit = Concatenate(3, "# ", DateToString(GetDate(-10)), " #");
			const char* toKeep = FindSubstring(limit, (const char*)logFile.memory);
			if(toKeep != Null && toKeep != logFile.memory) {
				auto length = logFile.size - ((ui8*)toKeep - (ui8*)logFile.memory);
				CopyMemory((void*)toKeep, length, logFile.memory);
				logFile.size = length;
			}
		}

		// Search through file for today's date header. If not found, add it
		char* lastChar = Null;
		char  lastCharCopy = Null;
		if(logFile.size > 0) {
			lastChar = (char*)logFile.memory + logFile.size - 1;
			lastCharCopy = *lastChar;
			*lastChar = '\0';
		}
		char* todayStringHeader = Concatenate(3, "# ", DateToString(GetDate(0)), " #");
		if(FindSubstring(todayStringHeader, (const char*)logFile.memory) == Null) {
			PushString("\n", false, logFile);
			PushString(todayStringHeader, false, logFile);
			PushString("\n", false, logFile);
		}
		if(lastChar != Null)
			*lastChar = lastCharCopy;
		
		// Add time
		const char* string = Concatenate(2, GetTimeNow(), " ");

		// Push arguments
		FromTo(0, argsCount)
			string = Concatenate(3, string, args[it], " ");

		UpdateLogFile(string, logFile, logFilePath);
	}

	// Parse command, output error message if invalid
	if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Add]) == true) {
		SaveTodosFileBackup(todoList, dataPath);

		dateAdded = DateToString(GetDate(0));

		// Create new entry
		auto* entry = (todoListEntry*)Push(sizeof(todoListEntry), todoList);
		entry->ID = ++guidCounter;
		entry->task = (char*)taskString;
		entry->dateAdded = (char*)dateAdded;
		if(dateDue != Null && DateDueIsUnspecified(dateDue) == true)
			entry->dateDue = Null;
		else
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
		PrintLogMessage("Task added");
		PrintTaskVertical(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);

		SaveTodosFile(todoList, dataPath);

		goto program_exit;
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::List]) == true) {
		if(specialOption != Null && StringsAreEqual(specialOption, "alltags") == true) { // Print all tags
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
			auto entriesToPrint = AllocateStack();

			guid ID = 0;
			if(id != Null)
				ID = StringToInt(id, Null);

			TodoEntriesLoop(todoList) {
				auto* entry = GetTodosEntry(todoList, it);

				if(specialOption != Null && StringsAreEqual(specialOption, "all") == true) { // Print all
					Push(&entry, sizeof(todoListEntry*), entriesToPrint);
					continue;
				}

				if(id != Null && ID == entry->ID) {
					Push(&entry, sizeof(todoListEntry*), entriesToPrint);
					continue;
				}

				if(taskString != Null) {
					ConvertStringToLowerCase(taskString);

					auto entryTaskString = AllocateString(entry->task, Null);
					ConvertStringToLowerCase(entryTaskString);

					if(FindSubstring(taskString, entryTaskString) != Null) {
						Push(&entry, sizeof(todoListEntry*), entriesToPrint);
						continue;
					}
				}

				if(dateAdded != Null) {
					auto targetDate = StringToDate(dateAdded);
					Assert(sizeof(targetDate.day) == sizeof(ui8));
					Assert(sizeof(targetDate.month) == sizeof(ui8));
					Assert(sizeof(targetDate.year) == sizeof(ui16));
					ui32 targetDateTogether = targetDate.year << 16 | targetDate.month << 8 | targetDate.day;

					auto entryDate = StringToDate(entry->dateAdded);
					ui32 entryDateTogether = entryDate.year << 16 | entryDate.month << 8 | entryDate.day;

					if(dateAddedLogic == date_logic::None && entryDateTogether == targetDateTogether ||
						 dateAddedLogic == date_logic::LT && entryDateTogether < targetDateTogether ||
						 dateAddedLogic == date_logic::LTE && entryDateTogether <= targetDateTogether ||
						 dateAddedLogic == date_logic::GT && entryDateTogether > targetDateTogether ||
						 dateAddedLogic == date_logic::GTE && entryDateTogether >= targetDateTogether)
					{
						Push(&entry, sizeof(todoListEntry*), entriesToPrint);
						continue;
					}
				}
				
				if(dateDue != Null && DateDueIsUnspecified(dateDue) == true) { // If dateDue == '-' and entry has no dateDue, add to list 
					if(entry->dateDue == Null) {
						Push(&entry, sizeof(todoListEntry*), entriesToPrint);
						continue;
					}
				}
				else if(dateDue != Null && entry->dateDue != Null) {
					auto targetDate = StringToDate(dateDue);
					Assert(sizeof(targetDate.day) == sizeof(ui8));
					Assert(sizeof(targetDate.month) == sizeof(ui8));
					Assert(sizeof(targetDate.year) == sizeof(ui16));
					ui32 targetDateTogether = targetDate.year << 16 | targetDate.month << 8 | targetDate.day;

					auto entryDate = StringToDate(entry->dateDue);
					ui32 entryDateTogether = entryDate.year << 16 | entryDate.month << 8 | entryDate.day;

					if(dateDueLogic == date_logic::None && entryDateTogether == targetDateTogether ||
						 dateDueLogic == date_logic::LT && entryDateTogether < targetDateTogether ||
						 dateDueLogic == date_logic::LTE && entryDateTogether <= targetDateTogether ||
						 dateDueLogic == date_logic::GT && entryDateTogether > targetDateTogether ||
						 dateDueLogic == date_logic::GTE && entryDateTogether >= targetDateTogether)
					{
						Push(&entry, sizeof(todoListEntry*), entriesToPrint);
						continue;
					}
				}

				ForAll(MaxTags) {
					const char* tag = tags[it];
					if(TagIsValid(tag) == true) {
						ForAll(MaxTags) {
							if(TagIsValid(entry->tags[it]) == true && StringsAreEqual(tag, entry->tags[it]) == true) {
								Push(&entry, sizeof(todoListEntry*), entriesToPrint);
								continue;
							}
						}
					}
				}
			}

			if(entriesToPrint.size != 0) {
				ui8 count = entriesToPrint.size / sizeof(todoListEntry*);

				// Determine length of tasks string column
				ui16 taskColumnLength = 4; // Size of "Task" in header without spaces
				ForAll(count) {
					auto* entry = ((todoListEntry**)entriesToPrint.memory)[it];
					auto length = GetStringLength(entry->task);
					if(length > taskColumnLength)
						taskColumnLength = length;
				}
				if(taskColumnLength > MaxTaskPrintLength)
					taskColumnLength = MaxTaskPrintLength;

				// Print header for horizontal print
				if(printHorizontal == true) {
					printf("\n  ID | Task ");
					if(taskColumnLength > 4) {
						ForAll(taskColumnLength - 4)
							printf(" ");
					}
					printf("| Date Added | Date Due          | Tags\n");

					// Print horizontal separator
					printf("============");
					ForAll(taskColumnLength - 4)
						printf("=");
					printf("=========================================\n");
				}
				
				if(sortByDateDue == true) {
					// void qsort (void* base, size_t num, size_t size,            int (*compar)(const void*,const void*));
					qsort(entriesToPrint.memory, count, sizeof(todoListEntry*), CompareEntriesByDateDue);
				}

				ForAll(count) {
					auto* entry = ((todoListEntry**)entriesToPrint.memory)[it];
					if(printHorizontal == true) { // Print task horizontally
						// ID
						Assert(entry->ID <= 999);
						if(entry->ID <= 9)
							printf("   %i ", entry->ID);
						else if(entry->ID <= 99)
							printf("  %i ", entry->ID);
						else
							printf(" %i ", entry->ID);
						printf("|");

						// Print task string
						char* remainingTaskToPrint = Null;
						if(GetStringLength(entry->task) <= MaxTaskPrintLength) { // Just print the string and pad the end
							printf(" %s ", entry->task);
							auto length = GetStringLength(entry->task);
							auto diff = Magnitude(length - taskColumnLength);
							ForAll(diff)
								printf(" ");
						}
						else { // Print up to MaxTaskPrintLength, then truncate
							remainingTaskToPrint = AllocateString(entry->task, Null);
							char c = remainingTaskToPrint[MaxTaskPrintLength];
							remainingTaskToPrint[MaxTaskPrintLength] = '\0';
							printf(" %s ", remainingTaskToPrint);
							remainingTaskToPrint[MaxTaskPrintLength] = c;
							remainingTaskToPrint += MaxTaskPrintLength;
						}

						printf("| %s |", entry->dateAdded);

						// dateDue
						{
							const char* empty = "         -        ";
							if(entry->dateDue == Null)
								printf(empty);
							else {
								const char* date = Concatenate(3, " ", entry->dateDue, " (");

								// Add day offset
								si32 diffDays = GetDaysOffsetFromToday(entry->dateDue);
								if(diffDays > 0)
									date = Concatenate(3, date, "+", ToString(diffDays));
								else
									date = Concatenate(2, date, ToString(diffDays));
								date = Concatenate(2, date, ")");
								printf("%s", date);

								// Pad the remaining space
								auto length = GetStringLength(date);
								auto maxLength = GetStringLength(empty);
								ForAll(maxLength - length)
									printf(" ");
							}
							printf(" |");
						}

						// tags
						if(AnyTagsPresent(entry->tags) == true) {
							ForAll(MaxTags) {
								if(TagIsValid(entry->tags[it]) == true)
									printf(it == 0 ? " %s" : ", %s", entry->tags[it]);
							}
						}
						else
							printf(" - ");
						printf("\n");
						
						// Keep printing the entry task string if any remains
						while (remainingTaskToPrint != Null) {
							printf("     "); // ID column
							
							// Task string
							if(GetStringLength(remainingTaskToPrint) <= MaxTaskPrintLength) { // Just print the string and pad the end
								printf("| %s ", remainingTaskToPrint);
								auto length = GetStringLength(remainingTaskToPrint);
								auto diff = Magnitude(length - taskColumnLength);
								ForAll(diff)
									printf(" ");
								remainingTaskToPrint = Null;
							}
							else { // Print up to MaxTaskPrintLength, then truncate again
								char c = remainingTaskToPrint[MaxTaskPrintLength];
								remainingTaskToPrint[MaxTaskPrintLength] = '\0';
								printf("| %s ", remainingTaskToPrint);
								remainingTaskToPrint[MaxTaskPrintLength] = c;
								remainingTaskToPrint += MaxTaskPrintLength;
							}
							
							// Date added, date due and tags
							printf("|            |                   |\n");
						}

						// Print horizontal separator
						printf("------------");
						ForAll(taskColumnLength - 4)
							printf("-");
						printf("-----------------------------------------\n");
					}
					else
						PrintTaskVertical(entry->ID, entry->task, entry->dateAdded, entry->dateDue, (char**)entry->tags);
				}
			}
			FreeStack(entriesToPrint);
		}
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Modify]) == true) {
		SaveTodosFileBackup(todoList, dataPath);

		Assert(id != Null);
		guid 					 ID = StringToInt(id, Null);
		todoListEntry* moddedEntry = Null;
		char* 				 previousString = Null;
		si8 					 modsCount = 0;
		TodoEntriesLoop(todoList) {
			auto* entry = GetTodosEntry(todoList, it);
			if(entry->ID == ID) {
				moddedEntry = entry;

				if(taskString != Null) {
					previousString = AllocateString(entry->task, Null);
					entry->task = (char*)taskString;
					modsCount += 1;
				}
				
				if(DateDueIsUnspecified(dateDue) == true) {
					entry->dateDue = Null;
					modsCount += 1;
				}
				else if(dateDue != Null) {
					entry->dateDue = (char*)dateDue;
					modsCount += 1;
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
					modsCount += 1;
				}

				break;
			}
		}

		if(modsCount > 0) {
			Assert(moddedEntry != Null);

			const char* outputString = Concatenate(3, "Task \"", taskString == Null ? moddedEntry->task : previousString, "\" modified, updated ");

			if(taskString != Null) {
				outputString = Concatenate(2, outputString, "task text");
				modsCount -= 1;
				if(modsCount > 0)
					outputString = Concatenate(2, outputString, " & ");
			}

			if(dateDue != Null) {
				outputString = Concatenate(2, outputString, "date due");
				modsCount -= 1;
				if(modsCount > 0)
					outputString = Concatenate(2, outputString, " & ");
			}

			if(AnyTagsPresent((char**)tags) == true)
				outputString = Concatenate(2, outputString, "tags");

			PrintLogMessage(outputString);

			PrintTaskVertical(moddedEntry->ID, moddedEntry->task, moddedEntry->dateAdded, moddedEntry->dateDue, (char**)moddedEntry->tags);
			SaveTodosFile(todoList, dataPath);
			UpdateLogFile(Concatenate(2, "- ", outputString), logFile, logFilePath);
		}
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Delete]) == true) {
		SaveTodosFileBackup(todoList, dataPath);

		Assert(id != Null);
		auto* entry = FindEntry(id, todoList);
		if(entry != Null) {
			char* taskString = AllocateString(entry->task, Null);

			ClearMemory(entry, sizeof(todoListEntry));
			void* dataStart = entry + 1;
			void* dataEnd = (ui8*)todoList.memory + todoList.size;
			if(dataStart != dataEnd) { // Would be the case for the very last entry
			  CopyMemory(dataStart, (ui32)((ui8*)dataEnd - (ui8*)dataStart), (void*)entry);
				ClearMemory((void*)((ui8*)dataEnd - sizeof(todoListEntry)), sizeof(todoListEntry));
			}
			todoList.size -= sizeof(todoListEntry);

			const char* outputString = Concatenate(3, "Todo \"", taskString, "\" deleted");
			PrintLogMessage(outputString);
			SaveTodosFile(todoList, dataPath);

			UpdateLogFile(Concatenate(2, "- ", outputString), logFile, logFilePath);
		}
	}
	else if(StringsAreEqual(command, ValidCommands[ValidCommandsIndex::Undo]) == true) {
		char* backupPath = GetBackupTodosFilePath(dataPath);
		Assert(FileExists(backupPath) == true);
		auto backupFile = LoadFile(backupPath);
		Assert(IsValid(backupFile) == true);
		SaveFile(backupFile.memory, backupFile.size, dataPath);
		PrintLogMessage(Concatenate(2, "Todos file replaced with backup ", GetBackupTodosFilePath(dataPath)));
		FreeFile(backupFile);
	}
	else
		PrintErrorExit("Invalid command supplied.");

	program_exit:
	printf("\n");

	return 0;
}