cli:cli_main.o mylib.o
	gcc -o cli $^

%.o:%.c
	gcc -Wall -O2 -c $^
.PHONY:clean
clean:
	rm *.o cli *~
