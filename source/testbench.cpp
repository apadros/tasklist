#include <stdio.h>
#include <stdlib.h> // Run system commands
#include "apad_array.h"
#include "apad_error.h"
#include "apad_file.h"
#include "apad_maths.h"
#include "apad_string.h"
#include "apad_time.h"
#include "apad_win32.h"
#include "helpers.cpp"

file todosFile = {};

void PrintToLog(const char* string) {
	system(Concatenate(3, "echo ", string, " >> testbench_log.txt"));
}

void PrintLogNewline() {
	system("echo: >> testbench_log.txt");
}

void CarryOutCommands(const char** commands, ui8 length) {
	AssertRet(commands != Null && length > 0);
	ForAll(length) {
		const char* command = commands[it];
		AssertRet(command != Null);
		PrintToLog(command);
		system(Concatenate(2, command, " >> temp.txt"));
	}
}

bool LoadTodosAndCompare(const char** targetFileContent, ui8 length) {
	AssertRetType(targetFileContent != Null && length > 0, false);
	
	PrintLogNewline();
	PrintToLog("Comparing...");
	
	todosFile = LoadFile("..\\..\\data\\todos.txt");
	const char* fileLine = (const char*)todosFile.memory;
	ForAll(length) {
		char* newLine = (char*)FindSubstring("\r", fileLine);
		AssertRetType(newLine != Null, false);
		*newLine = '\0';
		
		const char* targetLine = targetFileContent[it];
		Assert(targetLine != Null);
		if(StringsAreEqual(targetLine, fileLine) == false) {
			printf("failed\n");
			PrintLogNewline();
			PrintToLog("Test failed");
			PrintToLog(Concatenate(2, "  Target: ", targetLine));
			PrintToLog(Concatenate(2, "  Actual: ", fileLine));
			FreeFile(todosFile);
			return false;
		}
		
		fileLine = newLine + 2;
	}
	FreeFile(todosFile);
	
	// Add the contents of todos.txt to log file
	PrintLogNewline();
	PrintToLog("todos.txt contents");
	PrintLogNewline();
	system("type ..\\..\\data\\todos.txt >> testbench_log.txt");
	
	return true;
}

void ExitFunction() {
	// Restore originals
	PrintLogNewline();
	PrintToLog("Restoring original files...");
	PrintLogNewline();
	system("del ..\\..\\data\\todo*.txt /q >> testbench_log.txt");
	system("copy ..\\..\\data\\testbench_backups\\*.txt ..\\..\\data\\ /y >> testbench_log.txt");
	system("del ..\\..\\data\\testbench_backups\\* /q >> testbench_log.txt");
	
	PrintLogNewline();
	PrintToLog("Log end");
	system("del temp.txt /q");
	
	if(IsValid(todosFile) == true)
		FreeFile(todosFile);
}

