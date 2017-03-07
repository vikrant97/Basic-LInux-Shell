main: main.o functions.o
	gcc main.o functions.o -o main
main.o: main.c functions.h
	gcc -c main.c  
functions.o: functions.c functions.h
	gcc -c functions.c   
clean:
	rm -f main main.o functions.o
