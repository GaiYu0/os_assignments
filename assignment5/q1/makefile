CFLAGS=-DDEBUG=0
INC=-Iinc

facility:
	gcc -c -o obj/facility.o src/facility.c ${INC}

client: obj/facility.o
	gcc -c -o obj/client.o src/client.c ${INC} ${CFLAGS} -lrt -lpthread
	gcc -o bin/client obj/client.o obj/facility.o -lrt -lpthread

server: obj/facility.o
	gcc -c -o obj/server.o src/server.c ${INC} ${CFLAGS} -lrt -lpthread
	gcc -o bin/server obj/server.o obj/facility.o -lrt -lpthread

clean:
	rm obj/* bin/*
