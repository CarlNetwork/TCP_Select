serv:serv_main.o mylib.o
	gcc -o serv $^

%.o:%.c
	gcc -Wall -O2 -c $^
.PHONY:clean
clean:
	rm *.o serv *~
