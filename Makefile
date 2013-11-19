
CC = gcc
SDIR = src
ODIR = obj
SRC1 = main.c io_lib.c lcd.c
SRC = $(patsubst %.c,$(SDIR)/%.c,$(SRC1))
OBJ = $(patsubst %.c,$(ODIR)/%.o,$(SRC1))
EXE = temp_logger


build: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $^ -o $@ -lm

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c $< -o $@

clean:
	rm -rf $(OBJ) $(EXE) *~

rebuild: clean build
