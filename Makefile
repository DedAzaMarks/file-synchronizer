CC = g++
CFLAGS = -Wall -Werror -Wextra -std=c++17 
SANITIZE = -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
LIBS = -lPocoNet -lPocoFoundation

RELEASE = ${CC} -O3 ${CFLAGS}
DEBUG = ${CC} -O0 -g ${CFALGS} ${SANITIZE} 

all: sync_release server_release

sync_release: FSyncClient.cpp 
	${RELEASE} FSyncClient.cpp ${LIBS} -o sync

sync_debug: FSyncClient.cpp
	${DEBUG} FSyncClient.cpp ${LIBS} -o sync

server_release: FSyncServer.cpp
	${RELEASE} FSyncServer.cpp ${LIBS} -o server

server_debug: FSyncServer.cpp
	${DEBUG} FSyncServer.cpp ${LIBS} -o server

