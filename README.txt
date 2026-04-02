TODO: names of team members and their contributions to the project

Members:
Edmund Tsou
Evan Batten

Contributions:
Edmund: something or the other
Evan: some more stuff

TODO (for MS3): best cache report

We began with a direct-mapped baseline configuration:

./csim 256 1 16 write-allocate write-through lru < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 298863
Load misses: 19334
Store hits: 185202
Store misses: 12284
Total cycles: 32911483

This configuration produced a high number of misses and the worst overall performance, demonstrating the limitations of direct-mapped caches due to conflict misses.

Next, we increased associativity by testing a 4-way set associative cache:

./csim 256 4 16 write-allocate write-back lru < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9344483

This significantly reduced both load and store misses and greatly improved total cycles, showing that increased associativity reduces conflict misses.

We then tested a fully associative cache:

./csim 1 256 16 write-allocate write-back lru < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 313123
Load misses: 5074
Store hits: 187653
Store misses: 9833
Total cycles: 10802483

While fully associative caches eliminate conflict misses, this configuration performed worse than the 4-way cache. This indicates diminishing returns from very high associativity and less efficient use of locality.

Next, we compared eviction policies using the same 4-way configuration:

./csim 256 4 16 write-allocate write-back fifo < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 314171
Load misses: 4026
Store hits: 188047
Store misses: 9439
Total cycles: 9845283

FIFO resulted in more misses and higher total cycles compared to LRU, showing that LRU better exploits temporal locality.

We then evaluated write policies:

./csim 256 4 16 write-allocate write-through lru < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 25318283

The hit and miss counts remained the same, but total cycles increased significantly. This is because write-through writes to memory on every store, increasing memory traffic.

Finally, we tested different block sizes to evaluate spatial locality.

Block size 32 bytes:

./csim 256 4 32 write-allocate write-back lru < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 316126
Load misses: 2071
Store hits: 192823
Store misses: 4663
Total cycles: 9543683

Block size 64 bytes:

./csim 256 4 64 write-allocate write-back lru < gcc.trace

Result:
Total loads: 318197
Total stores: 197486
Load hits: 316955
Load misses: 1242
Store hits: 195056
Store misses: 2430
Total cycles: 9707683

Increasing block size reduced the number of misses due to improved spatial locality. However, larger block sizes also increased the miss penalty, resulting in higher total cycles compared to the 16-byte configuration.

Based on all experiments, the best overall configuration is:

256 sets, 4 blocks per set, 16-byte block size, write-allocate, write-back, and LRU eviction.

This configuration achieved the lowest total cycles while maintaining low miss rates. It provides a good balance between associativity, efficient eviction, reduced memory traffic, and manageable miss penalties.