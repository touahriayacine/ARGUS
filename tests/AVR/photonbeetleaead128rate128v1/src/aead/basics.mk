OBJ= photon.o encrypt.o

default: all

all: $(OBJ)

encrypt.o:  $(binaries_path)/encrypt.c
	$(CC) -c  $(binaries_path)/encrypt.c $(cFlags)

photon.o:  $(binaries_path)/photon.c
	$(CC) -c  $(binaries_path)/photon.c $(cFlags)