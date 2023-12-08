BLD = ./build
SRC = ./src
BLOCK_SIZE = 8G

all: 
	g++ -O3 -g -Wpedantic -Wextra -Wall -o $(BLD)/main main.cpp $(SRC)/BloomStore.cpp $(SRC)/BloomFilter.cpp

test: all
	$(BLD)/main `cat configure`

init:
	./script/init_block_device.sh

deinit:
	./script/deinit_block_device.sh

generate_block:
	sudo dd if=/dev/zero of=./raw_block ibs=1 count=$(BLOCK_SIZE)

debug: all
	gdb --args $(BLD)/main `cat configure`

clean:
	rm -rf ./$(BLD)/*