INCLUDE=-I.
server:
	gcc -o server server.c -pthread ${INCLUDE}
client:
	gcc -o client client.c ${INCLUDE}
array:
	gcc -o object/array dynamic_array.c -c ${INCLUDE}
	gcc -o object/array_test dynamic_array_test.c -c ${INCLUDE}
	gcc -o array-test object/array object/array_test
clean:
	rm -f ./server ./client ./array-test ./object/*
