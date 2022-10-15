.SUFFIXES: # disable builtin rules

E_CC?=arm-none-eabi-gcc
E_OBJCOPY?=arm-none-eabi-objcopy

OUTPUT:=scopeclock
DEBUG?=0
OBJ_DIR = ./obj
SRC_DIR = ./src
LIB_DIR = ./lib
TEST_DIR = ./test

C_LIB_SRC += $(wildcard $(LIB_DIR)/*.c)
C_SRC += $(wildcard $(SRC_DIR)/*.c) $(C_LIB_SRC)
ASM_SRC += $(wildcard $(SRC_DIR)/*.s)
LD_CONFIG = $(SRC_DIR)/ld_config.ld

C_OBJ = $(patsubst %, $(OBJ_DIR)/%, $(C_SRC:.c=.o))
ASM_OBJ = $(patsubst %, $(OBJ_DIR)/%, $(ASM_SRC:.s=.o))
OBJ = $(C_OBJ) $(ASM_OBJ) $(OBJ_DIR)/version.o
DEPS = $(C_OBJ:.o=.d)

CPUFLAGS := -mcpu=cortex-m3 -mthumb -DSTM32F100xB
ifeq ($(DEBUG),0)
  CFLAGS += -Os
else
  CFLAGS += -g -DDEBUG
endif
CFLAGS += -ffreestanding -nostdlib $(CPUFLAGS) -I$(SRC_DIR)/hw
ASMFLAGS += -ffreestanding -nostdlib $(CPUFLAGS)
LDFLAGS += -ffreestanding -nostdlib $(CPUFLAGS) -Wl,-Map=$(OUTPUT_MAP) -Wl,--gc-sections

OUTPUT_ELF = $(OUTPUT).elf
OUTPUT_BIN = $(OUTPUT).bin
OUTPUT_MAP = $(OUTPUT).map

TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_OUT = $(TEST_SRC:.c=)
TEST_DEPS = $(TEST_SRC:.c=.d)

VERSION := $(shell if ! git describe --always 2> /dev/null; then echo 0000000; fi)

.PHONY: all
all: tests $(OUTPUT_BIN)

-include $(DEPS)

.PHONY: $(OBJ_DIR)/version.o
$(OBJ_DIR)/version.o:
	echo 'char * const version_tag = "$(VERSION)";' > $(OBJ_DIR)/version.c
	$(E_CC) $(CFLAGS) -c $(OBJ_DIR)/version.c -o $@

$(OBJ_DIR)/%.o: %.c
	$(E_CC) $(CFLAGS) -MMD -c -I $(LIB_DIR) -o $@ $<

$(OBJ_DIR)/%.o: %.s
	$(E_CC) $(ASMFLAGS) -MMD -c -I $(LIB_DIR) -o $@ $<

$(OUTPUT_ELF): $(OBJ) $(LD_CONFIG)
	$(E_CC) $(LDFLAGS) -T$(LD_CONFIG) $(OBJ) -o $@

%.bin: %.elf
	$(E_OBJCOPY) -O binary $< $@

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/src
	mkdir -p $@/lib

.PHONY: flash
flash:
	st-flash --reset write $(OUTPUT_BIN) 0x08000000

.PHONY: tests
tests: $(TEST_OUT)

-include $(TEST_DEPS)

$(TEST_DIR)/%: $(TEST_DIR)/%.c $(C_LIB_SRC)
	$(CC) -g -MMD -DTEST -o $@ -I $(LIB_DIR) $< $(C_LIB_SRC)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)
	rm -f *.elf *.bin *.map *.log *.s
	rm -rf $(TEST_OUT) $(TEST_DIR)/*.d $(TEST_DIR)/*.dSYM
