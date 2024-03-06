# Threaded Merge Sort Experiments


## Host 1: [Kali VM]

- CPU: AMD Ryzen 9 5900HS
- Cores: 4
- Cache size (if known):
- RAM: 7 GB
- Storage (if known): 24GB
- OS: Kali Linux Rolling 2023.3

### Input data

I Tested with 4 different txt files, each with 100,000,000 ints.
I generated these files with the following cmd sequence:

DATA_SIZE=100000000
shuf -i 1-${DATA_SIZE} > data1.txt
shuf -i 1-${DATA_SIZE} > data2.txt
shuf -i 1-${DATA_SIZE} > data3.txt
shuf -i 1-${DATA_SIZE} > data4.txt

msort took 1.970586 seconds. [./msort 10000000 < data1.txt]

### Experiments

#### 1 Threads

Command used to run experiment: `MSORT_THREADS=1 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 24.010193 seconds
2. 23.705410 seconds
3. 23.705989 seconds
4. 23.593259 seconds

#### 2 Threads

Command used to run experiment: `MSORT_THREADS=2 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 12.479224 seconds
2. 12.395369 seconds
3. 12.488961 seconds
4. 12.441423 seconds

#### 3 Threads

Command used to run experiment: `MSORT_THREADS=3 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 12.241522 seconds
2. 12.304244 seconds
3. 12.234366 seconds
4. 12.292640 seconds

#### 4 Threads

Command used to run experiment: `MSORT_THREADS=4 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 8.102412 seconds
2. 7.247188 seconds
3. 7.505076 seconds
4. 7.570875 seconds


## Host 2: [linux-[071-085].khoury.northeastern.edu] / Khoury Login

- CPU: Intel(R) Xeon(R) Silver 4214R 
- Cores: 24
- Cache size (if known): 42 MB
- RAM: 156 GB
- Storage (if known): 36 GB
- OS: 
Rockey Linux release 8.8 (Green Obsidian)
### Input data

I Tested with 4 different txt files, each with 100,000,000 ints.
I generated these files with the following cmd sequence:

DATA_SIZE=100000000
shuf -i 1-${DATA_SIZE} > data1.txt
shuf -i 1-${DATA_SIZE} > data2.txt
shuf -i 1-${DATA_SIZE} > data3.txt
shuf -i 1-${DATA_SIZE} > data4.txt

Time it took for msort =  2.507352 seconds, (./msort 10000000 < data1.txt > sorted_output.txt)

### Experiments

#### 1 Threads

Command used to run experiment: `MSORT_THREADS=1 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 24.521140 seconds
2. 24.192198 seconds
3. 24.033178 seconds
4. 24.031268 seconds

#### 2 Threads

Command used to run experiment: `MSORT_THREADS=2 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 13.035881 seconds
2. 13.163095 seconds
3. 14.540906 seconds
4. 13.108544 seconds

#### 3 Threads

Command used to run experiment: `MSORT_THREADS=3 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 13.129393 seconds
2. 13.104675 seconds
3. 13.069744 seconds
4. 13.062477 seconds

#### 4 Threads

Command used to run experiment: `MSORT_THREADS=4 ./tmsort $DATA_SIZE < data<#>.txt > /dev/null`

Sorting portion timings:

1. 7.974092 seconds
2. 7.573185 seconds
3. 7.355995 seconds
4. 7.370637 seconds


## Observations and Conclusions
There seems to be a clear positive trend for performance as # of threads increased int he 1 to 4 range. I expected thissince multi-threading allows parallel processing/reduced time for computation. 
There does seem to be diminishing return after a certain # of threads, for example in Host2, moving 2 to 3 threads had minimal improvement and 3 to 4 had slight improvement. 

Host1 shows substantial improvement moving 3 to 4 threads whilst Host 2 had minimal gainz.
This suggests that other factors(cache size, memory, etc) started playing a signifiant role.

Improvement in performance dimnishes may be due to things like memory bandwidth limitations and cache misses which can bottleneck. Cache size and memory speed could play a significant role and impact perforrmance of multi-threaded operations. 

On a side note, this kind of tells us that the optimal # of threads in our case should probably not exeed the # of physical cores as it could diminish returns(counterproductive).

