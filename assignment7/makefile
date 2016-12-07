INCLUDE=-I.
all: clean server client

array:
	gcc -o object/array dynamic_array.c -c ${INCLUDE}
client:
	gcc -o object/client client.c -c ${INCLUDE}
	gcc -o client object/client object/utility ${INCLUDE}
server:	clean array utility
	gcc -o object/server server.c -c -lpthread ${INCLUDE} -D_REENTRANT
	gcc -o server object/server object/array object/utility -lpthread ${INCLUDE}
utility:
	gcc -o object/utility utility.c -c ${INCLUDE}
clean:
	rm -f ./server ./client ./array-test ./object/*
