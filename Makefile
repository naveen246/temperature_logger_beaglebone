
CC = gcc
CFLAGS=-c
SRC = main.c io_lib.c lcd.c
OBJ = $(SRC:.c=.o)
EXE = temp_logger


build: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf $(OBJ) $(EXE) *~

rebuild: clean build
