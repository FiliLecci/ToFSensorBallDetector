a.out : coordList.h simulator.c
	gcc -Wall -g simulator.c -fsanitize=address -lm -lSDL2 -o a.out

run : a.out
	./$<
