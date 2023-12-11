CFLAG = -march=sapphirerapids -fno-strict-aliasing -mamx-tile -mamx-bf16 -std=c++23 -mavx512fp16 -O3
CC = g++
BIN = test-amxtile 
CFILES =test-amxtile.cpp

all:
	$(CC) $(CFLAG) $(CFILES) -o $(BIN) $(LIBS)

clean:
	-rm $(BIN)

.PHONY: clean

