@echo off

if not exist build ( mkdir build )
cd build

REM Create backup and reset todos
pushd ..\..\data
echo:
echo Backing up todos...
copy todos.txt todos_testbench_backup.txt /y
echo "task 1" 04/05/2026 09/05/2026 "tag 1" > todos.txt
popd

REM Commands
todos add -s "sample task" -dd 04/05/2026 -t "tag 2" > temp.txt

REM Create testbench
echo:
echo "task 1" 04/05/2026 09/05/2026 "tag 1" > testbench.txt
echo "sample task" %date% 04/05/2026 "tag 2" >> testbench.txt

REM Run comparison
echo Running tesbench...
comp ..\..\data\todos.txt testbench.txt /m /l /a >> temp.txt

REM Restore todos
pushd ..\..\data
echo Restoring todos...
copy todos_testbench_backup.txt todos.txt /y
del todos_testbench_backup.txt
popd