## Introduction:
eau2 system that we designed is a distributed key/value system could use for big data analysis. It contains a toolbox of machine learning algorithms
running on GPU demoed. It could load a data set in a CSV format (files over 100GB), then it allows the data analyst issue interactive queries.
The system is scalable and fast, it allows the query come back in a second.

## Architecture:

### network:
network part is going to deal with the client and server, by using multi-thread to communicate.

### client:
...

#### DataFrame:
This dataframe is built for clients to use to send as dataframe packages.
Dataframe is constructed by columns and rows. There is also a schema class which tells the data type
of each column. Dataframe also contains the functionality to edit row by row which is
called rower class. In each row, it could also be edited by inheritance the fielder class.


### KVstore:
Key-value store is where we used to store our data. The structure of KVStore is constructed by multiple nodes.
To find where the value is being stored, it requires a key to be given. And the key
could also tells which node it belongs to. KVstore could be used to editing, storing and retrieving.
(not sure i should import a graph here, but professor does have a similiar one in the video)


### SorerDataFrameAdapter
This parts could parse the data from other type of file into a dataframe, with the given Schema


## Implementation:
(professor notes)
difficult to understand, 
how network, how key value store
where we describe how the system is built, this can include a description of the classes and their API, but only the class you deem relevant and worth describing. (For example: do not describe the utility classes.)

### network
(will be edited later)

### KVstore
In our kvstore class, we use an unordered map to store the key and value.
(Value is currently a dataframe). And the Key contains a stirng name, and a size_t
representing which node to find the key.

### Dataframe
In Dataframe class, We stored three things: the schema object, all the columns and the column type.
To provide the basic functionalities of the dataframe

### Serializer & Deserializer
These two classes we used to convert from the data to the char*.

#### Serializer
Serializer is built by two fields, `buffer(char*)` and `position(size_t)`,
the class has many local functions use memcopy and shifting bits to write the input value to local `buffer`. For the types
it contains, it includes string, boolean, float, int, stringArray, floatArray and sockaddr_in.

#### Deserializer
Deserializer is converting the message of char* back to the above types, which includes all the types that Serilizer has.

### Applications
(so far there is only one application, Trivial)


#### Trivial 
This is a class only used for testing, which inputs an array of prmitive type.
And to make them to dataframe without doing any other functions.
(what is required for M2)

### SorerDataFrameAdapter
The implementation of this class contains a sorer that we used from other group (previous assignments). With that parser, and the file (also filename) that we were given, we want to return a Dataframe in a shared pointer and initialize the schema of the dataframe. And by having the dataframe, we can add row by row with a private method called `parse_and_fill_row()`, and while there is no more rows need to be parsed, it will return the dataframe.  

For   `initialize_schema()`, we take in the sorer, and iterate through column by column, and store the format in a string.


For `parse_and_fill_row()`, we implenented by using a switch cases, it iterates thrugh all the columns and get column type for each column, and in each primitive type case, there is a parse function for it.   
For boolean and integer, we used std::stoi to parse the value.  
For double, we used std:: stod to parse the value.  
For string, we copy construct a heap allocated the string to parse the string.  

## Use cases:
Here are some simple examples of how to use them, for more implementation, please look at our tests.

### DataFrame:
the methods under DataFrame:
fromArray(kvstore kv, key k, (primitive_type)* arr , size_t len)
```c++
        size_t SZ = 1000*1000;
        double* vals = new double[SZ];
        double sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        Key key("triv",0);
        DataFrame* df = DataFrame::fromArray(&key, &kv, SZ, vals);
```
assume we have a kvstore setup above, fromArray will store the double array to a dataframe,
under the given kvstore, with the `key("triv",0)`.

add_column(Column col, string name):  
this method will add the column to the current schema with the given name.

```c++
get_int(size_t col, size_t row);  
get_string(size_t col, size_t row); 
get_bool(size_t col, size_t row);  
get_double(size_t col, size_t row);  
```
these methods will return the value for that position at the place in the dataframe.
Wrong type,column or row out of bound is undefined....

```c++
set(Row r, Column c, int val);  
set(Row r, Column c, double val);  
set(Row r, Column c, bool val);  
set(Row r, Column c, string val);  
```
These methods will set the primitive type of value on that position.


```c++
get_col(string col);
get_row(string row);
```
These methods will return the col or row from the schema with the given name.


```c++
nrows();  
ncols();
```
return the number of rows and number of columns

```c++
add_rows(Row r);
```
add the row at the end of the dataframe.

```c++
fill_row(size_t idx, Row r);
```
set the idx's row with the provided row. If the type is wrong, results are undefined.


```c++
map(Rower r);
```
map a function by using Rower, and edit the dataframe row by row.

```c++
pmap(Rower r);
```
using multi-thread to do the map's work, and merge the results.

```c++
filter(Rower r);
```
The Rower gives a function to check if the data is proved by the condition, if it is not, the data will not be
used in the new dataframe.

```c++
print()
```
print the dataframe


### KVStore:
the methods under KVstore: 
```c++
get(Key k);  
put(Key k, Dataframe v);  
waitAndGet(Key k);
``` 
Asssume we are storing

Node1: k1,v1  
Node2: no key and value;  
Node3: k2,v2; k3, v3

if we are currently on node 1:  
get(k1) -> v1;  
put(k4,v4) will input the value under the node that home provided; 
Because `(k2,v2)` is not on the current node, if we want to get `v2`, we need to call `waitAndGet(k2)`
to get the value from Node3.

### Application (will also be editing later on)
The method that application should have:
```c++
run_();
this_node();
```
run_() will start to run each node;
this_node() could tell the index of the current node();

### Serializer & Deserializer
Here is an example of a simple primitive type use-case:
```c++
    float first = 3.9;
    bool second = true;
    int third = 1000;

    Serializer s;
    s.write((float)3.9);
    s.write((bool) true);
    s.write(1000);

    char* buf = s.get();
    size_t buf_len = s.length();

    Deserializer ds(buf, buf_len);

    t_true(std::abs(ds.readFloat() - first) < 0.00001);
    t_true(ds.readBool() == second);
    t_true(ds.readInt() == third);
```
for float, boolean, int, they all converted to a `char* buf` by using the imported `Serializer`,
and the char* allows the system to convert them back to their originial type, such as float, boolean and integer.
Serializer&Deserializer could also work for any customized class objects, such as the message.


for more information please take a look at our test cases.

### SorerDataframe Adapter
```c++
    std::shared_ptr<DataFrame> parse_file(const std::string& filename);
```
The input is the file name, and outputs a shared pointer dataframe, and what it does is it will initialize the schema and parse the file row by row.

#### notes from video
put time;
read a dataFrame for first week
examples of uses of the system. This could be in the form of code like the one above. It is okay to leave this section mostly empty if there is nothing to say. Maybe just an example of creating a dataframe would be enough.

## Open questions:
where you list things that you are not sure of and would like the answer to.

//need filereader?  
//what is the net in application?
// serializer?


## Status:   
Week 1:  
This week we set up the directories as required and wrote tests for key/value store.  
Week 2:  
Clean up the code for network, now network is working.
Implemented the KVStore on one node.
Adding a sorer functionality.
Wrote a FromArray inside columns, and the tests for this part.
Catching up the report.
added sorer from another group
used sorer to read dataframe and wrote tests
use the optional to handle primitive type with null
