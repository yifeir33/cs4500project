CC=clang++
CFLAGS= -g -Wall -Wextra -Wpedantic -std=c++17 -pthread
AR=llvm-ar
ARFLAGS=rcs

network.a: server.o client.o connection.o server_connection.o
	$(AR) $(ARFLAGS) network.a server.o client.o server_connection.o 

netport.o: netport.h netport.cpp
	$(CC) $(CFLAGS) -c -o netport.o netport.cpp

server.o: server.h server.cpp server_connection.o object.h array.h helper.h socket_addr.h util.h
	$(CC) $(CFLAGS) -c -o server.o server.cpp

cts_connection.o: cts_connection.h cts_connection.o
	$(CC) $(CFLAGS) -c -o cts_connection.o cts_connection.cpp

ctc_connection.o: ctc_connection.h ctc_connection.o
	$(CC) $(CFLAGS) -c -o ctc_connection.o ctc_connection.cpp

connection.o: connection.h connection.o
	$(CC) $(CFLAGS) -c -o connection.o connection.cpp

server_connection.o: server_connection.cpp thread.h object.h array.h queue.h helper.h packet.h socket_addr.h connection.h
	$(CC) $(CFLAGS) -c -o server_connection.o server_connection.cpp 

client.o: client.h client.cpp helper.h array.h packet.h socket_addr.h connection.h ctc_connection.h cts_connection.h netport.h util.h
	$(CC) $(CFLAGS) -c -o client.o client.cpp

.PHONY: clean
clean:
	rm *.o
