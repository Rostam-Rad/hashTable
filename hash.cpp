// CMSC 341 - Fall 2021 - Project 4
#include "hash.h"
#include "math.h"
HashTable::HashTable(unsigned size, hash_fn hash){
  //sets capacity of first table
  if (isPrime(size) && (size >= MINPRIME) && (size <= MAXPRIME)){ //makes sure size is valid
    m_capacity1 = size;
  }else if (size < MINPRIME){
    size = MINPRIME;
    m_capacity1 = size;
  }else if(size > MAXPRIME){
    size = MAXPRIME;
    m_capacity1 = size;
  }else if(!isPrime(size)){
    size = findNextPrime(size);
    m_capacity1 = size;
  }
  m_hash = hash;
  m_size1 = 0;
  m_numDeleted1 = 0;
  m_capacity2 = 0;
  m_size2 = 0;
  m_numDeleted2 = 0;
  m_table1 = new File[m_capacity1];
  m_table2 = nullptr;
  m_newTable = TABLE1;
  isRehashing = false;
  resizeInc = 0.25; //resize increment
}

HashTable::~HashTable(){
  delete [] m_table1;
  delete [] m_table2;
}

File HashTable::getFile(string name, int diskBlock){
  unsigned int index; //stores index value
  if (m_newTable == TABLE1) { //look through table1
    index = m_hash(name) % m_capacity1;
    int i = 0; //used for quadratic probing
    while (m_table1[index].diskBlock() != diskBlock) { //if file is not found
      index = ((m_hash(name) % m_capacity1) + (i * i)) % m_capacity1; //uses quad probing to find
      i += 1;
    }
    if (m_table1[index].key() == name && m_table1[index].diskBlock() == diskBlock) { //if the name and diskblock match
      return m_table1[index]; //return the found file
    } else { //if file was not found
      if (isRehashing) { //if rehashing is happening check other table
	index = m_hash(name) % m_capacity2;
	int i = 0;
	while (m_table2[index].diskBlock() != diskBlock) {
	  index = ((m_hash(name) % m_capacity2) + (i * i)) % m_capacity2;
	  i += 1;
	}
	if (m_table2[index].key() == name && m_table2[index].diskBlock() == diskBlock) {
	  return m_table2[index];
	  
	} else {
	  return EMPTY; //if not found return empty
	}
      } else {
	return EMPTY;//if not found return empty
      }
    }
  } else { //if we are looking at table2
    index = m_hash(name) % m_capacity2;
    int i = 0;
    while (m_table2[index].diskBlock() != diskBlock) {
      index = ((m_hash(name) % m_capacity2) + (i * i)) % m_capacity2;
      i += 1;
    }
    if (m_table2[index].key() == name && m_table2[index].diskBlock() == diskBlock) {
      return m_table2[index]; //return found file
      
    } else {
      if (isRehashing) { //we also have to check table1 if rehashing
	index = m_hash(name) % m_capacity1;
	int i = 0;
	while (m_table1[index].diskBlock() != diskBlock) {
	  index = ((m_hash(name) % m_capacity1) + (i * i)) % m_capacity1;
	  i += 1;
	}
	if (m_table1[index].key() == name && m_table1[index].diskBlock() == diskBlock) {
	  return m_table1[index];
	} else {
	  return EMPTY; //if not found return empty
	}
      } else {
	return EMPTY; //if not found return empty
      }
    }
  }
  
}

