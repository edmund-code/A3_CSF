#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdint>
#include "cache.h"

int main( int argc, char **argv ) {
  // TODO: implement  
  if (argc != 7) {
    
    std::cerr << "./csim numSets blocksPerSet blockSize write-allocate|no-write-allocate write-through|write-back lru|fifo, you did this wrong" << std::endl;
    
    return 1;
  }

  //set and parse all the args
  uint32_t numSet = atoi(argv[1]);
  uint32_t blocksPer = atoi(argv[2]);
  uint32_t blockSize = atoi(argv[3]);

  std::string allocPol = argv[4];
  std::string writePol = argv[5];
  std::string evictPol = argv[6];

  //power of 2 correction
  if (numSet == 0 || (numSet & (numSet - 1)) != 0) {
    
    std::cerr << "numSet must be pos power of 2" << std::endl;
    return 1;
  }

  if (blocksPer == 0 || (blocksPer & (blocksPer - 1)) != 0) {
    
    std::cerr << "blocksFer pos power of 2" << std::endl;
    return 1;
  }

  if (blockSize < 4 || (blockSize & (blockSize - 1)) != 0) {
    
    std::cerr << "blockSize not positive power of 2 and at least 4" << std::endl;
    return 1;

  }

  //string args check
  if (allocPol != "write-allocate" && allocPol != "no-write-allocate") {
    
    std::cerr << "invalid alloc policy" << std::endl;
    return 1;

  }
  if (writePol != "write-through" && writePol != "write-back") {
    
    std::cerr << "invalid write pol." << std::endl;
    return 1;
 
  }
  
  if (evictPol != "lru" && evictPol != "fifo") {
  
    std::cerr << "invalid evict pol" << std::endl;
    return 1;
  
  }

  //write alloc and write back not allow
  if (allocPol == "no-write-allocate" && writePol == "write-back") {
    
    std::cerr << "cant do both alloc and write" << std::endl;
    return 1;
  
  }

  //flags for if alloc write, write back, and isLru
  bool writeAlloc = (allocPol == "write-allocate");
  bool writeBack = (writePol == "write-back");
  bool isLru = (evictPol == "lru");

  Cache cache(numSet, blocksPer, blockSize, writeAlloc, writeBack, isLru);

  //get each line
  std::string tmp;
  while (std::getline(std::cin, tmp)) {
    
    if (tmp.empty()) {
      continue;

    };

    //vars for the op, address, etc.
    char operation;
    uint32_t address;
    int ignored;
    
    //validates the input line, if we dont have 3 then not valid so we should not load or store
    if (sscanf(tmp.c_str(), " %c 0x%x %d", &operation, &address, &ignored) != 3) {
      continue;
    }

    //decides load or store op based on flag
    if (operation == 'l') {
     
      cache.load(address);

    } else if (operation == 's') {
      cache.store(address);
    }

  }

  cache.print_statistics();
  
  return 0;
}

//hi