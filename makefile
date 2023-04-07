main: main.o
	gcc -o main main.o -Wall -Werror -lpthread

main.o: main.c
	gcc -c main.c -Wall -Werror -lpthread

clean:
	rm -rf *.o