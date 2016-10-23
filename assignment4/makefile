CFLAGS=-Iinc -Wall

q1:
	gcc $(CFLAGS) -c -o obj/q1 src/1.c
	gcc $(CFLAGS) -c -o obj/q1-test src/1-test.c
	gcc ${CFLAGS} -o bin/q1 obj/q1 obj/q1-test
	bin/q1
q2:
	gcc $(CFLAGS) -c -o obj/q2 src/2.c
	gcc $(CFLAGS) -c -o obj/q2-test src/2-test.c
	gcc ${CFLAGS} -o bin/q2 obj/q2 obj/q2-test
	bin/q2
q3:
	gcc $(CFLAGS) -o bin/q3 src/3.c
	bin/q3

q4:
	gcc $(CFLAGS) -o bin/q4 src/4.c
	bin/q4

clean:
	rm -f bin/* obj/*
