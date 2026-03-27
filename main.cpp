#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdint>
#include "cache.h"

int main( int argc, char **argv ) {
  // TODO: implement  
  if (argc != 7) {
    
    std::cerr << "Usage: ./csim num_sets blocks_per_set block_size write_policy alloc_policy eviction_policy" << std::endl;
    
    return 1;
  }

  uint32_t num_sets = atoi(argv[1]);
  uint32_t blocks_per_set = atoi(argv[2]);
  uint32_t block_size = atoi(argv[3]);
  std::string alloc_policy = argv[4];
  std::string write_policy = argv[5];
  std::string eviction_policy = argv[6];

  //power of 2
  if (num_sets == 0 || (num_sets & (num_sets - 1)) != 0) {
    
    std::cerr << "Error: num_sets must be a positive power of 2" << std::endl;
    return 1;
  }

  if (blocks_per_set == 0 || (blocks_per_set & (blocks_per_set - 1)) != 0) {
    
    std::cerr << "Error: blocks_per_set must be a positive power of 2" << std::endl;
    return 1;
  }

  if (block_size < 4 || (block_size & (block_size - 1)) != 0) {
    
    std::cerr << "Error: block_size must be a positive power of 2 and at least 4" << std::endl;
    return 1;

  }

  //string arguments
  if (alloc_policy != "write-allocate" && alloc_policy != "no-write-allocate") {
    
    std::cerr << "Error: invalid allocation policy" << std::endl;
    return 1;

  }
  if (write_policy != "write-through" && write_policy != "write-back") {
    
    std::cerr << "Error: invalid write policy" << std::endl;
    return 1;
 
  }
  
  if (eviction_policy != "lru" && eviction_policy != "fifo") {
  
    std::cerr << "Error: invalid eviction policy" << std::endl;
    return 1;
  
  }

  //write alloc and write back not allow
  if (alloc_policy == "no-write-allocate" && write_policy == "write-back") {
    
    std::cerr << "Error: no-write-allocate cannot be combined with write-back" << std::endl;
    return 1;
  
  }

  bool write_allocate = (alloc_policy == "write-allocate");
  bool write_back = (write_policy == "write-back");
  bool is_lru = (eviction_policy == "lru");

  Cache cache(num_sets, blocks_per_set, block_size, write_allocate, write_back, is_lru);

  std::string line;
  while (std::getline(std::cin, line)) {
    
    if (line.empty()) {
      continue;

    };

    char operation;
    uint32_t address;
    int ignored;

    if (sscanf(line.c_str(), " %c 0x%x %d", &operation, &address, &ignored) != 3) {
      continue;
    }

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