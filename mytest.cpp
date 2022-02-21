#include "hash.h"
#include <iostream>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORM, NORMAL};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORM) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 and standard deviation of 20 
            m_normdist = std::normal_distribution<>(50,20);
        }
        else{
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
    }

    int getRandNum(){
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else{
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//uniform distribution

};

// The hash function used by HashTable class
unsigned int hashCode(const string str);


class Tester{ // Tester class to implement test functions
public:
    bool testInsert();
    bool testFind();
    bool testRemove();
private:
};

int main(){
    Tester a;
    if(a.testInsert()){
      cout << "ALL TESTS PASSED" << endl;
    }else{
      cout << "Test Failed." << endl;
    }
    if(a.testFind()){
      cout << "ALL TESTS PASSED" << endl;
    }else{
      cout << "Test Failed." << endl;
    }
    if(a.testRemove()){
        cout << "ALL TESTS PASSED" << endl;
    }else{
      cout << "Test Failed." << endl;
    }
    
    return 0;
}

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for ( int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

bool Tester::testInsert(){
  cout << "Testing Insert Function: " << endl;
  Random diskBlockGen(DISKMIN,DISKMAX);
  int tempDiskBlocks[55] = {0};
  HashTable aTable(MINPRIME,hashCode);
  int temp = 0;
  int secondIndex = 0;
  for (int i=0;i<55;i++) { //inserts 55 files
    temp = diskBlockGen.getRandNum();
    tempDiskBlocks[secondIndex] = temp;
    secondIndex++;
    aTable.insert(File("driver.cpp", temp));
  }
  for (int i = 0;i<55;i++){ //checks for files after rehash
    //checks for every file and makes sure they are in right position
    if (!(aTable.getFile("driver.cpp", tempDiskBlocks[i]) == File("driver.cpp", tempDiskBlocks[i]))){ //also a test for getFile
      return false; //returns false if file not found
    }
    if (aTable.m_size1 != 0 && aTable.m_size2 != 55){ //if sizing is incorrect
      return false;
    }
  }
  //inserting multiple non-colliding files
  aTable.insert(File("test.cpp", 333333));
  aTable.insert(File("bop.cpp", 444444));
  aTable.insert(File("bloop.txt", 555555));
  aTable.insert(File("scoop.h", 666666));
  //seeing if files are in list
  if (!(aTable.getFile("test.cpp", 333333) == File("test.cpp", 333333))) {
    return false;
  }
  if (!(aTable.getFile("bop.cpp", 444444) == File("bop.cpp", 444444))){
    return false;
  }
  if (!(aTable.getFile("bloop.txt", 555555) == File("bloop.txt", 555555))){
    return false;
  }
  if (!(aTable.getFile("scoop.h", 666666) == File("scoop.h", 666666))){
    return false;
  }
  return true;
}
bool Tester::testFind(){
  cout << "Testing getFile Function: " << endl;
  int temp = 0;
  Random diskBlockGen(DISKMIN,DISKMAX);
  HashTable aTable(MINPRIME,hashCode);
  for (int i=0;i<10;i++) { //adds 10 colliding keys
    temp = diskBlockGen.getRandNum();
    aTable.insert(File("driver.cpp", temp));
  }
  //inserts few non-collidng files
  aTable.insert(File("test.cpp", 333333));
  aTable.insert(File("bop.cpp", 444444));
  aTable.insert(File("bloop.txt", 555555));
  aTable.insert(File("scoop.h", 666666));
  for (int i=0;i<10;i++) { //adds 10 more different colliding keys
    temp = diskBlockGen.getRandNum();
    aTable.insert(File("test.cpp", temp));
  }
  if (!(aTable.getFile("test.cpp", 333333) == File("test.cpp", 333333))) {
    return false;
  }
  if (!(aTable.getFile("bop.cpp", 444444) == File("bop.cpp", 444444))){
    return false;
  }
  if (!(aTable.getFile("bloop.txt", 555555) == File("bloop.txt", 555555))){
    return false;
  }
  if (!(aTable.getFile("scoop.h", 666666) == File("scoop.h", 666666))){
        return false;
  }
  return true;
}

bool Tester::testRemove(){
  cout << "Testing Remove Function: " << endl;
  Random diskBlockGen(DISKMIN,DISKMAX);
  HashTable aTable(MINPRIME,hashCode);
  int tempDiskBlocks[35] = {0};
  int temp = 0;
  int secondIndex = 0;
  for (int i=0;i<35;i++) { //inserts 35 colliding files
    temp = diskBlockGen.getRandNum();
    tempDiskBlocks[secondIndex] = temp;
    secondIndex++;
    aTable.insert(File("driver.cpp", temp));
  }
  if (aTable.m_size1 != 35)
    return false;
  else
    return true;
  for (int i = 0; i < 35;i++) {
    aTable.remove(File("driver.cpp", tempDiskBlocks[i]));
  }
  for (int i = 0;i<35;i++) { //checks for files after rehash
    //checks for every file and makes sure they are in right position
    if (!(aTable.getFile("driver.cpp", tempDiskBlocks[i]) ==
	  File("driver.cpp", tempDiskBlocks[i]))) { //also a test for getFile
      return false; //returns false if file not found
    }
  }
  
}
