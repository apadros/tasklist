#include "apad_error.h"
#include "apad_file.h"
#include "apad_win32.h"
#include <stdio.h>
#include <stdlib.h> // Run system commands

void ExitFunction() {
	system("copy ..\\..\\data\\todos_testbench_backup.txt ..\\..\\data\\todos.txt /y");
	system("del ..\\..\\data\\todos_testbench_backup.txt");
}

ConsoleAppEntryPoint(args, argsCount) {
	SetExitIfAssertionHit(true);
	SetDisplayAPIAssertions(true);
	SetCallExitInAPIAssertions(true);
	
	printf("\nBacking up todos.txt...");
	system("copy ..\\..\\data\\todos.txt ..\\..\\data\\todos_testbench_backup.txt");
	RegisterExitFunction(ExitFunction); // No matter what happens, the original will be restored
	
	auto file = LoadFile("../../data/todos.txt");
	
	printf("\nRunning testbench...\n");
	system("echo \"task 1\" 04/05/2026 09/05/2026 \"tag 1\" > ..\\..\\data\\todos.txt"); // Manually create 1st task, todos.txt needs to have at least 1
	
	printf("\nTesting Add command...");
	system("todos add -s \"sample task\", -dd 10/06/2026 -t \"tag 1\"");
	
	
	// @TODO - Run a comp?
	printf("OK\n");
	
	// @TODO - List
	// @TODO - Del
	// @TODO - Mod
	// @TODO - Undo	
	
	printf("\nTestbench finished.\n");
	
	FreeFile(file);
}