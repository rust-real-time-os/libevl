# SPDX-License-Identifier: MIT

ARCH = arm64
CROSS_COMPILE	= aarch64-linux-gnu-
CC		= $(CROSS_COMPILE)gcc
CXX		= $(CROSS_COMPILE)g++
LD		= $(CROSS_COMPILE)ld
AR		= $(CROSS_COMPILE)ar

DESTDIR	?= build
LIBEVL_OUTPUT ?= /tmp/output
BUILD_DIR ?= build
BUILD_TYPE ?= debug # Set this to release, debugoptimized, or default
LIBEPOS_WRAPPER := $(LIBEVL_OUTPUT)/lib/epos.wrappers
DEBUG_FLAGS = -O0 -g

CFLAGS = -I. -I$(LIBEVL_OUTPUT)/include -fasynchronous-unwind-tables -Wl,--no-as-needed -Wl,--wrap=main -D_GNU_SOURCE -Wl,@$(LIBEPOS_WRAPPER)

LDFLAGS = -L$(LIBEVL_OUTPUT)/lib -lepos -levl -lpthread -lrt -static

# Build type specific flags
ifeq ($(BUILD_TYPE),debug)
    CFLAGS += -O0 -g
else ifeq ($(BUILD_TYPE),release)
    CFLAGS += -O3
else ifeq ($(BUILD_TYPE),debugoptimized)
    CFLAGS += -O2 -g
endif

SOURCES = 	init/hello_world.c \
  			thread/pthread_create.c
			
TARGETS = $(patsubst %.c,$(BUILD_DIR)/%,$(SOURCES))

all: $(BUILD_DIR) $(TARGETS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: %.c | $(BUILD_DIR)
	BASENAME=$(basename $(notdir $@)) && \
	$(CC) $(CFLAGS) -o "$(BUILD_DIR)/$$BASENAME" $< $(LDFLAGS) && \
	aarch64-linux-gnu-objdump -d "$(BUILD_DIR)/$$BASENAME" > "$(BUILD_DIR)/$$BASENAME.txt"

clean :
	rm -f $(TARGETS)
