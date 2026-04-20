CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = main
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
	rm -f *.o
    @echo "Build complete! Run with: ./$(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build files"

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET)"

debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "Debug build complete"

run: $(TARGET)
	./$(TARGET)

check:
	@echo "Checking dependencies..."
	@which gcc >/dev/null || echo "ERROR: gcc not found"
	@ls main_pages/ >/dev/null 2>&1 || echo "WARNING: main_pages/ directory not found"
	@ls movies/ >/dev/null 2>&1 || echo "WARNING: movies/ directory not found" 
	@ls television/ >/dev/null 2>&1 || echo "WARNING: television/ directory not found"
	@echo "Check complete"


help:
	@echo "Available targets:"
	@echo "  all       - Build the video server (default)"
	@echo "  clean     - Remove build files"
	@echo "  debug     - Build with debug symbols"
	@echo "  run       - Build and run the server"
	@echo "  install   - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  check     - Check for dependencies and directories"
	@echo "  help      - Show this help"


.PHONY: all clean install uninstall debug run check help
