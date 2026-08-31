# ─────────────────────────────────────────────────────────────────────────────
# AVR / Arduino Makefile
# Target: Arduino Uno / ATmega328P
# Usage:
#   make -f /path/to/Makefile SRC_DIR=/path/to/project
# ─────────────────────────────────────────────────────────────────────────────


# ── Toolchain ────────────────────────────────────────────────────────────────

CC      := avr-gcc
CXX     := avr-g++
AR      := avr-gcc-ar
RANLIB  := avr-gcc-ranlib


# ── MCU configuration ────────────────────────────────────────────────────────

MCU    := atmega328p
F_CPU  := 16000000L


# ── Input project directory ──────────────────────────────────────────────────

ifndef SRC_DIR
    $(error SRC_DIR is not set. Usage: make SRC_DIR=/path/to/your/project)
endif

SRC_DIR := $(abspath $(SRC_DIR))

DEST_DIR := $(SRC_DIR)/build


# ── Makefile directory ───────────────────────────────────────────────────────
#
# Allows framework-arduino-avr to be located relative to this Makefile,
# regardless of the directory from which make is executed.

MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


# ── Arduino framework ────────────────────────────────────────────────────────

FRAMEWORK_ARDUINO := $(MAKEFILE_DIR)/framework-arduino-avr

ARDUINO_CORE := $(FRAMEWORK_ARDUINO)/cores/arduino

ARDUINO_VARIANT := $(FRAMEWORK_ARDUINO)/variants/standard


# ── Defines ──────────────────────────────────────────────────────────────────

COMMON_DEFS := \
    -DPLATFORMIO=60118 \
    -DARDUINO_AVR_UNO \
    -DF_CPU=$(F_CPU) \
    -DARDUINO_ARCH_AVR \
    -DARDUINO=10808 \
    -D__PLATFORMIO_BUILD_DEBUG__


# ── Common compiler flags ────────────────────────────────────────────────────

COMMON_FLAGS := \
    -mmcu=$(MCU) \
    -Wall \
    -ffunction-sections \
    -fdata-sections \
    -flto \
    -Os


# ── Include directories ──────────────────────────────────────────────────────

INC_ARD := \
    -I$(ARDUINO_CORE) \
    -I$(ARDUINO_VARIANT)

PROJECT_INCLUDES := \
    -I$(SRC_DIR)


# ── Compiler flags ───────────────────────────────────────────────────────────

CFLAGS := \
    -std=gnu11 \
    -fno-fat-lto-objects \
    $(COMMON_FLAGS) \
    $(COMMON_DEFS) \
    $(INC_ARD) \
    $(PROJECT_INCLUDES)


CXXFLAGS := \
    -std=gnu++11 \
    -fno-exceptions \
    -fno-threadsafe-statics \
    -fpermissive \
    -fno-fat-lto-objects \
    $(COMMON_FLAGS) \
    $(COMMON_DEFS) \
    $(INC_ARD) \
    $(PROJECT_INCLUDES)


ASFLAGS := \
    -x assembler-with-cpp \
    -mmcu=$(MCU) \
    $(COMMON_DEFS) \
    $(INC_ARD)


# ── Recursive wildcard ───────────────────────────────────────────────────────

