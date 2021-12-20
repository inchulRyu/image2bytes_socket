APP:= img2bytes

CC=g++

SRCS:= clinet.cpp server.cpp

PKGS:= opencv4

# CFLAGS+=$(shell pkg-config --cflags $(PKGS))
# LIBS+=$(shell pkg-config --libs $(PKGS))

# OBJS:= $(SRCS:.cpp=.o)

CFLAGS+= -g -Wall -ansi

all: clientsocket serversocket

clientsocket: clientsocket.cpp
	$(CC) clientsocket.cpp -o clientsocket $(shell pkg-config --cflags --libs $(PKGS))
serversocket: serversocket.cpp
	$(CC) serversocket.cpp -o serversocket $(CFLAGS) $(shell pkg-config --cflags --libs $(PKGS)) -lpthread -std=c++11

clean:
	rm -f clientsocket serversocket