CC = gcc

ifeq ($(DEBUG), 1)
	CFLAGS += -g -DDEBUG
endif

CFLAGS += -Wall -Wextra -O2 -I/usr/local/include -I./include -I./comm

MAIN_LDFLAGS = -L/usr/local/lib -ldl -lpthread -lcurl -lssl -lcrypto -rdynamic
MOD_LDFLAGS = -ldl -rdynamic

BIN_NAME ?= loader.bin
APP_SRCS = $(wildcard src/*.c)

BUILD_DIR = builds
MOD_DIR = $(BUILD_DIR)/modules

MODULE_DIRS ?= \
#	./src/mod_dev/tst_mod/ \

CLEAN_MOD_DIRS = $(patsubst %/,%,$(MODULE_DIRS))
DEFAULT_MOD_TARGETS = $(patsubst %, $(MOD_DIR)/%.so, $(notdir $(CLEAN_MOD_DIRS)))

all: $(BUILD_DIR) $(BIN_NAME) $(DEFAULT_MOD_TARGETS)

main: $(BUILD_DIR) $(BIN_NAME)

modules: $(MOD_DIR) $(DEFAULT_MOD_TARGETS)

$(BUILD_DIR) $(MOD_DIR):
	mkdir -p $@

%: $(MOD_DIR)/%.so ;

$(BIN_NAME): $(APP_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(APP_SRCS) -o $(BUILD_DIR)/$@ -L/usr/local/lib -Wl,--whole-archive -ldiscord -Wl,--no-whole-archive -ldl -lpthread -lcurl -lssl -lcrypto -rdynamic

$(MOD_DIR)/%.so: ./src/mod_dev/%
	@mkdir -p $(MOD_DIR)
	$(CC) $(CFLAGS) -shared -fPIC $(wildcard $</*.c) -o $@ $(MOD_LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean main modules
