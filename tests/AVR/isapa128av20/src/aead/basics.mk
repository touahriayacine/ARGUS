OBJ = Ascon-reference.o isap.o crypto_aead.o

default: all

all: $(OBJ)

crypto_aead.o: $(binaries_path)/crypto_aead.c
	$(CC) -c $(binaries_path)/crypto_aead.c $(cFlags)
	
Ascon-reference.o: $(binaries_path)/Ascon-reference.c
	$(CC) -c $(binaries_path)/Ascon-reference.c $(cFlags)

isap.o: $(binaries_path)/isap.c
	$(CC) -c $(binaries_path)/isap.c $(cFlags)