rwildcard = $(foreach d,$(wildcard $(1:=/*)),\
             $(call rwildcard,$d,$2) \
             $(filter $(subst *,%,$2),$d))


# ── Project sources ──────────────────────────────────────────────────────────

C_SOURCES := \
    $(filter-out $(SRC_DIR)/main.c,\
    $(call rwildcard,$(SRC_DIR),*.c))


CXX_SOURCES := \
    $(filter-out $(SRC_DIR)/main.cpp,\
    $(call rwildcard,$(SRC_DIR),*.cpp))


# ── Main source ──────────────────────────────────────────────────────────────

MAIN_SRC := $(or \
    $(wildcard $(SRC_DIR)/main.cpp), \
    $(wildcard $(SRC_DIR)/main.c) \
)

ifeq ($(MAIN_SRC),)
    $(error No main.c or main.cpp found inside $(SRC_DIR))
endif


# ── Project objects ──────────────────────────────────────────────────────────
#
# Basename-only objects are used here for compatibility with your previous
# Makefile.
#
# Note:
# Two files with the same basename in different directories will collide.
#
# Example:
#
#   src/foo/utils.cpp
#   src/bar/utils.cpp
#
# both become:
#
#   build/utils.o

C_OBJ := \
    $(addprefix $(DEST_DIR)/,\
    $(notdir $(C_SOURCES:.c=.o)))


CXX_OBJ := \
    $(addprefix $(DEST_DIR)/,\
    $(notdir $(CXX_SOURCES:.cpp=.o)))


OBJ := $(C_OBJ) $(CXX_OBJ)


MAIN_OBJ := \
    $(DEST_DIR)/$(notdir \
    $(basename $(MAIN_SRC))).o


# ── Arduino core sources ─────────────────────────────────────────────────────

ARDUINO_C_SRC := \
    WInterrupts.c \
    hooks.c \
    wiring.c \
    wiring_analog.c \
    wiring_digital.c \
    wiring_pulse.c \
    wiring_shift.c


ARDUINO_CPP_SRC := \
    CDC.cpp \
    HardwareSerial.cpp \
    HardwareSerial0.cpp \
    HardwareSerial1.cpp \
    HardwareSerial2.cpp \
    HardwareSerial3.cpp \
    IPAddress.cpp \
    PluggableUSB.cpp \
    Print.cpp \
    Stream.cpp \
    Tone.cpp \
    USBCore.cpp \
    WMath.cpp \
    WString.cpp \
    abi.cpp \
    main.cpp \
    new.cpp


ARDUINO_S_SRC := \
    wiring_pulse.S


# ── Arduino objects ──────────────────────────────────────────────────────────

ARDUINO_C_OBJS := \
    $(addprefix $(DEST_DIR)/arduino_,\
    $(ARDUINO_C_SRC:.c=.o))


ARDUINO_CPP_OBJS := \
    $(addprefix $(DEST_DIR)/arduino_,\
    $(ARDUINO_CPP_SRC:.cpp=.o))


ARDUINO_S_OBJS := \
    $(addprefix $(DEST_DIR)/arduino_,\
    $(ARDUINO_S_SRC:.S=.o))


ARDUINO_OBJS := \
    $(ARDUINO_C_OBJS) \
    $(ARDUINO_CPP_OBJS) \
    $(ARDUINO_S_OBJS)


# ── Output files ─────────────────────────────────────────────────────────────

LIB_ARD := $(DEST_DIR)/libFrameworkArduino.a

TARGET := $(DEST_DIR)/firmware.elf


# ── vpath ────────────────────────────────────────────────────────────────────
#
# Allows Make to locate source files using their basename.

vpath %.c \
    $(sort $(dir $(C_SOURCES))) \
    $(SRC_DIR) \
    $(ARDUINO_CORE)


vpath %.cpp \
    $(sort $(dir $(CXX_SOURCES))) \
    $(SRC_DIR) \
    $(ARDUINO_CORE)


vpath %.S \
    $(ARDUINO_CORE)


# ── Targets ──────────────────────────────────────────────────────────────────

.PHONY: all clean info

all: $(TARGET)


# ── Build directory ──────────────────────────────────────────────────────────

$(DEST_DIR):
	mkdir -p $@


# ── Final link ───────────────────────────────────────────────────────────────
#
# Use avr-g++ rather than avr-gcc because Arduino contains C++ code.

$(TARGET): $(MAIN_OBJ) $(OBJ) $(LIB_ARD) | $(DEST_DIR)
	$(CXX) \
	    -mmcu=$(MCU) \
	    -Wl,--gc-sections \
	    -flto \
	    -fuse-linker-plugin \
	    -Os \
	    -o $@ \
	    $(MAIN_OBJ) \
	    $(OBJ) \
	    -Wl,--start-group \
	    $(LIB_ARD) \
	    -lm \
	    -Wl,--end-group


# ── Arduino static library ───────────────────────────────────────────────────

$(LIB_ARD): $(ARDUINO_OBJS) | $(DEST_DIR)
	$(AR) rc $@ $^
	$(RANLIB) $@


# ── Project C compilation ────────────────────────────────────────────────────

$(DEST_DIR)/%.o: %.c | $(DEST_DIR)
	$(CC) \
	    -c \
	    -o $@ \
	    $(CFLAGS) \
	    $<


# ── Project C++ compilation ──────────────────────────────────────────────────

$(DEST_DIR)/%.o: %.cpp | $(DEST_DIR)
	$(CXX) \
	    -c \
	    -o $@ \
	    $(CXXFLAGS) \
	    $<


# ── Arduino C compilation ────────────────────────────────────────────────────

$(DEST_DIR)/arduino_%.o: $(ARDUINO_CORE)/%.c | $(DEST_DIR)
	$(CC) \
	    -c \
	    -o $@ \
	    $(CFLAGS) \
	    $<


# ── Arduino C++ compilation ──────────────────────────────────────────────────

$(DEST_DIR)/arduino_%.o: $(ARDUINO_CORE)/%.cpp | $(DEST_DIR)
	$(CXX) \
	    -c \
	    -o $@ \
	    $(CXXFLAGS) \
	    $<


# ── Arduino assembly compilation ─────────────────────────────────────────────

$(DEST_DIR)/arduino_%.o: $(ARDUINO_CORE)/%.S | $(DEST_DIR)
	$(CC) \
	    -c \
	    -o $@ \
	    $(ASFLAGS) \
	    $<


# ── Information ──────────────────────────────────────────────────────────────

info:
	@echo "SRC_DIR            = $(SRC_DIR)"
	@echo "DEST_DIR           = $(DEST_DIR)"
	@echo "MAKEFILE_DIR       = $(MAKEFILE_DIR)"
	@echo "FRAMEWORK_ARDUINO  = $(FRAMEWORK_ARDUINO)"
	@echo "ARDUINO_CORE       = $(ARDUINO_CORE)"
	@echo "ARDUINO_VARIANT    = $(ARDUINO_VARIANT)"
	@echo "MCU                = $(MCU)"
	@echo "F_CPU              = $(F_CPU)"
	@echo "MAIN_SRC           = $(MAIN_SRC)"
	@echo "TARGET             = $(TARGET)"


# ── Clean ────────────────────────────────────────────────────────────────────

clean:
	rm -rf $(DEST_DIR)