.SUFFIXES: # disable builtin rules

OUTPUT_BASE	:=scopeclock

DEBUG		?= 0
TARGET	?=stm32

OBJ_DIR 		 := ./obj/$(TARGET)
SRC_DIR 		 := ./src
LIB_DIR 		 := ./lib
TARGET_DIR 	 := ./target/$(TARGET)
TEST_OBJ_DIR := ./obj/test
TEST_DIR 		 := ./test

C_LIB_SRC += $(wildcard $(LIB_DIR)/*.c)
C_TGT_SRC += $(wildcard $(TARGET_DIR)/*.c)
C_APP_SRC += $(wildcard $(SRC_DIR)/*.c)
C_SRC     += $(C_APP_SRC) $(C_TGT_SRC) $(C_LIB_SRC)
C_OBJ     := $(patsubst %, $(OBJ_DIR)/%, $(C_SRC:.c=.o))

ASM_LIB_SRC += $(wildcard $(LIB_DIR)/*.s)
ASM_TGT_SRC += $(wildcard $(TARGET_DIR)/*.s)
ASM_APP_SRC += $(wildcard $(SRC_DIR)/*.s)
ASM_SRC     += $(ASM_APP_SRC) $(ASM_TGT_SRC) $(ASM_LIB_SRC)
ASM_OBJ     := $(patsubst %, $(OBJ_DIR)/%, $(ASM_SRC:.s=.o))

OBJ         := $(C_OBJ) $(ASM_OBJ) $(OBJ_DIR)/version.o
DEPS        := $(C_OBJ:.o=.d)

ifeq ($(DEBUG),0)
  CFLAGS += -Os
else
  CFLAGS += -g -DDEBUG
endif
CFLAGS += -DTARGET_$(shell echo $(TARGET) | tr '[a-z]' '[A-Z]')
CFLAGS += -I$(SRC_DIR) -I$(LIB_DIR) -I$(TARGET_DIR)

TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_OUT = $(patsubst %, $(TEST_OBJ_DIR)/%, $(notdir $(TEST_SRC:.c=)))
TEST_DEPS = $(TEST_OUT:%=%.d)

VERSION := $(shell if ! git describe --always 2> /dev/null; then echo 0000000; fi)

.PHONY: all
all: $(OUTPUT_BASE) tests

-include $(TARGET_DIR)/Makefile.in

-include $(DEPS)

.PHONY: $(OBJ_DIR)/version.o
$(OBJ_DIR)/version.o:
	echo 'char * const version_tag = "$(VERSION)";' > $(OBJ_DIR)/version.c
	$(E_CC) $(CFLAGS) -c $(OBJ_DIR)/version.c -o $@

$(OBJ_DIR)/%.o: %.c
	$(E_CC) $(CFLAGS) -MMD -c -o $@ $<

$(OBJ_DIR)/%.o: %.s
	$(E_CC) $(ASMFLAGS) -MMD -c -o $@ $<

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/src
	mkdir -p $@/lib
	mkdir -p $@/$(TARGET_DIR)


.PHONY: tests
tests: $(TEST_OUT)

-include $(TEST_DEPS)

$(TEST_OBJ_DIR)/%: $(TEST_DIR)/%.c $(C_LIB_SRC)
	$(CC) -g -MMD -DTEST -o $@ -I $(LIB_DIR) $< $(C_LIB_SRC)

$(TEST_OUT): | $(TEST_OBJ_DIR)

$(TEST_OBJ_DIR):
	mkdir -p $@


.PHONY: clean
clean:
	rm -rf ./obj
	rm -f $(OUTPUT) *.elf *.bin *.map
