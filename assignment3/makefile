CFLAGS=-Iinclude -Wall

all: build run

build: clean
	mkdir obj
	gcc $(CFLAGS) -c -o obj/policies.o src/policies.c	
	gcc $(CFLAGS) -c -o obj/simulator.o src/simulator.c
	mkdir bin
	gcc $(CFLAGS) -o bin/scheduling-simulator obj/simulator.o obj/policies.o

run:
#bin/scheduling-simulator "rr_tasks" RR 100 "rr_log"
#bin/scheduling-simulator "mfq_tasks" MFQ 100 "mfq_log"
#bin/scheduling-simulator "mfq_tasks" MFQ 100 "mfq_log" 1>scheduler-log
	bin/scheduling-simulator "iorr_tasks" IORR 100 "iorr_log"
#bin/sched-simulator "tasks" 1>scheduler-log.txt
#cat sched-run.txt

clean:
	rm -rf bin obj scheduler-log.txt
