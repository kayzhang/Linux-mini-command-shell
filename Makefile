myShell: myShell.cpp command.h command.cpp shell.h shell.cpp
	g++ -std=gnu++03 -pedantic -Wall -Werror -ggdb3 -o myShell myShell.cpp command.cpp shell.cpp

.Phony: clean

clean:
	rm -f myShell *~ *.o *.out test