ConsoleAppEntryPoint(args, argsCount) {
	SetExitIfAssertionHit(true);
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(true);
	
	// Create backup folder
	system("if not exist ..\\..\\data\\testbench_backups (mkdir ..\\..\\data\\testbench_backups) >> testbench_log.txt");
	system("del ..\\..\\data\\testbench_backups /q >> testbench_log.txt");
	
	// Open testbench log file
	system("echo Log start > testbench_log.txt");
	PrintLogNewline();
	
	// Backup all necessary files and reset todos.txt it
	PrintToLog("Backing up all files...");
	PrintLogNewline();
	system("copy ..\\..\\data\\todos*.txt ..\\..\\data\\testbench_backups\\ >> testbench_log.txt");
	system("copy ..\\..\\data\\log.txt ..\\..\\data\\testbench_backups\\ >> testbench_log.txt");
	RegisterExitFunction(ExitFunction); // No matter what happens, the originals will be restored and cleanup will be carried out
	system("del ..\\..\\data\\todos.txt /q >> testbench_log.txt");
	
	// Run testbench
	printf("\nRunning testbench...");
	
	const char* todayString = DateToString(GetDate(0));
	
	// Test add commands
	PrintLogNewline();
	PrintToLog("Running add commands...");
	const char* commands[] = { "todos add -s \"first task\" -dd 09/05/2026 -t \"tag 2\"",
														 "todos add -s \"task 2\" -dd 10/06/2026 -t \"tag 2\"",
		                         "todos add -s third -dd today",
		                         "todos add -s \"task number 4\" -t3 tag3"
														 };	
	PrintLogNewline();
	system(Concatenate(3, "echo \"first task\" ", todayString, " 09/05/2026 \"tag 1\" > ..\\..\\data\\todos.txt")); // Need to add first task manually for now
	PrintToLog(commands[0]);
	CarryOutCommands(commands + 1, GetArrayLength(commands) - 1);
	
	// Run comparison	
	const char* targetFileContents[] = { Concatenate(3, "\"first task\" ", todayString, " 09/05/2026 \"tag 1\" "),
																			 Concatenate(3, "\"task 2\" ", todayString, " 10/06/2026 \"tag 2\" "),
																			 Concatenate(5, "\"third\" ", todayString, " ", todayString, " - "),
																			 Concatenate(3, "\"task number 4\" ", todayString, " - \"tag3\" ")
																			 };
	bool comparison = LoadTodosAndCompare(targetFileContents, GetArrayLength(targetFileContents));
	if(comparison == false)
		goto program_exit;
	
	// Check that the todos_[today].txt file was created
	{
		PrintLogNewline();
		PrintToLog("Checking whether todos_[today].txt file was created...");
		
		char* date = DateToString(GetDate(0));
		date[2] = '_';
		date[5] = '_';
		const char* filePath = Concatenate(3, "..\\..\\data\\todos_", date, ".txt");
		if(FileExists(filePath) == false) {
			printf("failed\n");
			PrintLogNewline();
			PrintToLog("Test failed");
			goto program_exit;
		}
	}			
	
	// Test del commands
	PrintLogNewline();
	PrintToLog("Running del commands...");
	commands[0] = "todos del 1";
	commands[1] = "todos del 2";
	PrintLogNewline();
	CarryOutCommands(commands, 2);
	
	// Compare
	targetFileContents[0] = targetFileContents[1];
	targetFileContents[1] = targetFileContents[2];
	targetFileContents[2] = targetFileContents[3];
	targetFileContents[3] = Null;
	targetFileContents[1] = targetFileContents[2];
	targetFileContents[2] = Null;
	comparison = LoadTodosAndCompare(targetFileContents, 2);
	if(comparison == false)
		goto program_exit;
	
	// Test mod commands
	PrintLogNewline();
	PrintToLog("Running mod commands...");
	commands[0] = "todos mod -id 1 -s \"modded string\" -dd - -t2 \"new tag\"";
	commands[1] = "todos mod -id 2 -dd 10/10/2030+5 -t \"\"";
	PrintLogNewline();
	CarryOutCommands(commands, 2);
	
	// Compare
	targetFileContents[0] = Concatenate(3, "\"modded string\" ", todayString, " - \"tag 2\" \"new tag\" ");
	targetFileContents[1] = Concatenate(3, "\"task number 4\" ", todayString, " 15/10/2030 - ");
	comparison = LoadTodosAndCompare(targetFileContents, 2);
	if(comparison == false)
		goto program_exit;
	
	// Test undo command
	PrintLogNewline();
	PrintToLog("Running undo command...");
	commands[0] = "todos undo";
	PrintLogNewline();
	CarryOutCommands(commands, 1);
	
	// Compare
	targetFileContents[1] = Concatenate(3, "\"task number 4\" ", todayString, " - \"tag3\" ");
	comparison = LoadTodosAndCompare(targetFileContents, 2);
	if(comparison == false)
		goto program_exit;
	
	// List vertically and horizontally
	PrintLogNewline();
	PrintToLog("Running list commands...");
	ForAll(3) {
		system("del temp.txt");
		PrintLogNewline();
		
		if(it == 0) {
			commands[0] = "todos mod -id 1 -dd 10/06/2026";
			CarryOutCommands(commands, 1);
			
			system("del temp.txt"); // Test only the listing outputs
			commands[0] = "todos list all";
			CarryOutCommands(commands, 1);
		}
		else if(it ==1) {
			commands[0] = "todos add -s \"task x\" -dd 01/06/2026 -t \"tag 2\"";
			CarryOutCommands(commands, 1);
			
			system("del temp.txt"); // Test only the listing outputs
			commands[0] = "todos list all -printhor -sortbydd -maxwidth 10";
			CarryOutCommands(commands, 1);
		}
		else {
			commands[0] = "todos list -t \"tag 2\" -printhor";
			CarryOutCommands(commands, 1);
		}
		const char* targetOutput = Null;
		if(it == 0) {
			auto daysOffset = GetDaysOffsetFromToday("10/06/2026");
			targetOutput = Concatenate(6, "\r\n"
																		"  ID:         1\r\n"
																		"  String:     modded string\r\n",
										 Concatenate(3, "  Date added: ", todayString, "\r\n"),
										 Concatenate(4, "  Date due:   10/06/2026 (", daysOffset > 0 ? "+" : "-", ToString(daysOffset), ")\r\n"),
																		"  Tags:       tag 2\r\n"
																		"              new tag\r\n"
																		"\r\n"
																		"  ID:         2\r\n"
																		"  String:     task number 4\r\n",
										 Concatenate(3, "  Date added: ", todayString, "\r\n"),
																		"  Date due:   -\r\n"
																		"  Tags:       tag3\r\n"
																		"\r\n");
		}
		else {
			auto daysOffset = GetDaysOffsetFromToday("10/06/2026");
			const char* dateDueString1 = Concatenate(4, "10/06/2026 (", daysOffset > 0 ? "+" : "-", ToString(daysOffset), ")");
			si32 diffDays = GetDaysOffsetFromToday("10/06/2026");
			if(Magnitude(diffDays) <= 99)
				dateDueString1 = Concatenate(2, dateDueString1, " ");
			if(Magnitude(diffDays) < 9)
				dateDueString1 = Concatenate(2, dateDueString1, "  ");
			
			daysOffset = GetDaysOffsetFromToday("01/06/2026");
			const char* dateDueString2 = Concatenate(4, "01/06/2026 (", daysOffset > 0 ? "+" : "-", ToString(daysOffset), ")");
			diffDays = GetDaysOffsetFromToday("10/06/2026");
			if(Magnitude(diffDays) <= 99)
				dateDueString2 = Concatenate(2, dateDueString2, " ");
			if(Magnitude(diffDays) < 9)
				dateDueString2 = Concatenate(2, dateDueString2, "  ");
			
			if(it == 1)
			  targetOutput = Concatenate(11, "\r\n"
			  															 "  ID | Task       | Date Added | Date Due          | Tags\r\n"
			  															 "===========================================================\r\n" 
			  															 "   3 | task x     | ", todayString, " | ", dateDueString2, "  | tag 2\r\n"
			  															 "-----------------------------------------------------------\r\n"
			  															 "   1 | modded str | ", todayString, " | ", dateDueString1, " | tag 2, new tag\r\n"
			  															 "     | ing        |            |                   |\r\n"
			  															 "-----------------------------------------------------------\r\n"
			  															 "   2 | task numbe | ", todayString, " |         -         | tag3\r\n"
			  															 "     | r 4        |            |                   |\r\n"
			  															 "-----------------------------------------------------------\r\n"
			  															 "\r\n");	
			else
				targetOutput = Concatenate(9, "\r\n"
			  															"  ID | Task          | Date Added | Date Due          | Tags\r\n"
			  															"==============================================================\r\n" 
			  															"   1 | modded string | ", todayString, " | ", dateDueString1, " | tag 2, new tag\r\n"
			  															"--------------------------------------------------------------\r\n"
			  															"   3 | task x        | ", todayString, " | ", dateDueString2, "  | tag 2\r\n"
			  															"--------------------------------------------------------------\r\n"
			  															"\r\n");	
		}
		// Format output and run custom comparison
		{
			auto tempFile = LoadFile("temp.txt");
			const char* tempContents = (const char*)tempFile.memory;
			
			if(StringsAreEqual(tempContents, targetOutput) == false) {
				printf("failed\n");
				PrintLogNewline();
				PrintToLog("Test failed");
				
				char* target = Concatenate(2, "Target: ", targetOutput);
				auto  targetLength = GetStringLength(target);
				FromTo(1, targetLength) { // Process to remove excess spaces, newlines and carriage returns
					char* c = target + it;
					if(*c == ' ' || *c == '\r' || *c == '\n') { 
						*c = ' ';
						if(*(c - 1) == ' ') { // Shift back by 1
							ui16 start = it;
							FromTo(start, targetLength) // Shift back the rest
								target[it] = target[it + 1];
							targetLength -= 1;
							it -= 1; // To check the newly moved char
						}
					}
				}
				char* actual = Concatenate(2, "Actual: ", tempContents);
				auto  actualLength = GetStringLength(actual);
				FromTo(1, actualLength) { // Process to remove excess spaces, newlines and carriage returns
					char* c = actual + it;
					if(*c == ' ' || *c == '\r' || *c == '\n') { 
						*c = ' ';
						if(*(c - 1) == ' ') { // Shift back by 1
							ui16 start = it;
							FromTo(start, actualLength) // Shift back the rest
								actual[it] = actual[it + 1];
							actualLength -= 1;
							it -= 1; // To check the newly moved char
						}
					}
				}
				PrintToLog(target);
				PrintToLog(actual);
				FreeFile(tempFile);
				goto program_exit;
			}
			
			FreeFile(tempFile);
		}
	}
	
	printf("passed\n");
	PrintLogNewline();
	PrintToLog("All tests passed");
	
	program_exit:
	return 0;
}