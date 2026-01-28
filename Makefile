# Makefile for Solar-Synchronized Light Controller
# Target: PIC18F67K40
# Compiler: Microchip XC8
#
# Note: This is a reference Makefile for command-line builds.
# For MPLAB X IDE, use the IDE's build system instead.

# Compiler and tools
CC = xc8-cc
OBJCOPY = xc8-objcopy
PROGRAMMER = pk4tool  # Adjust based on your programmer

# Target device
DEVICE = 18F67K40

# Compiler flags
CFLAGS = -mcpu=$(DEVICE) \
         -std=c99 \
         -Wall \
         -Wextra \
         -Og \
         -mdouble=24 \
         -mfloat=24 \
         -msummary=-psect,-class,+mem,-hex,-file

# Optimization flags
OPTFLAGS = --opt=default,+asm,+asmfile,-speed,+space,-debug

# Linker flags
LDFLAGS = -Wl,-Map=dist/output.map

# Source files
SOURCES = main.c \
          bsp.c \
          rtc_soft.c \
          solar_mgr.c \
          app_control.c

# Object files (derived from sources)
OBJECTS = $(SOURCES:.c=.p1)

# Output files
OUTPUT_DIR = dist
HEX_FILE = $(OUTPUT_DIR)/solar_light_controller.hex
ELF_FILE = $(OUTPUT_DIR)/solar_light_controller.elf

# Phony targets
.PHONY: all clean program test production help

# Default target
all: $(HEX_FILE)

# Create output directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Compile source files to object files
%.p1: %.c config.h
	$(CC) $(CFLAGS) $(OPTFLAGS) -c -o $@ $<

# Link object files to create ELF
$(ELF_FILE): $(OBJECTS) | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(ELF_FILE) $(OBJECTS)

# Create HEX file from ELF
$(HEX_FILE): $(ELF_FILE)
	$(OBJCOPY) -O ihex $(ELF_FILE) $(HEX_FILE)
	@echo "Build complete: $(HEX_FILE)"

# Clean build artifacts
clean:
	rm -f *.p1 *.d *.pre *.sym *.cmf *.cof *.hxl *.lst *.obj *.sdb *.rlf
	rm -rf $(OUTPUT_DIR)
	@echo "Clean complete"

# Program the device (adjust for your programmer)
program: $(HEX_FILE)
	@echo "Programming $(DEVICE)..."
	# Example for PICkit 4:
	# $(PROGRAMMER) -p $(HEX_FILE) -d $(DEVICE)
	@echo "Please use MPLAB X IPE or your programmer's tool to flash $(HEX_FILE)"

# Build with TEST_MODE enabled
test:
	@echo "Building in TEST MODE (24-second day)..."
	@grep -q "^#define TEST_MODE" config.h || (echo "ERROR: TEST_MODE not enabled in config.h" && exit 1)
	$(MAKE) clean
	$(MAKE) all

# Build with TEST_MODE disabled (production)
production:
	@echo "Building in PRODUCTION MODE (24-hour day)..."
	@grep -q "^//#define TEST_MODE" config.h || grep -q "^// #define TEST_MODE" config.h || (echo "WARNING: TEST_MODE may be enabled in config.h" && exit 1)
	$(MAKE) clean
	$(MAKE) all

# Display help
help:
	@echo "Solar-Synchronized Light Controller - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build the project (default)"
	@echo "  clean       - Remove all build artifacts"
	@echo "  program     - Flash the HEX file to device"
	@echo "  test        - Build in TEST MODE (24-second day)"
	@echo "  production  - Build in PRODUCTION MODE (24-hour day)"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build with current config.h settings"
	@echo "  make test         # Build for testing (verify TEST_MODE is enabled)"
	@echo "  make production   # Build for deployment (verify TEST_MODE is disabled)"
	@echo "  make clean all    # Clean rebuild"
	@echo ""
	@echo "Note: For MPLAB X IDE users, use the IDE's build system instead."

# Dependencies (auto-generated during compilation)
-include $(SOURCES:.c=.d)

# Additional targets for code quality

# Check for common issues
check:
	@echo "Checking for common issues..."
	@grep -n "TODO\|FIXME\|XXX" *.c *.h || echo "No TODOs found"
	@echo "Checking for division operators (avoid on PIC18)..."
	@grep -n "[^/]/[^/]" *.c || echo "No division operators found"

# Generate documentation (requires Doxygen)
docs:
	@which doxygen > /dev/null || (echo "Doxygen not found. Install with: sudo apt-get install doxygen" && exit 1)
	doxygen Doxyfile

# Count lines of code
count:
	@echo "Lines of code:"
	@wc -l *.c *.h
	@echo ""
	@echo "Total:"
	@cat *.c *.h | wc -l
