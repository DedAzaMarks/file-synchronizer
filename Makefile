CC = g++
CFLAGS = -Wall -Werror -Wextra -std=c++17 
SANITIZE = -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
LIBS = -lPocoNet -lPocoFoundation

RELEASE = ${CC} -O3 ${CFLAGS}
DEBUG = ${CC} -O0 -g ${CFALGS} ${SANITIZE} 

sync_release: FSyncClient.cpp 
	${RELEASE} FSyncClient.cpp ${LIBS} -o sync

server_release: FSyncServer.cpp
	${RELEASE} FSyncServer.cpp ${LIBS} -o server

# functions.o: functions.cpp
# 	${RELEASE} -c functions.cpp -lPocoNet
# 
# Types.o: Types.cpp
# 	${RELEASE} -c Types.cpp -lPocoFoundations
# 
# Hasher.o: Hasher.cpp Types.o
# 	${RELEASE} -c Hasher.cpp Types.o
