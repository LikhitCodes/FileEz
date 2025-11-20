# P2P File Sharing System Makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
DEBUG_FLAGS = -g -DDEBUG -O0
TARGET = p2p_share
SRCDIR = src
INCDIR = include
OBJDIR = obj
DATADIR = data

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    #LDFLAGS = -lws2_32
    # Uncomment next line for static linking (no DLLs needed):
    LDFLAGS = -static -lws2_32 -static-libgcc -static-libstdc++
    RM = del /Q
    MKDIR = mkdir
    TARGET := $(TARGET).exe
else
    LDFLAGS = -lpthread
    RM = rm -f
    MKDIR = mkdir -p
endif

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Default target
all: directories $(TARGET)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: directories $(TARGET)

# Release build  
release: CXXFLAGS += -DNDEBUG
release: directories $(TARGET)

# Create necessary directories
directories:
	@if not exist $(OBJDIR) $(MKDIR) $(OBJDIR)
	@if not exist $(DATADIR)\shared $(MKDIR) $(DATADIR)\shared
	@if not exist $(DATADIR)\chunks $(MKDIR) $(DATADIR)\chunks
	@if not exist $(DATADIR)\downloads $(MKDIR) $(DATADIR)\downloads

# Link target
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Individual module builds for testing
main: $(OBJDIR)/main.o
	$(CXX) $(OBJDIR)/main.o -o test_main $(LDFLAGS)

server: $(OBJDIR)/server.o $(OBJDIR)/network_utils.o $(OBJDIR)/peer.o $(OBJDIR)/file_utils.o
	$(CXX) $^ -o test_server $(LDFLAGS)

client: $(OBJDIR)/client.o $(OBJDIR)/network_utils.o $(OBJDIR)/peer.o $(OBJDIR)/file_utils.o
	$(CXX) $^ -o test_client $(LDFLAGS)

# Network test program
test-network: test_network.cpp $(OBJDIR)/network_utils.o $(OBJDIR)/peer.o $(OBJDIR)/file_utils.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) test_network.cpp $(OBJDIR)/network_utils.o $(OBJDIR)/peer.o $(OBJDIR)/file_utils.o -o test_network.exe $(LDFLAGS)
	@echo "Network test program built: test_network.exe"

# File management test program
test-files: test_file_management.cpp $(OBJDIR)/network_utils.o $(OBJDIR)/peer.o $(OBJDIR)/file_utils.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) test_file_management.cpp $(OBJDIR)/network_utils.o $(OBJDIR)/peer.o $(OBJDIR)/file_utils.o -o test_files.exe $(LDFLAGS)
	@echo "File management test program built: test_files.exe"

# Install target (copy to system path)
install: $(TARGET)
	@echo "Installing $(TARGET)..."
	@copy $(TARGET) C:\Windows\System32\ 2>nul || cp $(TARGET) /usr/local/bin/ || echo "Install failed - run as administrator"

# Clean build artifacts
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	@if exist $(TARGET) del /q $(TARGET)
	@if exist test_* del /q test_*
else
	$(RM) -r $(OBJDIR) $(TARGET) test_*
endif
	@echo "Clean complete"

# Clean all generated files including data
clean-all: clean
ifeq ($(OS),Windows_NT)
	@if exist $(DATADIR)\chunks rmdir /s /q $(DATADIR)\chunks
	@if exist $(DATADIR)\downloads rmdir /s /q $(DATADIR)\downloads
	@$(MKDIR) $(DATADIR)\chunks $(DATADIR)\downloads 2>nul || true
else
	$(RM) -r $(DATADIR)/chunks/* $(DATADIR)/downloads/*
endif
	@echo "All data cleaned"

# Run targets
run-share: $(TARGET)
	./$(TARGET) --mode=share --port=8080

run-download: $(TARGET)
	./$(TARGET) --mode=download --port=8081

run-hybrid: $(TARGET)
	./$(TARGET) --mode=hybrid --port=8082

# Development helpers
format:
	@echo "Formatting code..."
	@clang-format -i $(SRCDIR)/*.cpp $(INCDIR)/*.h 2>nul || echo "clang-format not available"

check-syntax:
	@echo "Checking syntax..."
	@$(CXX) $(CXXFLAGS) -I$(INCDIR) -fsyntax-only $(SOURCES)

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build the complete application (default)"
	@echo "  debug        - Build with debug symbols and no optimization"
	@echo "  release      - Build optimized release version"
	@echo "  clean        - Remove build artifacts"
	@echo "  clean-all    - Remove build artifacts and data files"
	@echo "  install      - Install to system path"
	@echo "  run-share    - Run in share mode"
	@echo "  run-download - Run in download mode"
	@echo "  run-hybrid   - Run in hybrid mode"
	@echo "  format       - Format source code"
	@echo "  check-syntax - Check code syntax"
	@echo "  help         - Show this help message"

# Phony targets
.PHONY: all debug release directories clean clean-all install run-share run-download run-hybrid format check-syntax help