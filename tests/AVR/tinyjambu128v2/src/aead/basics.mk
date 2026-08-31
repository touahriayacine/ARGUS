OBJ = encrypt.o

default: all

all: $(OBJ)

encrypt.o: $(binaries_path)/encrypt.c
	$(CC) -c $(binaries_path)/encrypt.c $(cFlags)
