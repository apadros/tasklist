#include "apad_array.h"
#include "apad_error.h"
#include "apad_file.h"
#include "apad_string.h"
#include "apad_time.h"
#include "apad_win32.h"
#include <stdio.h>
#include <stdlib.h> // Run system commands

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
	PrintLogNewline();
	
	return true;
}

void ExitFunction() {
	// Restore original todos.txt
	PrintLogNewline();
	PrintToLog("Restoring todos.txt...");
	system("copy ..\\..\\data\\todos_testbench_backup.txt ..\\..\\data\\todos.txt /y >> testbench_log.txt");
	// system("del ..\\..\\data\\todos_testbench_backup.txt");
	
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
	
	// Open log file
	system("echo Log start > testbench_log.txt");
	PrintLogNewline();
	
	// Backup and reset todos.txt
	PrintToLog("Backing up todos.txt...");
	system("copy ..\\..\\data\\todos.txt ..\\..\\data\\todos_testbench_backup.txt >> testbench_log.txt");
	PrintLogNewline();
	RegisterExitFunction(ExitFunction); // No matter what happens, the original will be restored and cleanup will be carried out
	system("del ..\\..\\data\\todos.txt /q");
	
	// Run testbench
	printf("\nRunning testbench...");
	
	const char* todayString = DateToString(GetDate(0));
	
	// Test add commands
	PrintToLog("Running add commands...");
	PrintLogNewline();
	const char* commands[] = { "todos add -s \"first task\" -dd 09/05/2026 -t \"tag 2\"",
														 "todos add -s \"task 2\" -dd 10/06/2026 -t \"tag 2\"",
		                         "todos add -s third -dd today",
		                         "todos add -s \"task number 4\" -t3 tag3"
														 };	
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
	PrintToLog("Running del commands...");
	PrintLogNewline();
	commands[0] = "todos del 1";
	commands[1] = "todos del 2";
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
	PrintToLog("Running mod commands...");
	PrintLogNewline();
	commands[0] = "todos mod -id 1 -s \"modded string\" -t2 \"new tag\"";
	commands[1] = "todos mod -id 2 -dd 10/10/2030+5 -t \"\"";
	CarryOutCommands(commands, 2);
	
	// Compare
	targetFileContents[0] = Concatenate(3, "\"modded string\" ", todayString, " 10/06/2026 \"tag 2\" \"new tag\" ");
	targetFileContents[1] = Concatenate(3, "\"task number 4\" ", todayString, " 15/10/2030 - ");
	comparison = LoadTodosAndCompare(targetFileContents, 2);
	if(comparison == false)
		goto program_exit;
	
	// Test undo command
	PrintToLog("Running undo command...");
	PrintLogNewline();
	commands[0] = "todos undo";
	CarryOutCommands(commands, 1);
	
	// Compare
	targetFileContents[1] = Concatenate(3, "\"task number 4\" ", todayString, " - \"tag3\" ");
	comparison = LoadTodosAndCompare(targetFileContents, 2);
	if(comparison == false)
		goto program_exit;
	
	// @TODO - List - do at the end
	
	printf("passed\n");
	PrintToLog("All tests passed");
	
	program_exit:
	return 0;
}