bool HashTable::insert(File file){
  unsigned int index; //used to store index value
  bool inserted = false; //tracks if file is inserted
  if (m_newTable == TABLE1) { //if we are inserting in table1
    index = m_hash(file.key()) % m_capacity1; //hashcode
    int i = 0;
    while(!(m_table1[index].key().empty()) && m_table1->diskBlock() != file.diskBlock() && m_table1[index].key() != DELETEDKEY){ //looks for open slot
      index = ((m_hash(file.key()) % m_capacity1) + (i * i)) % m_capacity1; //collision handling
      i += 1;
    }
    if (m_table1[index] == EMPTY || m_table1[index] == DELETED){ //inserts if open
      m_table1[index] = File(file.key(), file.diskBlock());
      m_size1++;
      inserted = true; //returns true later
    }else {
      cout << file.key() << " : " << file.diskBlock() << "not inserted" << endl;
    }
    
  }else { //if we are inserting in table2
    index = m_hash(file.key()) % m_capacity2; //hashcode
    int i = 0;
    while (!(m_table2[index] == EMPTY) && !(m_table2[index] == DELETED)) { //looks for slot
      index = ((m_hash(file.key()) % m_capacity2) + (i * i)) % m_capacity2; //collision handling
      i += 1;
    }
    if (m_table2[index] == EMPTY || m_table2[index] == DELETED) { //inserting
      m_table2[index] = File(file.key(), file.diskBlock()); //sets index
      m_size2++; //increase size
      inserted = true;
    } else {
      cout << file.key() << " : " << file.diskBlock() << "not inserted" << endl;
    }
  }
  if (lambda(m_newTable) > 0.5){ //needs rehashing
    //cout << "needs rehashing!" << endl;
    isRehashing = true; //lets program know rehashing is in progress
    if (m_newTable == TABLE1){ //changes tables
      m_newTable = TABLE2;
      m_capacity2 = findNextPrime((m_size1 - m_numDeleted1) * 4); //sets new capacity
      m_table2 = new File[m_capacity2]; //allocated memory to new table
    }else{
      m_newTable = TABLE1;
      m_capacity1 = findNextPrime((m_size2 - m_numDeleted2) * 4); //sets new capacity
      m_table1 = new File[m_capacity1];
    }
  }
  if (isRehashing){ //rehash is in progress
    rehash(resizeInc); //transfers 25%
    //cout << "REHASHED " << endl;
    //cout << "m_size1 = " << m_size1 << " m_size 2 = " << m_size2 << endl;
    if (resizeInc <= .75){
      resizeInc += .25; //increments 25%
    }else{ //if completely transfered
      if (m_newTable == TABLE1) { //changes tables
	m_size2 = 0; //sets size for empty table
	m_capacity2 = 0;
	m_numDeleted2 = 0; //resets table value
      }else{
	m_size1 = 0; //sets size for empty table
	m_capacity1 = 0;
	m_numDeleted1 = 0;
      }
      resizeInc = .25; //resets resize increment to 25%
    }
  }
  return inserted; //return true if inserted false otherwise
}

