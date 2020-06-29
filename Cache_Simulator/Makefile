all:first

first: first.c
	gcc -std=c11 -Wall -Werror -fsanitize=address -o first first.c -lm
	
clean:
	rm -f first
