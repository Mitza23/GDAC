CC = gcc
CFLAGS = -Wall -g -ggdb

SRC = lib/ult.c lib/mutex.c lib/util.c lib/barrier.c
OBJ = $(patsubst %.c,%.o,$(SRC))

# pattern rule for object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

# rule to create the bin directory
bin:
	mkdir -p bin

main: bin main.o $(OBJ)
	$(CC) -o bin/main main.o $(OBJ)

mutex: bin mutex_test.o $(OBJ)
	$(CC) -o bin/mutex_test mutex_test.o $(OBJ)

barrier: bin barrier_test.o $(OBJ)
	$(CC) -o bin/barrier_test barrier_test.o $(OBJ)
	
deadlocks: bin deadlocks.o $(OBJ)
	$(CC) -o bin/deadlocks deadlocks.o $(OBJ)

clean:
	$(RM) -f *.o lib/*.o bin/*