bool HashTable::remove(File file){
  unsigned int index;
  unsigned int index2;
  bool removed = false; //removed status
  if (isRehashing){
    //cout << " removing " << file.key() << endl;
    index = m_hash(file.key()) % m_capacity1;
    index2 = m_hash(file.key()) % m_capacity2;
    int i = 0;
    while ((m_table1[index].diskBlock() != file.diskBlock()) && (m_table2[index2].diskBlock() != file.diskBlock())) { //finding file
      //cout << "index = " << index << " index2 = " << index2 << endl;
      index = ((m_hash(file.key()) % m_capacity1) + (i * i)) % m_capacity1; //quad probing until index matches key
      index2 = ((m_hash(file.key()) % m_capacity2) + (i * i)) % m_capacity2;
      i += 1;
    }
    if (m_table1[index].diskBlock() == file.diskBlock()) { //if file is found
      m_table1[index] = DELETED; //deletes file
      m_numDeleted1++;
      removed = true;
    }
    if (m_table2[index2].diskBlock() == file.diskBlock()) { //if file found
      m_table2[index2] = DELETED; //deletes file
      m_numDeleted2++;
      removed = true;
    }
  }else {
    if (m_newTable == TABLE1) { //if we are removing from table1
      index = m_hash(file.key()) % m_capacity1;
      int i = 0;
      while (m_table1[index].diskBlock() != file.diskBlock()) { //finding file
	index = ((m_hash(file.key()) % m_capacity1) + (i * i)) %
	  m_capacity1; //quad probing until index matches key
	i += 1;
      }
      if (m_table1[index].key() == file.key() &&
	  m_table1[index].diskBlock() == file.diskBlock()) { //if file is found
	m_table1[index] = DELETED; //deletes file
	m_numDeleted1++;
	removed = true;
      } else { //if file not found in table1
	cout << file.key() << " : " << file.diskBlock() << "not found2" << endl;
      }
    } else { //if we are removing from table2
      index = m_hash(file.key()) % m_capacity2;
      int i = 0;
      while (m_table2[index].diskBlock() != file.diskBlock()) { //looking for file
	index = ((m_hash(file.key()) % m_capacity2) + (i * i)) % m_capacity2;
	i += 1;
      }
      if (m_table2[index].key() == file.key() &&
	  m_table2[index].diskBlock() == file.diskBlock()) { //if file found
	m_table2[index] = DELETED; //deletes file
	m_numDeleted2++;
	removed = true;
      } else {
	cout << file.key() << " : " << file.diskBlock() << "not found4" << endl;
      }
    }
  }
  
  if (isRehashing){ //rehash is in progress
    rehash(resizeInc); //transfers 25%
    //cout << "REHASHED " << endl;
    //cout << "m_size1 = " << m_size1 << " m_size 2 = " << m_size2 <<  " raiseInce = " << resizeInc<< endl;
    if (resizeInc <= .75){
      resizeInc += .25; //increments 25%
    }else{ //if completely transfered
      if (m_newTable  == TABLE1) { //changes tables
	m_size2 = 0; //sets size for empty table
	m_capacity2 = 0;
	m_numDeleted2 = 0; //resets table value
      }else{
	m_size1 = 0; //sets size for empty table
	m_capacity1 = 0;
	m_numDeleted1 = 0;
      }
      resizeInc = .25; //resets resize increment to 25%
    }
  }else{
    if (deletedRatio(m_newTable) > .8) { //rehash
      //cout << "deleted needs rehashing" << endl;
      isRehashing = true; //lets program know rehashing is in progress
      if (m_newTable == TABLE1){ //changes tables
	m_newTable = TABLE2;
	m_capacity2 = findNextPrime((m_size1 - m_numDeleted1) * 4); //sets new capacity
	m_table2 = new File[m_capacity2]; //allocated memory to new table
      }else{
	m_newTable = TABLE1;
	m_capacity1 = findNextPrime((m_size2 - m_numDeleted2) * 4); //sets new capacity
	m_table1 = new File[m_capacity1];
      }
      rehash(resizeInc);
      resizeInc += .25;
    }
    
  }
  return removed; //return true is file was successfully removed    
}

