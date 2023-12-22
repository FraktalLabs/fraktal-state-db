CC := clang++
# TODO: Remove Wno's
CPPFLAGS ?= -std=c++20 -Wall -Wno-for-loop-analysis -Wno-unused-variable

INTX_PATH ?= ../intx
ETHASH_PATH ?= ../ethash

INTX_LIB_PATH ?= ${INTX_PATH}/include
ETHASH_LIB_PATH ?= ${ETHASH_PATH}/include
ETHASH_LINK_PATHS ?= $(shell find ${ETHASH_PATH}/build/lib -name '*.o')

INC_LIBS := -I ${INTX_LIB_PATH} -I ${ETHASH_LIB_PATH}
LINK_LIBS := ${ETHASH_LINK_PATHS}

BUILD_DIR := ./builds
BIN_DIR := ./bin
SRC_DIR := ./src

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
SRCS_WITH_SRC_STRIPPED := $(subst $(SRC_DIR)/,,$(SRCS))
OBJS := $(SRCS_WITH_SRC_STRIPPED:%=$(BUILD_DIR)/%.o)

all: evm-state-db

clear:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

evm-state-db: $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(INC_LIBS) $(LINK_LIBS) $(OBJS) -o $(BIN_DIR)/evm-state-db

$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $(INC_LIBS) -c $< -o $@

TEST_DIR := ./test
TEST_SNAPSHOT_FILE := ${TEST_DIR}/snapshot.json

get-test:
	$(BIN_DIR)/evm-state-db get --snapshotFile ${TEST_SNAPSHOT_FILE} --contractAddress 4200000000000000000000000000000000000aaa --key 42

set-test:
	$(BIN_DIR)/evm-state-db set --snapshotFile ${TEST_SNAPSHOT_FILE} --contractAddress 4200000000000000000000000000000000000aaa --key 42 --value 512
