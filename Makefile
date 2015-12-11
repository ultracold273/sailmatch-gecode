CC			= g++
TAR			= sailmatch
CFLAGS		= -O3 -g -Wall
SRC			= $(TAR).cpp
LIBS		= /Library/Frameworks

$(TAR): $(SRC)
	$(CC) $(CFLAGS) -F$(LIBS) -stdlib=libstdc++ -framework gecode -o $(TAR) $(SRC)
