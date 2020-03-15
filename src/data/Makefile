CC=clang++
CFLAGS=-Wall -Wextra -Wpedantic -g -std=c++17 -I../
AR=llvm-ar
AFLAGS=rcs

.PHONY: clean

# Create Static Library
data.a: dataframe.o fielder.o row.o schema.o
	$(AR) $(AFLAGS) data.a dataframe.o fielder.o row.o schema.o

dataframe.o: dataframe.h dataframe.cpp rower.h fielder.o row.o 
	$(CC) $(CFLAGS) -c -o dataframe.o

fielder.o: fielder.h fielder.cpp
	$(CC) $(CFLAGS) -c -o fielder.o fielder.cpp

row.o: row.h row.cpp
	$(CC) $(CFLAGS) -c -o row.o row.cpp

schema.o: schema.h schema.cpp column.o
	$(CC) $(CFLAGS) -c -o schema.o schema.cpp

clean:
	rm *.o data.a
