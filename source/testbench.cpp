#include "apad_array.h"
#include "apad_error.h"
#include "apad_file.h"
#include "apad_string.h"
#include "apad_time.h"
#include "apad_win32.h"
#include <stdio.h>
#include <stdlib.h> // Run system commands

file todosFile = {};

void ExitFunction() {
	system("copy ..\\..\\data\\todos_testbench_backup.txt ..\\..\\data\\todos.txt /y");
	// system("del ..\\..\\data\\todos_testbench_backup.txt");
	if(IsValid(todosFile) == true)
		FreeFile(todosFile);
}

ConsoleAppEntryPoint(args, argsCount) {
	SetExitIfAssertionHit(true);
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(true);
	
	printf("\nBacking up todos.txt...");
	system("copy ..\\..\\data\\todos.txt ..\\..\\data\\todos_testbench_backup.txt");
	RegisterExitFunction(ExitFunction); // No matter what happens, the original will be restored and cleanup will be carried out
	
	system("echo \"task 1\" 04/05/2026 09/05/2026 \"tag 1\" > ..\\..\\data\\todos.txt"); // Manually create 1st task, todos.txt needs to have at least 1
	
	// Run a number of commands, then compare files
	printf("\nRunning testbench...\n");
	system("todos add -s \"sample task\" -dd 10/06/2026 -t \"tag 2\"");
	
	// Run comparison	
	const char* fileStrings[] = { "\"task 1\" 04/05/2026 09/05/2026 \"tag 1\"",
																Concatenate(3, "\"sample task\" ", DateToString(GetDate(0)), " 10/06/2026 \"tag 2\"")
																};
	todosFile = LoadFile("../../data/todos.txt");
	ui8 stringsRead = 0;
	LineReadLoopHeader(readIndex, todosFile) {
		// If this is hit, need to add more to fileString[]
		if(stringsRead >= GetArrayLength(fileStrings)) {
			printf("ERROR: more strings were written to todos.txt than were tested\n");
			break;
		}
		
		// Read line and compare with fileStrings[]
		auto line = ReadLine(todosFile, readIndex);
		char* lineString = (char*)line.data.memory;
		if(StringsAreEqual(lineString, fileStrings[stringsRead++]) == true)
			printf("Passed\n");
		else
			printf("Failed\n");
	}
	
	// @TODO - List
	// @TODO - Del
	// @TODO - Mod
	// @TODO - Undo	
	
	printf("\nTestbench finished.\n");
}