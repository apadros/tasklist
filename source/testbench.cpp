#include <stdio.h>
#include <stdlib.h> // Run system commands
#include "apad_array.h"
#include "apad_error.h"
#include "apad_file.h"
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
	// Restore original todos.txt
	PrintLogNewline();
	PrintToLog("Restoring todos.txt...");
	system("copy ..\\..\\data\\todos_testbench_backup.txt ..\\..\\data\\todos.txt /y >> testbench_log.txt");
	// system("del ..\\..\\data\\todos_testbench_backup.txt");
	
	// Restore todos log
	if(FileExists("..\\..\\data\\log.txt") == true) {
		PrintLogNewline();
		PrintToLog("Restoring log.txt...");
		system("copy ..\\..\\data\\log_testbench_backup.txt ..\\..\\data\\log.txt /y >> testbench_log.txt");
	}
	
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
	
	// Open testbench log file
	system("echo Log start > testbench_log.txt");
	PrintLogNewline();
	
	
	// Backup and todos.txt and log.txt, reset todos.txt
	PrintToLog("Backing up todos.txt...");
	system("copy ..\\..\\data\\todos.txt ..\\..\\data\\todos_testbench_backup.txt >> testbench_log.txt");
	PrintLogNewline();
	RegisterExitFunction(ExitFunction); // No matter what happens, the original will be restored and cleanup will be carried out
	system("del ..\\..\\data\\todos.txt /q");
	if(FileExists("..\\..\\data\\log.txt") == true) {
		PrintToLog("Backing up log.txt...");
		system("copy ..\\..\\data\\log.txt ..\\..\\data\\log_testbench_backup.txt /y >> testbench_log.txt");
	}
	
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
	system(Concatenate(3, "echo \"first task\" ", todayString, " 09/05/2026 \"tag 1\"  > ..\\..\\data\\todos.txt")); // Need to add first task manually for now
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
	commands[0] = "todos mod -id 1 -s \"modded string\" -t2 \"new tag\"";
	commands[1] = "todos mod -id 2 -dd 10/10/2030+5 -t \"\"";
	PrintLogNewline();
	CarryOutCommands(commands, 2);
	
	// Compare
	targetFileContents[0] = Concatenate(3, "\"modded string\" ", todayString, " 10/06/2026 \"tag 2\" \"new tag\" ");
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
	
	// List
	PrintLogNewline();
	PrintToLog("Running list command...");
	system("del temp.txt");
	commands[0] = "todos list all";
	PrintLogNewline();
	CarryOutCommands(commands, 1);
	auto daysOffset = GetDaysOffsetFromToday("10/06/2026");
	const char* targetOutput = Concatenate(6, "\r\n"
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
	// Run custom comparison
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
	
	printf("passed\n");
	PrintLogNewline();
	PrintToLog("All tests passed");
	
	program_exit:
	return 0;
}