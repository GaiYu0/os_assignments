CFLAGS=-Iinclude -Wall

all: clean
	mkdir obj
	gcc $(CFLAGS) -c -o obj/policies.o src/policies.c	
	gcc $(CFLAGS) -c -o obj/simulator.o src/simulator.c
	mkdir bin
	gcc $(CFLAGS) -o bin/scheduling-simulator obj/simulator.o obj/policies.o

rr: all
	bin/scheduling-simulator "test_cases/tasks" RR 4 1>scheduler-log.txt
	cat scheduler-log.txt

rr_extra: all
	bin/scheduling-simulator "test_cases/rr_tasks" RR 100 "test_cases/rr_log"

mfq: all
	bin/scheduling-simulator "test_cases/tasks" MFQ 4 1>scheduler-log.txt
	cat scheduler-log.txt

mfq_extra: all
	bin/scheduling-simulator "test_cases/mfq_tasks" MFQ 100 "test_cases/mfq_log"

iorr: all
	bin/scheduling-simulator "test_cases/tasks" IORR 4 1>scheduler-log.txt
	cat scheduler-log.txt

iorr_extra: all
	bin/scheduling-simulator "test_cases/iorr_tasks" IORR 100 "test_cases/iorr_log"

clean:
	rm -rf bin obj scheduler-log.txt
