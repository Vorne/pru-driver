REPO_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
BUILD_DIR := $(REPO_ROOT)/xl
BUILD_TAG ?= bbb-release

include $(BUILD_DIR)/makefile.d/base.mk

BUILD_TAG_SET := bbb-debug bbb-release

ifeq ($(filter $(BUILD_TAG),$(BUILD_TAG_SET)),)
    $(error Unknown BUILD_TAG "$(BUILD_TAG)". Valid choices include { $(BUILD_TAG_SET) })
endif

TARGET := arm-linux-gnueabihf

# Fall back to OUTPUT_DIR. Keeps this build working if there is a mismatch with the xl repo.
LIB_DIR ?= static
LIBRARY_OUTPUT_DIR ?= $(OUTPUT_DIR)/$(LIB_DIR)

APP_LOADER_SUBDIR := pru_sw/app_loader
INTERFACE_SUBDIR := $(APP_LOADER_SUBDIR)/interface

INCLUDE_DIR := ./$(APP_LOADER_SUBDIR)/include

CFLAGS := -I$(INCLUDE_DIR)
CFLAGS += -I/usr/$(TARGET)/include
CFLAGS += -Wall
CFLAGS += --target=$(TARGET)

DRIVER_A := $(LIBRARY_OUTPUT_DIR)/libpru-driver.a

SOURCES := $(wildcard $(INTERFACE_SUBDIR)/*.c)
HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)
OBJS := $(SOURCES:%.c=$(LIBRARY_OUTPUT_DIR)/%.o)

.PHONY: all
all: $(DRIVER_A)

$(DRIVER_A): $(OBJS)
	@echo ARCHIVE $@
	$(AR) src $@ $(OBJS)

$(OBJS): $(LIBRARY_OUTPUT_DIR)/%.o: %.c $(HEADERS) | $(LIBRARY_OUTPUT_DIR)/$(INTERFACE_SUBDIR)
	@echo COMPILE $@
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY_OUTPUT_DIR)/$(INTERFACE_SUBDIR):
	@echo CREATE $@
	mkdir -p $@

.PHONY: clean
clean:
	@echo DELETE $(OUTPUT_DIR)
	$(RMRF) $(OUTPUT_DIR)

.PHONY: cleanall
cleanall: clean
	rm -rf build
