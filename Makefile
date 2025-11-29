CPP = g++
CPPFLAGS = -std=c++17 -Wall -O2 -g

all: create-server create-client


create-server: server/src/server.cpp
	@mkdir -p bin
	$(CPP) $(CPPFLAGS) -o bin/server-MNV server/src/*.cpp

create-client: client/src/client.cpp
	@mkdir -p bin
	$(CPP) $(CPPFLAGS) -o bin/client-MNV client/src/*.cpp

clean:
	rm -rf bin

.PHONY: all clean
