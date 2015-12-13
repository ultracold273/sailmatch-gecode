OS			= $(shell uname)
CC			= g++
TAR			= sailmatch
CFLAGS		= -O3 -g -Wall
SRC			= $(TAR).cpp
ifeq ($(OS), Linux)
LIBS		= /usr/lib
INCS		= /usr/include
OBJ			= $(TAR).o
LIBRARY		= -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodegist -lgecodedriver
else
LIBS		= /Library/Frameworks
endif

ifeq ($(OS), Linux)
$(TAR): $(OBJ)
	$(CC) -o $(TAR) -L$(LIBS) $(OBJ) $(LIBRARY)
$(OBJ): $(SRC)
	$(CC) -std=gnu++11 -I$(INCS) -c $(SRC)
else
$(TAR): $(SRC)
	$(CC) $(CFLAGS) -F$(LIBS) -stdlib=libstdc++ -std=gnu++11 -framework gecode -o $(TAR) $(SRC)
endif

clean: $(TAR) $(OBJ)
	rm $(TAR) $(OBJ)
