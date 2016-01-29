ifeq ($(OS),Windows_NT)
	RM := del
	TARGET = DungeonCrawler.exe
	LIBS =
	DEFINES =
	FixPath = $(subst /,\,$1)
else
	RM := rm -f
	TARGET = DungeonCrawler
	LIBS =
	DEFINES = 
	FixPath = $1
endif

SFML_DIR = $(HOME)/git/SFML

INC = -I$(SFML_DIR)/include
LIBS += -L$(SFML_DIR)/lib -lsfml-audio -lsfml-graphics -lsfml-system -lsfml-window -llua

CC = g++
FLAGS = -std=c++0x
CFLAGS = -Wall $(INC)

SRC = $(wildcard *.cpp) $(wildcard */*.cpp)

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

clean:
	$(foreach file,$(OBJ),$(RM) $(call FixPath,$(file);))
	$(RM) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(DEFINES) $(FLAGS) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)
.cpp.o:
	$(CC) $(DEFINES) $(FLAGS) $(CFLAGS) -o $@ -c $<
