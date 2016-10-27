CFLAGS=-Iinc -Wall

q1:
	gcc $(CFLAGS) -c -o obj/q1 src/1.c
	gcc $(CFLAGS) -c -o obj/q1-test src/1-test.c
	gcc ${CFLAGS} -o bin/q1 obj/q1 obj/q1-test
	bin/q1

q2:
	gcc $(CFLAGS) -o bin/q2 src/2.c
	bin/q2

q3_1:
	gcc $(CFLAGS) -o bin/q3-1 src/3-1.c
	bin/q3-1

q3_2:
	gcc $(CFLAGS) -o bin/q3-2 src/3-2.c

clean:
	rm -f bin/* obj/*
