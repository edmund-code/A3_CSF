#include "cache.h"
#include <iostream>

/*
 * helper to calculate log base 2
 * x: value to compute log2 of
 * returns: number of bits needed to represent x
 * used to derive offset_bits and index_bits from block_size and num_sets
 */
static uint32_t log2_impl(uint32_t x) {
    uint32_t count = 0;
    // number of times can divide x by 2
    while (x > 1) {
        x >>= 1;
        count++;
    }
    return count;
}

// constructor to initializes the cache and zero statistics
Cache::Cache(uint32_t num_sets, uint32_t blocks_per_set, uint32_t block_size, bool write_allocate, bool write_back, bool is_lru): 
    num_sets(num_sets), blocks_per_set(blocks_per_set), block_size(block_size),
    write_allocate(write_allocate), write_back(write_back), is_lru(is_lru),
    time_counter(0), total_loads(0), total_stores(0), load_hits(0), 
    load_misses(0), store_hits(0), store_misses(0), total_cycles(0) {

    // create cache which is set of sets where elements are blocks
    sets.resize(num_sets);
    
    // initialize all blocks to invalid, untagged, and unused
    for (uint32_t i = 0; i < num_sets; i++) {
        sets[i].blocks.resize(blocks_per_set);
        for (uint32_t j = 0; j < blocks_per_set; j++) {
            sets[i].blocks[j].valid = false;
            sets[i].blocks[j].dirty = false;
            sets[i].blocks[j].tag = 0;
            sets[i].blocks[j].last_used = 0;
            sets[i].blocks[j].load_time = 0;
        }
    }

    // calculate number of offset and index bits based off block size and number of sets
    offset_bits = log2_impl(block_size);
    index_bits = log2_impl(num_sets);
}

/*
 * extracts the set index bits from a memory address
 * address: 32-bit memory address
 * returns: index used to select the corresponding cache set
 * returns 0 for fully associative caches (index_bits == 0)
 */
uint32_t Cache::get_index(uint32_t address) const {
    if (index_bits == 0) return 0;
    return (address >> offset_bits) & ((1 << index_bits) - 1);
}

/*
 * extracts the tag bits from a memory address
 * address: 32-bit memory address
 * returns: tag used to identify the block within a set
 * prevent undefined behavior when shift amount >= 32
 */
uint32_t Cache::get_tag(uint32_t address) const {
    uint32_t shift_amount = offset_bits + index_bits;
    if (shift_amount >= 32) return 0;
    return address >> shift_amount;
}

/*
 * searches a set for a block matching the given tag
 * index: index of the cache set to search
 * tag: tag bits extracted from the memory address
 * returns: position of the matching block, or -1 on a miss
 */
int Cache::find_block(uint32_t index, uint32_t tag) const {
    for (uint32_t i = 0; i < blocks_per_set; i++) {
        if (sets[index].blocks[i].valid && sets[index].blocks[i].tag == tag) {
            return i;
        }
    }
    return -1;
}

/*
 * finds an empty (invalid) block slot in the given set
 * index: index of the cache set to search
 * returns: position of an invalid block, or -1 if the set is full
 */
int Cache::find_empty_block(uint32_t index) const {
    for (uint32_t i = 0; i < blocks_per_set; i++) {
        if (!sets[index].blocks[i].valid) {
            return i;
        }
    }
    return -1;
}

/*
 * selects the block to evict from a full set
 * index: index of the cache set to evict from
 * returns: position of the block chosen for eviction
 * uses lru or fifo depending on is_lru
 */
int Cache::find_evict_block(uint32_t index) const {
    int block_to_evict = 0;
    
    if (is_lru) {
        // LRU: Find the block with the smallest last_used time
        uint32_t min_time = sets[index].blocks[0].last_used;
        for (uint32_t i = 1; i < blocks_per_set; i++) {
            if (sets[index].blocks[i].last_used < min_time) {
                min_time = sets[index].blocks[i].last_used;
                block_to_evict = i;
            }
        }
    } else {
        // finish this for fifo
    }
    return block_to_evict;
}

/*
 * load memory access
 * address: 32-bit memory address to load from
 * increments total_loads and time_counter, then uses process_hit or process_miss
 */
