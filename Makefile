CC=g++
CFLAGS=-Wall -Werror -Wextra -std=c++17
SANITIZE=-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
RELEASE=-O3
DEBUG=-O0 -g
LIBS=-lPocoNet -lPocoFoundation

all: server_release sync_release

server_release: tcp_server.cpp
	${CC} ${RELEASE} ${CFLAGS} ${SANITIZE} tcp_server.cpp ${LIBS} -o server

server_debug: tcp_server.cpp
	${CC} ${DEBUG} ${CFLAGS} ${SANITIZE} tcp_server.cpp ${LIBS} -o server

sync_release: send_to_server.cpp
	${CC} ${RELEASE} ${CFLAGS} send_to_server.cpp ${LIBS} -o sync

sync_debug: send_to_server.cpp
	${CC} ${DEBUG} ${CFLAGS} send_to_server.cpp ${LIBS} -o sync
