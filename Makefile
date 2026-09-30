CC=gcc
FLAGS=-Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer
INC_DIR=hmc/alloc hmc/init hmc/ds hmc/sync base arch defs utils
SRC_DIR=test
BUILD_DIR=./
LIB=lib

FLAGS+=$(addprefix -I, $(INC_DIR))
CFILE=$(SRC_DIR)/$(TEST_FILE)

.PHONY: all

all:
	$(CC) -o test_file $(CFILE) $(FLAGS) -Llib -lhmc -Wl,-rpath,lib

