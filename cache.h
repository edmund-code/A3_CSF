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
     * address: 32-bit memory address to load from
     * updates load hit/miss counts and cycles accordingly
     * on a miss, evicts a block if needed using lru or fifo policy
     */
    void load(uint32_t addresss);
    /*
     * store memory access
     * address: 32-bit memory address to store to
     * updates store hit/miss counts and cycles accordingly
     * behavior depends on write-allocate and write-back/write-through settings
     */
    void store(uint32_t addresss);
    /*
     * print cache statistics
     * outputs total loads, stores, hits, misses, and cycles to stdout
     * called at the end of all memory accesses
     */
    void print_statistics() const;

    private:
    // struct to represent a block in the cache
    struct Block {
        bool valid;
        bool dirty;
        uint32_t tag;
        uint32_t last_used; // when block was last touched for LRU
        uint32_t load_time; // when block was brought into cache for FIFO
    };

    // struct to represent a set of blocks / set of sets in the cache
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
    uint64_t total_loads;
    uint64_t total_stores;
    uint64_t load_hits;
    uint64_t load_misses;
    uint64_t store_hits;
    uint64_t store_misses;
    uint64_t total_cycles;

    // helper functions to get index and tag from address
    /*
     * extracts the set index bits from a memory address
     * address: 32-bit memory address
     * returns: index used to select the corresponding cache set
     */
    uint32_t get_index(uint32_t address) const;

    /*
     * extracts the tag bits from a memory address
     * address: 32-bit memory address
     * returns: tag used to identify the block within a set
     */
    uint32_t get_tag(uint32_t address) const;

    // helper functions for cache hit/miss logic
    /*
     * searches a set for a block matching the given tag
     * index: index of the cache set to search
     * tag: tag bits extracted from the memory address
     * returns: position of the matching block, or -1 on a miss
     */
    int find_block(uint32_t index, uint32_t tag) const;

    /*
     * finds an empty (invalid) block slot in the given set
     * index: index of the cache set to search
     * returns: position of an invalid block, or -1 if the set is full
     */
    int find_empty_block(uint32_t index) const;

    /*
     * selects the victim block to evict from a full set
     * index: index of the cache set to evict from
     * returns: position of the block chosen for eviction
     * uses lru (smallest last_used) or fifo (smallest load_time) depending on is_lru
     */
    int find_evict_block(uint32_t index) const;

    /*
     * updates block state and cycle count on a cache hit
     * index: cache set index of the hit block
     * block_idx: position of the hit block within the set
     * is_store: true if the access is a store, false if a load
     */
    void process_hit(uint32_t index, int block_idx, bool is_store);
    
    /*
     * handles eviction, block loading, and cycle accounting on a cache miss
     * index: cache set index where the miss occurred
     * tag: tag of the missing block to load
     * is_store: true if the access is a store, false if a load
     */
    void process_miss(uint32_t index, uint32_t tag, bool is_store);
};

#endif // CACHE_H