// Cache class declaration

#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <cstdint>

class Cache {
public:
    // based off command line arguments from project description
    /*
     * constructor to initialize the cache with user-provided configurations
     * num_sets: number of sets in the cache
     * blocks_per_set: number of blocks per set (associativity)
     * block_size: size of each block in bytes
     * write_allocate: true if write-allocate, false if no-write-allocate
     * write_back: true if write-back, false if write-through
     * is_lru: true for LRU eviction, false for FIFO eviction
     */
    Cache(uint32_t num_sets, uint32_t blocks_per_set, uint32_t block_size, bool write_allocate, bool write_back, bool is_lru);

    /*
     * load memory access
     * address: 32 bit int memory address to load from
    */
    void load(uint32_t addresss);
    /*
     * store memory access
     * address: 32 bit int memory address to store to
    */
    void store(uint32_t addresss);
    /*
     * print cache statistics
    */
    void print_statistics() const;

    private:
    struct Block {
        bool valid;
        bool dirty;
        uint32_t tag;
        uint32_t last_used; // when block was last touched for LRU
        uint32_t load_time; // when block was brought into cache for FIFO
    };

    struct Set {
        std::vector<Block> blocks;
    };

    std::vector<Set> sets;

    // initial configuration variables
    uint32_t num_sets;
    uint32_t blocks_per_set;
    uint32_t block_size;
    bool write_allocate;
    bool write_back;
    bool is_lru; 

    // bit shifts to read address
    uint32_t offset_bits;
    uint32_t index_bits;
    
    // timer to increament to track relative LRU and FIFO ages
    uint32_t time_counter; 

    // track needed statistics for output
    uint32_t total_loads;
    uint32_t total_stores;
    uint32_t load_hits;
    uint32_t load_misses;
    uint32_t store_hits;
    uint32_t store_misses;
    uint32_t total_cycles;

    // helper functions to get index and tag from address
    uint32_t get_index(uint32_t address) const;
    uint32_t get_tag(uint32_t address) const;
};

#endif // CACHE_H
        
    
}