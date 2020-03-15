CC=clang++
CFLAGS=-Weverything -g -std=c++17
AR=llvm-ar
AFLAGS=rcs

.PHONY: clean

# Create Static Library
# TODO: different name + create header for libaray
dataframe.a: dataframe.o fielder.o row.o schema.o
	$(AR) $(AFLAGS) dataframe.a fielder.o row.o schema.o

dataframe.o: dataframe.h dataframe.cpp rower.h fielder.o row.o 
	$(CC) $(CFLAGS) -c -o dataframe.o dataframe.cpp fielder.o row.o

fielder.o: fielder.h fielder.cpp object.h
	$(CC) $(CFLAGS) -c -o fielder.o fielder.cpp

row.o: row.h row.cpp object.h
	$(CC) $(CFLAGS) -c -o row.o row.cpp

schema.o: schema.h schema.cpp object.h column.o
	$(CC) $(CFLAGS) -c -o schema.o schema.cpp column.o

clean:
	rm *.o dataframe.a
