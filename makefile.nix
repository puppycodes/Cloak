###############################################################################
#                                                                             #
# MAKEFILE for Cloak application                                              #
#                                                                             #
# � Guy Wilson 2012                                                           #
#                                                                             #
# Build using Visual Studio C++                                               #
# Use 'nmake' to build with this makefile, 'nmake -a' to rebuild all          #
#                                                                             #
###############################################################################

# Source directory
SOURCE=src

# Build output directory
BUILD=build

# What is our target
TARGET=cloak

# C++ compiler
CPP=g++
C=gcc

# Linker
LINKER=g++

# C++ compiler flags
CPPFLAGS=-c -fpermissive -Wall
CFLAGS=-c -Wall

# Object files (in linker ',' seperated format)
OBJFILES=$(BUILD)/main.o $(BUILD)/md5.o $(BUILD)/exception.o $(BUILD)/iterator.o $(BUILD)/image.o $(BUILD)/data.o $(BUILD)/encryption.o $(BUILD)/cloak.o $(BUILD)/pngreadwrite.o $(BUILD)/aes.o $(BUILD)/memdebug.o

# Target
all: $(TARGET)

# Compile C++ source files
#
$(BUILD)/md5.o: $(SOURCE)/md5.cpp $(SOURCE)/md5.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/md5.o $(SOURCE)/md5.cpp

$(BUILD)/exception.o: $(SOURCE)/exception.cpp $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/exception.o $(SOURCE)/exception.cpp

$(BUILD)/iterator.o: $(SOURCE)/iterator.cpp $(SOURCE)/iterator.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/iterator.o $(SOURCE)/iterator.cpp

$(BUILD)/image.o: $(SOURCE)/image.cpp $(SOURCE)/image.h $(SOURCE)/iterator.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h $(SOURCE)/pngreadwrite.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/image.o $(SOURCE)/image.cpp
	
$(BUILD)/data.o: $(SOURCE)/data.cpp $(SOURCE)/data.h $(SOURCE)/iterator.h $(SOURCE)/encryption.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/data.o $(SOURCE)/data.cpp
	
$(BUILD)/encryption.o: $(SOURCE)/encryption.cpp $(SOURCE)/encryption.h $(SOURCE)/md5.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h $(SOURCE)/salt.h $(SOURCE)/key.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/encryption.o $(SOURCE)/encryption.cpp

$(BUILD)/cloak.o: $(SOURCE)/cloak.cpp $(SOURCE)/cloak.h $(SOURCE)/image.h $(SOURCE)/data.h $(SOURCE)/iterator.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/cloak.o $(SOURCE)/cloak.cpp

$(BUILD)/main.o: $(SOURCE)/main.cpp $(SOURCE)/cloak.h $(SOURCE)/image.h $(SOURCE)/data.h $(SOURCE)/iterator.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/main.o $(SOURCE)/main.cpp

$(BUILD)/aes.o: $(SOURCE)/aes.c $(SOURCE)/aes.h
	$(C) $(CFLAGS) -o $(BUILD)/aes.o $(SOURCE)/aes.c

$(BUILD)/memdebug.o: $(SOURCE)/memdebug.c $(SOURCE)/memdebug.h $(SOURCE)/types.h
	$(C) $(CFLAGS) -o $(BUILD)/memdebug.o $(SOURCE)/memdebug.c

$(BUILD)/pngreadwrite.o: $(SOURCE)/pngreadwrite.c $(SOURCE)/pngreadwrite.h $(SOURCE)/writepng.h
	$(C) $(CFLAGS) -I/usr/local/include -o $(BUILD)/pngreadwrite.o $(SOURCE)/pngreadwrite.c

$(TARGET): $(OBJFILES)
	$(LINKER) -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu -lstdc++ -o $(TARGET) $(OBJFILES) -lpng -lz -lbsd
