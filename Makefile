.SUFFIXES: # disable builtin rules

E_CC?=arm-none-eabi-gcc
E_OBJCOPY?=arm-none-eabi-objcopy

OUTPUT:=scopeclock
DEBUG?=1
OBJ_DIR = ./obj
SRC_DIR = ./src

C_SRC += $(wildcard $(SRC_DIR)/*.c)
ASM_SRC += $(wildcard $(SRC_DIR)/*.s)
LD_CONFIG = $(SRC_DIR)/ld_config.ld

C_OBJ = $(patsubst %, $(OBJ_DIR)/%, $(notdir $(C_SRC:.c=.o)))
ASM_OBJ = $(patsubst %, $(OBJ_DIR)/%, $(notdir $(ASM_SRC:.s=.o)))
OBJ = $(C_OBJ) $(ASM_OBJ)
DEPS = $(C_OBJ:.o=.d)

CPUFLAGS := -mcpu=cortex-m3 -mthumb -DSTM32F100xB
ifeq ($(DEBUG),0)
  CFLAGS += -Os
else
  CFLAGS += -g
endif
CFLAGS += -ffreestanding -nostdlib $(CPUFLAGS) -I$(SRC_DIR)/hw
ASMFLAGS += -ffreestanding -nostdlib $(CPUFLAGS)
LDFLAGS += -ffreestanding -nostdlib $(CPUFLAGS) -Wl,-Map=$(OUTPUT_MAP) -Wl,--gc-sections

OUTPUT_ELF = $(OUTPUT).elf
OUTPUT_BIN = $(OUTPUT).bin
OUTPUT_MAP = $(OUTPUT).map

.PHONY: all
all: $(OUTPUT_BIN)

-include $(DEPS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(E_CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	$(E_CC) $(ASMFLAGS) -MMD -c $< -o $@

$(OUTPUT_ELF): $(OBJ) $(LD_CONFIG)
	$(E_CC) $(LDFLAGS) -T$(LD_CONFIG) $(OBJ) -o $@

%.bin: %.elf
	$(E_OBJCOPY) -O binary $< $@

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@

.PHONY: flash
flash:
	st-flash --reset write $(OUTPUT_BIN) 0x08000000

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)
	rm -f *.elf *.bin *.map *.log *.s
