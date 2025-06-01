# Makefile for Coin Archana Berry (CAB) dan Pool

CC       := gcc
CFLAGS   := -Iinclude -Wall
LDFLAGS  := -lcrypto

SRC_DIR   := source
BUILD_DIR := build
TARGET    := $(BUILD_DIR)/cabcoin
POOL_BIN  := $(BUILD_DIR)/pool

SRCS_CAB := \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/blockchain.c \
    $(SRC_DIR)/wallet.c \
    $(SRC_DIR)/miner.c \
    $(SRC_DIR)/cabcrypto.c

SRCS_POOL := \
    $(SRC_DIR)/pool.c \
    $(SRC_DIR)/cabcrypto.c

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET) $(POOL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build cabcoin (client + miner)
$(TARGET): $(SRCS_CAB)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS_CAB) $(LDFLAGS)

# Build pool server
$(POOL_BIN): $(SRCS_POOL)
	$(CC) $(CFLAGS) -o $(POOL_BIN) $(SRCS_POOL) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET) $(POOL_BIN)
