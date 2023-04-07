main: main.o utilityFuncs.o
	gcc -o main main.o utilityFuncs.o -Wall -Werror -lpthread

main.o: main.c
	gcc -c main.c -Wall -Werror -lpthread

utilityFuncs.o: utilityFuncs.c
	gcc -c utilityFuncs.c -Wall -Werror -lpthread

clean:
	rm -rf *.o