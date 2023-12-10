CFLAG = -O2 -march=sapphirerapids -fno-strict-aliasing -mamx-tile -mamx-int8 -mamx-bf16 -std=c++23 -mavx512fp16
CC = g++
BIN = test-amxtile 
CFILES =test-amxtile.cpp

all:
	$(CC) $(CFLAG) $(CFILES) -o $(BIN) $(LIBS)

clean:
	-rm $(BIN)

.PHONY: clean

