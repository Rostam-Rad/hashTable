// file.h
// Simple class for use with HashTable class

#ifndef FILE_H
#define FILE_H
#include <iostream>
#include <string>
using namespace std;
using std::string;
using std::ostream;
const int DISKMIN = 100000;
const int DISKMAX = 999999;
class File{
    public:
    File(string name="", unsigned int diskBlock=0);
    string key() const;
    unsigned int diskBlock() const;
    void setKey(string key);
    void setDiskBlock(unsigned int serial);
    const File& operator=(const File& rhs);
    // Overloaded insertion operator
    friend ostream& operator<<(ostream& sout, const File &file );
    // Overloaded equality operator
    friend bool operator==(const File& lhs, const File& rhs);

    private:
    // m_name is the key of a File object and it is used for indexing
    string m_name;
    // m_diskBlock specifies the uniquness of a File object
    // It can hold a value in the range of [DISKMIN-DISKMAX]
    unsigned int m_diskBlock;
};

#endif
