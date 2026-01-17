# Probabilistic Call Stack PoC - Makefile
# Supports both MSVC (nmake) and MinGW (make)

# Output binary name
TARGET = probabilistic_callstack.exe
SOURCE = probabilistic_callstack.cpp

# Detect compiler
ifdef VSINSTALLDIR
    # MSVC detected (run from Developer Command Prompt)
    CC = cl
    CFLAGS = /EHsc /O2 /W3 /nologo
    LDFLAGS = /link user32.lib dbghelp.lib
    RM = del /Q

    all: msvc

    msvc:
    	$(CC) $(CFLAGS) $(SOURCE) /Fe:$(TARGET) $(LDFLAGS)

    debug:
    	$(CC) /EHsc /Zi /Od /W4 /nologo $(SOURCE) /Fe:$(TARGET) $(LDFLAGS) /DEBUG
else
    # MinGW/GCC
    CC = g++
    CFLAGS = -O2 -Wall -static
    LDFLAGS = -luser32 -ldbghelp
    RM = rm -f

    all: mingw

    mingw:
    	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

    debug:
    	$(CC) -g -O0 -Wall $(SOURCE) -o $(TARGET) $(LDFLAGS)
endif

clean:
	$(RM) $(TARGET) *.obj *.pdb *.ilk

run: all
	./$(TARGET)

.PHONY: all clean run debug msvc mingw
