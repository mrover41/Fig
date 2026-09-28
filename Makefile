CC = gcc

ifeq ($(DEBUG), 1)
	CFLAGS += -Wall -Wextra -O2 -g -I/usr/local/include -DDEBUG
else
	CFLAGS += -Wall -Wextra -O2 -I/usr/local/include
endif

MAIN_LDFLAGS = -L/usr/local/lib -ldl -lpthread -lcurl -lssl -lcrypto -rdynamic
MOD_LDFLAGS = -ldl -rdynamic

BIN_NAME ?= loader.bin
APP_SRCS = $(wildcard *.c)

MOD_DIR = modules

MODULE_DIRS ?= \
#	./mod_dev/tst_mod/ \

CLEAN_MOD_DIRS = $(patsubst %/,%,$(MODULE_DIRS))
DEFAULT_MOD_TARGETS = $(patsubst %, $(MOD_DIR)/%.so, $(notdir $(CLEAN_MOD_DIRS)))

H_INCLUDES ?= \
	process.h \
	inject.h \

all: $(BIN_NAME) $(DEFAULT_MOD_TARGETS)

main: $(BIN_NAME)

modules: $(DEFAULT_MOD_TARGETS)

%: $(MOD_DIR)/%.so ;

$(BIN_NAME): $(APP_SRCS)
	$(CC) $(CFLAGS) $(APP_SRCS) -o $@ -L/usr/local/lib -Wl,--whole-archive -ldiscord -Wl,--no-whole-archive -ldl -lpthread -lcurl -lssl -lcrypto -rdynamic

$(MOD_DIR)/%.so: ./mod_dev/%
	@mkdir -p $(MOD_DIR)
	@for header in $(H_INCLUDES); do \
		ln -sf ../../$$header $</ 2>/dev/null || true; \
	done
	$(CC) $(CFLAGS) -shared -fPIC $(wildcard $</*.c) -o $@ $(MOD_LDFLAGS)

clean:
	rm -rf $(BIN_NAME) $(MOD_DIR)/*

.PHONY: all clean main modules
