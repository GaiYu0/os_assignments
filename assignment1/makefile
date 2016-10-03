CC=gcc -Wall -ansi
BIN=bin
INC=include
LIB=lib
OBJ=obj
SRC=src

# ####################################################################################################
# Q2 START
# compile lib_stack_array.a
# ####################################################################################################
STACK_SIZE=1024
${OBJ}/stack_array.o: ${INC}/stack.h ${SRC}/stack_array.c
	${CC} -c -o ${OBJ}/stack_array.o ${SRC}/stack_array.c -I${INC} -D STACK_SIZE=${STACK_SIZE}

${LIB}/lib_stack_array.a: ${OBJ}/stack_array.o
	ar r $@ $<
# ####################################################################################################
# Q2 END
# ####################################################################################################

# ####################################################################################################
# Q3 START
# ####################################################################################################
Q3: ${SRC}/stack_test.c ${LIB}/lib_stack_array.a
	${CC} -c -o ${OBJ}/stack_test_Q3.o ${SRC}/stack_test.c -I${INC} -D STACK_SIZE=${STACK_SIZE}
	${CC} -o ${BIN}/stack_test_Q3 ${OBJ}/stack_test_Q3.o ${LIB}/lib_stack_array.a 
	@echo "Testing Stack"
	${BIN}/stack_test_Q3
# ####################################################################################################
# Q3 END
# ####################################################################################################

# compile list.o
${OBJ}/list.o: ${SRC}/list_impl.c
	${CC} -c -o ${OBJ}/list.o ${SRC}/list_impl.c -I${INC}

# compile list-based stack
${OBJ}/stack_list.o: ${INC}/list.h ${SRC}/stack_list.c
	${CC} -c -o ${OBJ}/stack_list.o ${SRC}/stack_list.c -I${INC}

${LIB}/lib_stack_list.a: ${OBJ}/stack_list.o ${OBJ}/list.o
	ar r $@ ${OBJ}/stack_list.o ${OBJ}/list.o

# compile stack_test
${OBJ}/stack_test.o: ${INC}/stack.h ${SRC}/stack_test.c
	${CC} -c -o ${OBJ}/stack_test.o ${SRC}/stack_test.c -I${INC}

${BIN}/stack_test: ${OBJ}/stack_test.o ${LIB}/lib_stack_list.a
	${CC} -o ${BIN}/stack_test ${OBJ}/stack_test.o ${LIB}/lib_stack_list.a 

stack: ${BIN}/stack_test
	@echo "Testing Stack"
	${BIN}/stack_test

# compile queue_test
${OBJ}/fifo_list.o: ${INC}/list.h ${SRC}/fifo_list.c
	${CC} -c -o ${OBJ}/fifo_list.o ${SRC}/fifo_list.c -I${INC}

${LIB}/libfifo.a: ${OBJ}/fifo_list.o ${OBJ}/list.o
	ar r $@ ${OBJ}/fifo_list.o ${OBJ}/list.o

${OBJ}/fifo_test.o: ${INC}/fifo.h ${SRC}/fifo_test.c
	${CC} -c -o ${OBJ}/fifo_test.o ${SRC}/fifo_test.c -I${INC}

${BIN}/queue_test: ${OBJ}/fifo_test.o ${LIB}/libfifo.a
	${CC} -o ${BIN}/queue_test ${OBJ}/fifo_test.o ${LIB}/libfifo.a

queue: ${BIN}/queue_test
	@echo "Testing Queue"
	${BIN}/queue_test
	
clean:
	rm -f ${OBJ}/* ${BIN}/* ${LIB}/*
