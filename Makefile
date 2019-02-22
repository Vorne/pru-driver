BUILD_TAG ?= bbb-release
include $(BUILD_DIR)/makefile.d/base.mk

# Fall back to OUTPUT_DIR. Keeps this build working if there is a mismatch with the xl repo.
LIBRARY_OUTPUT_DIR ?= $(OUTPUT_DIR)

ROOTDIR = .

INCLUDEDIR = ./pru_sw/app_loader/include

CFLAGS += -I. -I$(INCLUDEDIR) -Wall -Wno-unused-result

COMPILE.c = $(CC) $(CFLAGS) $(CPP_FLAGS) -c
AR.c = $(AR) rc
LINK.c = $(CC) -shared

DRIVER_A = $(LIBRARY_OUTPUT_DIR)/libpru-driver.a

SOURCES = $(wildcard pru_sw/app_loader/interface/*.c)

PUBLIC_HDRS = $(wildcard $(INCLUDEDIR)/*.h)
PRIVATE_HDRS = $(wildcard *.h)
HEADERS = $(PUBLIC_HDRS) $(PRIVATE_HDRS)

OBJS = $(SOURCES:%.c=$(LIBRARY_OUTPUT_DIR)/%.o)

all: $(DRIVER_A)

$(LIBRARY_OUTPUT_DIR):
	$(VERBOSE)mkdir -p $(LIBRARY_OUTPUT_DIR)/pru_sw/app_loader/interface

$(DRIVER_A): $(OBJS)
	@mkdir -p $(ROOTDIR)/lib
	@echo $(AR) src $@ $(OBJS)
	@echo OBJS: $(OBJS)
	$(AR) src $@ $(OBJS)

$(OBJS): $(LIBRARY_OUTPUT_DIR)/%.o: %.c $(HEADERS) | $(LIBRARY_OUTPUT_DIR)
	$(CC) $(CFLAGS) $(PROFILE_FLAGS) -c $< -o $@

clean:
	-rm -rf *~ ./lib/* $(LIBRARY_OUTPUT_DIR)
