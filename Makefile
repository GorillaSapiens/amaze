amaze: amaze.c
	gcc -g amaze.c -o amaze -lm

debug: amaze.c
	gcc -g -DDEBUG_SIGHT amaze.c -o amaze -lm
