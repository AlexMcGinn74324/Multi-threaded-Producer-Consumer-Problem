main: main.o utilityFuncs.o queue.o
	gcc -o main main.o utilityFuncs.o queue.o -Wall -Werror -lpthread

main.o: main.c
	gcc -c main.c -Wall -Werror -lpthread

utilityFuncs.o: utilityFuncs.c
	gcc -c utilityFuncs.c -Wall -Werror -lpthread

queue.o: queue.c
	gcc -c queue.c -Wall -Werror -lpthread

clean:
	rm -rf *.o