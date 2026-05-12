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

void ExitFunction() {
	// Add the contents of todos.txt to log file
	PrintLogNewline();
	PrintToLog("todos.txt contents");
	PrintLogNewline();
	system("type ..\\..\\data\\todos.txt >> testbench_log.txt");
	PrintLogNewline();
	
	// Restore original todos.txt
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
	
	// Run a number of commands
	PrintToLog("Running add commands...");
	PrintLogNewline();
	const char* addCommands[] = { "todos add -s \"first task\" -dd 09/05/2026 -t \"tag 2\"",
														    "todos add -s \"task 2\" -dd 10/06/2026 -t \"tag 2\"",
		                            "todos add -s third -dd today",
		                            "todos add -s \"task number 4\" -t3 tag3"
															};	
	const char* delCommands[] = { "todos del 1", "todos del 2" };
	
	// Test add commands
	ForAll(GetArrayLength(addCommands)) {
		PrintToLog(addCommands[it]);
		
		if(it == 0) { // Need to add first task manually for now
			const char* string = Concatenate(3, "echo \"first task\" ", todayString, " 09/05/2026 \"tag 1\"  > ..\\..\\data\\todos.txt");
			system(string);
		}
		else
			system(Concatenate(2, addCommands[it], " >> temp.txt"));
	}
	
	// Run comparison	
	PrintLogNewline();
	PrintToLog("Comparing...");
	PrintLogNewline();
	const char* addFileString[] = { Concatenate(3, "\"first task\" ", todayString, " 09/05/2026 \"tag 1\" "),
																  Concatenate(3, "\"task 2\" ", todayString, " 10/06/2026 \"tag 2\" "),
																  Concatenate(5, "\"third\" ", todayString, " ", todayString, " - "),
																  Concatenate(3, "\"task number 4\" ", todayString, " - \"tag3\" ")
																  };
	todosFile = LoadFile("..\\..\\data\\todos.txt");
	const char* fileLine = (const char*)todosFile.memory;
	ForAll(GetArrayLength(addCommands)) {
		char* newLine = (char*)FindSubstring("\r", fileLine);
		Assert(newLine != Null);
		*newLine = '\0';
		
		if(StringsAreEqual(addFileString[it], fileLine) == false) {
			printf("failed\n");
			PrintToLog("Test failed");
			PrintToLog(Concatenate(2, "  Target: ", addFileString[it]));
			PrintToLog(Concatenate(2, "  Actual: ", fileLine));
			goto program_exit;
		}
		
		fileLine = newLine + 2;
	}
	FreeFile(todosFile);
	
	// Test del commands @WIP
	PrintToLog("Running del commands...");
	PrintLogNewline();
	ForAll(GetArrayLength(delCommands)) {
		PrintToLog(delCommands[it]);
		system(Concatenate(2, delCommands[it], " >> temp.txt"));
		
	}
	
	
	// @TODO - Del
	// @TODO - Mod
	// @TODO - Undo
	
	// @TODO - List - do at the end
	
	printf("passed\n");
	PrintToLog("All tests passed");
	
	program_exit:
	return 0;
}