void HashTable::rehash(double inc) {
  //cout << "INC = " << inc << endl;
  if (m_newTable == TABLE1) { //if we are inserting into TABLE1
    unsigned int index;
    if (inc == .25) { //transfers first 25%
      //cout << "In .25" << endl;
      int twoFive = m_capacity2 * inc; //finds 25% of list to search
      for (int i = 0; i < twoFive; i++) { //loops through 25%
	if ((!(m_table2[i] == EMPTY)) || (!(m_table2[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table2[i].key()) % m_capacity1;
	  int j = 0;
	  while (!(m_table1[index] == EMPTY) && !(m_table1[index] == DELETED)) { //looking for empty index
	    index = ((m_hash(m_table1[i].key()) % m_capacity1) + (j * j)) % m_capacity1;
	    j++;
	  }
	  if ((m_table1[index] == EMPTY || m_table1[index] == DELETED) && (!(m_table2[i] == EMPTY))) {
	    m_table1[index] = File(m_table2[i].key(), m_table2[i].diskBlock()); //inserts into empty index
	    m_size1++; //increments size
	    m_size2--;
	  }
	  m_table2[i] = EMPTY; //removes file from original table
	}
      }
    } else if (inc == .5) { //transfers next 25% (50% total)
      //cout << "In .5" << endl;
      int twoFive = m_capacity2 * (inc - 0.25); //25
      int fiveOne = m_capacity2 * inc; //50
      for (int i = twoFive; i < fiveOne; i++) { //loop through secon quarter of table
	if ((!(m_table2[i] == EMPTY)) || (!(m_table2[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table2[i].key()) % m_capacity1; //hashcode
	  int j = 0;
	  while (!(m_table1[index] == EMPTY) && !(m_table1[index] == DELETED)) { //finds empty index
	    index = ((m_hash(m_table2[i].key()) % m_capacity1) + (j * j)) % m_capacity1; //quad probing to find empty slot
	    j++;
	  }
	  if ((m_table1[index] == EMPTY || m_table1[index] == DELETED) && (!(m_table2[i] == EMPTY))) { //inserts into empty slot
	    m_table1[index] = File(m_table2[i].key(), m_table2[i].diskBlock()); //sets index
	    m_size1++; //increment size
	    m_size2--;
	  }
	  m_table2[i] = EMPTY; //empties file from old table
	}
      }
      
    } else if (inc == .75) { //fills table to 75%
      //cout << "in .75" << endl;
      int fiveOne = m_capacity2 * (inc - 0.25);
      int sevFive = m_capacity2 * inc;
      for (int i = fiveOne; i < sevFive; i++) {
	if ((!(m_table2[i] == EMPTY)) || (!(m_table2[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table2[i].key()) % m_capacity1;
	  int j = 0;
	  while (!(m_table1[index] == EMPTY) && !(m_table1[index] == DELETED)) {
	    index = ((m_hash(m_table2[i].key()) % m_capacity1) + (j * j)) % m_capacity1;
	    j++;
	  }
	  if ((m_table1[index] == EMPTY || m_table1[index] == DELETED) && (!(m_table2[i] == EMPTY))) { //inserts into open slots
	    m_table1[index] = File(m_table2[i].key(), m_table2[i].diskBlock()); //inserts into empty index
	    m_size1++; //increment size
	    m_size2--;
	  }
	  m_table2[i] = EMPTY; //empty old table
	}
      }
      
    } else if (inc == 1) { //transferring last 25% of files
      //cout << "in 1" << endl;
      int sevFive = m_capacity2 * (inc - 0.25); //75
      int hundo = m_capacity2 * inc; //to 100
      for (int i = sevFive; i < hundo; i++) { //loops throgh last 25% of old table
	if ((!(m_table2[i] == EMPTY)) || (!(m_table2[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table2[i].key()) % m_capacity1;
	  int j = 0;
	  while (!(m_table1[index] == EMPTY) && !(m_table1[index] == DELETED)) { //if index is empty
	    index = ((m_hash(m_table2[i].key()) % m_capacity1) + (j * j)) % m_capacity1;
	    j++;
	  }
	  if ((m_table1[index] == EMPTY || m_table1[index] == DELETED) && (!(m_table2[i] == EMPTY))) { //transfer file
	    m_table1[index] = File(m_table2[i].key(), m_table2[i].diskBlock()); //sets index
	    m_size1++; //+size
	    m_size2--;
	  }
	  m_table2[i] = EMPTY; //sets file on old table to empty
	}
      }
      isRehashing = false; //rehashing is finished
    } else {
      cout << "Something is wrong with rehash?" << endl; //should never reach here
    }
    
  }else { //if we are inserting into TABLE2
    unsigned int index;
    if (inc == .25) {
      //cout << "In .25" << endl;
      int twoFive = m_capacity1 * inc; //finds 25% of list to search
      for (int i = 0; i < twoFive; i++) { //loops through 25%
	if ((!(m_table1[i] == EMPTY)) || (!(m_table1[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table1[i].key()) % m_capacity2;
	  int j = 0;
	  while (!(m_table2[index] == EMPTY) && !(m_table2[index] == DELETED)) {
	    index = ((m_hash(m_table1[i].key()) % m_capacity2) + (j * j)) % m_capacity2;
	    j++;
	  }
	  if ((m_table2[index] == EMPTY || m_table2[index] == DELETED) && (!(m_table1[i] == EMPTY))) {
	    m_table2[index] = File(m_table1[i].key(), m_table1[i].diskBlock()); //inserts into empty index
	    m_size2++; //increments size
	    m_size1--;
	  }
	  m_table1[i] = EMPTY; //sets file on old table to empty
	}
      }
      
    } else if (inc == .5) {
      //cout << "In .5" << endl;
      int twoFive = m_capacity1 * (inc - 0.25);
      int fiveOne = m_capacity1 * inc;
      for (int i = twoFive; i < fiveOne; i++) {
	if ((!(m_table1[i] == EMPTY)) || (!(m_table1[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table1[i].key()) % m_capacity2;
	  int j = 0;
	  while (!(m_table2[index] == EMPTY) && !(m_table2[index] == DELETED)) {
	    index = ((m_hash(m_table1[i].key()) % m_capacity2) + (j * j)) % m_capacity2;
	    j++;
	  }
	  if ((m_table2[index] == EMPTY || m_table2[index] == DELETED) && (!(m_table1[i] == EMPTY))) {
	    m_table2[index] = File(m_table1[i].key(), m_table1[i].diskBlock()); //inserts into empty index
	    m_size2++; //increments size
	    m_size1--;
	  }
	  m_table1[i] = EMPTY; //sets file on old table to empty
	}
      }

    } else if (inc == .75) {
      //cout << "in .75" << endl;
      int fiveOne = m_capacity1 * (inc - 0.25);
      int sevFive = m_capacity1 * inc;
      for (int i = fiveOne; i < sevFive; i++) {
	if ((!(m_table1[i] == EMPTY)) || (!(m_table1[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table1[i].key()) % m_capacity2;
	  int j = 0;
	  while (!(m_table2[index] == EMPTY) && !(m_table2[index] == DELETED)) {
	    index = ((m_hash(m_table1[i].key()) % m_capacity2) + (j * j)) % m_capacity2;
	    j++;
	  }
	  if ((m_table2[index] == EMPTY || m_table2[index] == DELETED) && (!(m_table1[i] == EMPTY))) {
	    m_table2[index] = File(m_table1[i].key(), m_table1[i].diskBlock()); //inserts into empty index
	    m_size2++; //increments size
	    m_size1--;
	  }
	  m_table1[i] = EMPTY; //sets file on old table to empty
	}
      }
      
    } else if (inc == 1) { //transferring last 25%
      //cout << "in 1" << endl;
      int sevFive = m_capacity1 * (inc - 0.25); //75%
      int hundo = m_capacity1 * inc; //100%
      for (int i = sevFive; i < hundo; i++) { //loops through last 25% of old table
	if ((!(m_table1[i] == EMPTY)) || (!(m_table1[i] == DELETED))) { //if spot has live data
	  //cout << "i = " << i << endl;
	  index = m_hash(m_table1[i].key()) % m_capacity2; //sets index
	  int j = 0;
	  while (!(m_table2[index] == EMPTY) && !(m_table2[index] == DELETED)) { //finds empty index
	    index = ((m_hash(m_table1[i].key()) % m_capacity2) + (j * j)) % m_capacity2; //usees quad probing to do so
	    j++;
	  }
	  if ((m_table2[index] == EMPTY || m_table2[index] == DELETED) && (!(m_table1[i] == EMPTY))) {
	    m_table2[index] = File(m_table1[i].key(), m_table1[i].diskBlock()); //inserts into empty index
	    m_size2++; //increments size
	    m_size1--;
	  }
	  m_table1[i] = EMPTY; //sets file on old table to empty
	}
      }
      isRehashing = false; //rehashing complete
    } else { //should never emter this loop
      cout << "whats going on?" << endl;
    }
  }
}

float HashTable::lambda(TABLENAME tablename) const {
  if (tablename == TABLE1){ //calculates for table1
    return (float(m_size1)/m_capacity1);
  }else{ //for table2
    return (float(m_size2)/m_capacity2);
  }      
}

float HashTable::deletedRatio(TABLENAME tableName) const {
  if (tableName == TABLE1){ //calculates for table1
    return (float(m_numDeleted1))/(m_size1 - m_numDeleted1);
  }else{ //calculates for table2
    return (float(m_numDeleted2))/(m_size2 - m_numDeleted2);
  }   
}

void HashTable::dump() const {
    cout << "Dump for table 1: " << endl;
    if (m_table1 != nullptr)
        for (int i = 0; i < m_capacity1; i++) {
            cout << "[" << i << "] : " << m_table1[i] << endl;
        }
    cout << "Dump for table 2: " << endl;
    if (m_table2 != nullptr)
        for (int i = 0; i < m_capacity2; i++) {
            cout << "[" << i << "] : " << m_table2[i] << endl;
        }
}

bool HashTable::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int HashTable::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}
