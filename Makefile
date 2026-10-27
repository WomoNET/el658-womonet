MCU := atmega16m1
F_CPU := 8000000UL

BUILD := build
TARGET := $(BUILD)/el658

AVR_CC := avr-gcc
AVR_OBJCOPY := avr-objcopy
AVR_SIZE := avr-size
HOST_CC := cc

# The logic is plain C and also builds on the host; hal/ and main.c are AVR only
LOGIC_SRC := $(filter-out src/main.c,$(wildcard src/*.c))
AVR_SRC := $(LOGIC_SRC) src/main.c $(wildcard src/hal/*.c)
TEST_SRC := $(wildcard test/*.c)
HEADERS := $(wildcard src/*.h src/hal/*.h)

# avr-gcc 16.2 with LTO and the assembler-generated ISR prologues (the default) emits
# an epilogue that leaves r18 of the interrupted code zeroed; the classic prologues
# save and restore it properly.
ISR_FLAGS := -mno-gas-isr-prologues

WARNINGS := -Wall -Wextra -Wshadow -Wstrict-prototypes -Werror
AVR_CFLAGS := -std=gnu11 $(WARNINGS) $(ISR_FLAGS) -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -flto \
	-ffunction-sections -fdata-sections
AVR_LDFLAGS := -Wl,--gc-sections
HOST_CFLAGS := -std=gnu11 $(WARNINGS) -Isrc -O1 -g -fsanitize=address,undefined

PORT ?=

.PHONY: all size test curve flash clean

all: $(TARGET).hex size

$(BUILD):
	mkdir -p $@

$(TARGET).elf: $(AVR_SRC) $(HEADERS) | $(BUILD)
	$(AVR_CC) $(AVR_CFLAGS) $(AVR_SRC) $(AVR_LDFLAGS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@

size: $(TARGET).elf
	$(AVR_SIZE) -C --mcu=$(MCU) $<

$(BUILD)/test_runner: $(LOGIC_SRC) $(TEST_SRC) $(HEADERS) $(wildcard test/*.h) | $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) $(LOGIC_SRC) $(TEST_SRC) -o $@

test: $(BUILD)/test_runner
	./$(BUILD)/test_runner

# Regenerate the dimming curve; pass e.g. CURVE_ARGS="--min-duty 30"
curve:
	python3 tools/gen_curve.py $(CURVE_ARGS) > src/curve_table.h

flash: $(TARGET).hex
	tools/flash.sh "$(PORT)" $<

clean:
	rm -rf $(BUILD)