void Cache::load(uint32_t address) {
    total_loads++;
    time_counter++;
    
    uint32_t index = get_index(address);
    uint32_t tag = get_tag(address);

    int block_idx = find_block(index, tag);
    
    // if block found then process hit else process miss
    if (block_idx != -1) {
        load_hits++;
        process_hit(index, block_idx, false);
    } else {
        load_misses++;
        process_miss(index, tag, false);
    }
}

/*
 * store memory access
 * address: 32-bit memory address to store to
 * increments total_stores and time_counter, then uses process_hit or process_miss
 */
void Cache::store(uint32_t address) {
    total_stores++;
    time_counter++;
    
    uint32_t index = get_index(address);
    uint32_t tag = get_tag(address);

    int block_idx = find_block(index, tag);
    
    // if block found then process hit else process miss
    if (block_idx != -1) {
        store_hits++;
        process_hit(index, block_idx, true);
    } else {
        store_misses++;
        process_miss(index, tag, true);
    }
}

/*
 * update block and cycle count on a cache hit
 * index: cache set index of the hit block
 * block_idx: position of the hit block within the set
 * is_store: true if the access is a store, false if a load
 * if store hit then mark dirty write-back or add 101 cycles for write-through (penalty for writing to main memory)
 * if load hit then add 1 cycle
 */
void Cache::process_hit(uint32_t index, int block_idx, bool is_store) {
    // update last used time for LRU
    sets[index].blocks[block_idx].last_used = time_counter;

    if (is_store) {
        // if write back then mark dirty
        if (write_back) {
            sets[index].blocks[block_idx].dirty = true;
            total_cycles += 1;
        } else {
            // write through: 1 cycle for cache + 100 to write to memory
            total_cycles += 101; 
        }
    } else {
        total_cycles += 1;
    }
}

/*
 * handles eviction, block loading, and cycle accounting on a cache miss
 * index: cache set index where the miss occurred
 * tag: tag of the missing block to load
 * is_store: true if the access is a store, false if a load
 * no-write-allocate store misses write directly to memory (100 cycles) and return early
 * evicts dirty write-back blocks when the set is full, adding extra memory cycles
 */
void Cache::process_miss(uint32_t index, uint32_t tag, bool is_store) {
    uint32_t block_transfer_cycles = (block_size / 4) * 100;

    // no-write-allocate on a store miss write directly to memory (100 cycles) and return early
    if (is_store && !write_allocate) {
        total_cycles += 100; 
        return; 
    }

    // fetch block from memory
    total_cycles += block_transfer_cycles; 
    
    int target_idx = find_empty_block(index);

    // eviction needed
    if (target_idx == -1) {
        target_idx = find_evict_block(index);
        
        // write back dirty evict block to memory
        if (write_back && sets[index].blocks[target_idx].dirty) {
            total_cycles += block_transfer_cycles; 
        }
    }

    // load new block
    sets[index].blocks[target_idx].valid = true;
    sets[index].blocks[target_idx].tag = tag;
    sets[index].blocks[target_idx].last_used = time_counter;
    sets[index].blocks[target_idx].load_time = time_counter;

    // process the initial request on the newly loaded block
    if (is_store) {
        if (write_back) {
            sets[index].blocks[target_idx].dirty = true;
            total_cycles += 1;
        } else {
            sets[index].blocks[target_idx].dirty = false;
            total_cycles += 101;
        }
    } else {
        sets[index].blocks[target_idx].dirty = false;
        total_cycles += 1;
    }
}

/*
 * print cache statistics
 * outputs total loads, stores, hits, misses, and cycles to stdout
 * format matches the assignment specification exactly
 */
void Cache::print_statistics() const {
    std::cout << "Total loads: " << total_loads << "\n";
    std::cout << "Total stores: " << total_stores << "\n";
    std::cout << "Load hits: " << load_hits << "\n";
    std::cout << "Load misses: " << load_misses << "\n";
    std::cout << "Store hits: " << store_hits << "\n";
    std::cout << "Store misses: " << store_misses << "\n";
    std::cout << "Total cycles: " << total_cycles << "\n";
}