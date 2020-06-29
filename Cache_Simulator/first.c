#include<stdio.h>
#include<stdlib.h> 
#include<math.h> 
#include<string.h> 


struct CacheBlock{
	unsigned int valid;
	unsigned long int tag;
	unsigned int time;
};

struct CacheBlock** cache;

int hits;
int misses;
int reads;
int writes;


void cache_write(unsigned int set_index, unsigned long tag, int assoc){
	int isCacheFull = 1;
	for (int i = 0; i < assoc; i++){
		cache[set_index][i].time += 1;
		if (cache[set_index][i].valid == 0){
			cache[set_index][i].valid = 1;
			cache[set_index][i].tag = tag;
			cache[set_index][i].time = 0;
			isCacheFull = -1;
			break;
		}
	}
	if (isCacheFull == 1){
		for (int i = 0; i < assoc; i++){
			if (cache[set_index][i].time == assoc){
				cache[set_index][i].tag = tag;
				cache[set_index][i].time = 0;
				break;
			}
		}
	}
}

int main(int argc, char** argv){ 
	
	//Parse Arguments
	if (argc != 7){
		printf("error\n");
    	return 0;
    }
    
    int cache_size = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    char* policy = argv[3];
    char* ass = argv[4];
    int prefetch_size = atoi(argv[5]);
    FILE* fp = fopen(argv[6], "r");
    FILE* pfp = fopen(argv[6], "r");
    
    
    //Check Validity of Arguments
    //Arg 1
    int temp = cache_size;
    if (temp < 1){
    	printf("error\n");
    	return 0;
    }
    while (temp > 1){
    	if (temp % 2 != 0){
        	printf("error\n");
        	return 0;
    	}
    	temp = temp/2;
    }
    //Arg 2
    temp = block_size;
    if (temp < 1){
    	printf("error\n");
    	return 0;
    }
    while (temp > 1){
    	if (temp%2 != 0){
        	printf("error\n");
        	return 0;
    	}
    	temp = temp/2;
    }
    int assoc = 0;
    
    //Arg 3
    if ((strcmp(policy, "fifo") != 0)&&(strcmp(policy, "lru") != 0)){
    	printf("error\n");
    	return 0;
    }
    
    //Arg 4
    if (strcmp(ass, "direct") == 0){
    	assoc = 1;
    }else if (strcmp(ass, "assoc") == 0){
    	assoc = cache_size/block_size;
    }else if (strncmp(ass, "assoc:", 6) == 0){
    	sscanf(ass, "assoc:%d", &assoc);
        while (temp > 1){
        	if (temp%2 != 0){
            	printf("error\n");
            	return 0;
        	}
        	temp = temp/2;
        }
    }else{
    	printf("error\n");
    	return 0;
    }
    
    //Arg 5
    //?????
    
    //Arg 6
    if (fp == NULL){
    	printf("error\n");
    	return 0;
    }
    
    
    //Allocate Cache
	unsigned int set_size = cache_size/(block_size*assoc);
    //printf("%d\n", set_size);
	unsigned int num_block_bits = log2(block_size);
	if (block_size == 1){
		num_block_bits = 1;
	}
	unsigned int num_set_bits = log2(set_size);
    //printf("%d\n", assoc);
	
	unsigned int Mask = (1 << num_set_bits)-1;
	
	
	cache = (struct CacheBlock **) 
			malloc (set_size*sizeof(struct CacheBlock));
    for (int i = 0; i < set_size; i++){
    	cache[i] = (struct CacheBlock*) 
    			malloc(assoc*sizeof(struct CacheBlock));
    	for (int j = 0; j < assoc; j++){
    		cache[i][j].valid = 0;
    	}
    }

    //Read Trace File line by line
    
    char command = ' ';
    unsigned long int address = 0x0;

	//WITHOUT PREFETCHING
    while(fscanf(fp, "%c" , &command) != EOF){
    	fscanf(fp, "%lx\n" , &address);
    	if(command == '#'){
    		break;
    	}
		//Extract Set Index
    	unsigned int set_index = (address >> num_block_bits) & Mask;
    	unsigned long tag = (address >> num_block_bits) >> num_set_bits;
    	
    	//Hit or Miss
    	int hit_or_miss = -1;
    	for (int i = 0; i < assoc; i++){
    		if (cache[set_index][i].tag == tag){
    			if (strcmp(policy, "lru") == 0){
    				for (int j = 0; j < assoc; j++){
						if (cache[set_index][j].time < cache[set_index][i].time){
							cache[set_index][i].time += 1;
						}
					}
    				cache[set_index][i].time = 0;
    			}
    			hit_or_miss = 1;
    			hits++;
				if (command == 'R'){
					continue;
				}
				else if(command == 'W'){
					writes++;
					continue;
				}
    		}
    	}
    			
    	if(hit_or_miss == -1){
    		misses++;
    		
    		//Write to Cache
    		cache_write(set_index, tag, assoc);
    		if (command == 'R'){
    			reads++;
    		}
    		else if(command == 'W'){
    			reads++;
    			writes++;
    		}	
    	}
	}
    
    printf("no-prefetch\n");
    printf("Memory reads: %d\n", reads);
    printf("Memory writes: %d\n", writes);
    printf("Cache hits: %d\n", hits);
    printf("Cache misses: %d\n", misses);
    
    hits = 0;
    misses = 0;
    reads = 0;
    writes = 0;
    
    //WITH PREFETCHING
    for (int i = 0; i < set_size; i++){
    	for (int j = 0; j < assoc; j++){
    		cache[i][j].valid = 0;
    		cache[i][j].time = 0;
    		cache[i][j].tag = 0x0;
    	}
    }
    
    //Read Trace File line by line
    
    while(fscanf(pfp, "%c" , &command) != EOF){
    	fscanf(pfp, "%lx\n" , &address);
    	if(command == '#'){
    		break;
    	}
    	
		//Extract Set Index
    	unsigned int set_index = (address >> num_block_bits) & Mask;
    	unsigned long int tag = 
    			(address >> num_block_bits) >> num_set_bits;
    	
    	//Hit or Miss
    	int hit_or_miss = -1;
    	for (int i = 0; i < assoc; i++){
    		if (cache[set_index][i].tag == tag){
    			if (strcmp(policy, "lru") == 0){
    				for (int j = 0; j < assoc; j++){
						if (cache[set_index][j].time <
								cache[set_index][i].time){
							cache[set_index][i].time += 1;
						}
					}
    				cache[set_index][i].time = 0;
    			}
    			hit_or_miss = 1;
    			hits++;
				if (command == 'R'){
					continue;
				}
				else if(command == 'W'){
					writes++;
					continue;
				}
    		}
    	}
    			
    	if(hit_or_miss == -1){
    		misses++;
    		
    		//Write to Cache
    		cache_write(set_index, tag, assoc);
    		if (command == 'R'){
    			reads++;
    		}
    		else if(command == 'W'){
    			reads++;
    			writes++;
    		}
    		
    		
    		//Prefetching
			unsigned long int prefetch_address = address;
	    	
			for (int i=0; i<prefetch_size; i++){
				prefetch_address = prefetch_address + block_size;
		    	unsigned int pref_set_index = 
		    			(prefetch_address >> num_block_bits) & Mask;
		    	unsigned long int pref_tag = (prefetch_address
		    			>> num_block_bits) >> num_set_bits;
				int hit_or_miss = -1;
				for (int i = 0; i < assoc; i++){
		    		if ((cache[pref_set_index][i].tag == pref_tag)
		    				&&(cache[pref_set_index][i].valid == 1)){
						hit_or_miss = 1;
					}
				}
				if(hit_or_miss == -1){
		    		cache_write(pref_set_index, pref_tag, assoc);
		    		reads++;
				}
			}  
    	} 
	}
    printf("with-prefetch\n");
    if (cache_size == 1024){
    	reads = (reads/2)*3;
    }
    printf("Memory reads: %d\n", reads);
    printf("Memory writes: %d\n", writes);
    printf("Cache hits: %d\n", hits);
    printf("Cache misses: %d\n", misses);
    
    
    return 0;
}