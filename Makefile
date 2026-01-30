all: amaze amaze2

amaze: amaze.c
	gcc -g -Wall amaze.c -o amaze -lm

amaze2: amaze2.c
	gcc -g -Wall amaze2.c -o amaze2 -lm

debug: amaze.c
	gcc -g -Wall -DDEBUG_SIGHT amaze.c -o amaze -lm

debug2: amaze2.c
	gcc -g -Wall -DDEBUG_SIGHT amaze2.c -o amaze2 -lm
