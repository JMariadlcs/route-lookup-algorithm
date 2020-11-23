
#include "tables.h"


void start_ftable(struct forwarding_table *fwtable, unsigned int entries){
	if(entries != 0){															//If it's static, we allocate the necessary memory from the beginning
		fwtable->table = calloc(entries, sizeof(short));						
		fwtable->num_entries = entries;
	}else{																		//If the table is dynamic we only initialize the number of entries
		fwtable->num_entries = 0;
	}
}


void initializeRoute(struct route *ro){
	ro->IP_prefix = (uint32_t*) malloc(sizeof(uint32_t*));
	ro->mask = (int*) malloc(sizeof(int*));
	ro->outInterface= (int*) malloc(sizeof(int));
}

void initializeAddress(address *add){
	add->IPaddress = (uint32_t*) malloc(sizeof(uint32_t));
	add->outIfc = (int*) calloc(1, sizeof(int));
	add->accesedTables = (unsigned char*) malloc(sizeof(unsigned char));
	add->startLU = (struct timespec*) malloc(sizeof(struct timespec));
	add->endLU = (struct timespec*) malloc(sizeof(struct timespec));
	add->searchingTime = (double*) calloc(1, sizeof(double));
}


void fillFtable(ftable *mainTable, ftable *secondTable, struct route *ro){
	int firstEntrie = 0;																														
	int lastEntrie = 0;
	if(*ro->mask <= 24){																									  //If the mask is lower or equal than 24 we only store the output interface in the corresponding entries of the main table
		firstEntrie = (*ro->IP_prefix>>8);																					  //We compute the first entrie with that out. interface
		lastEntrie = firstEntrie + pow(2,24 - *ro->mask);																	  //We compute the last entrie
		int i = 0;
		for(i = firstEntrie; i<lastEntrie; i++){																				//We store the out. interfaces
			mainTable->table[i] = *ro->outInterface;
		}
	}else{ 
		int entrie = (*ro->IP_prefix>>8);																						//We store the first 24 bits of the prefix
		if((mainTable->table[entrie] & BIT_16) == 0){																			//We check if the bit 16 is low
			if(secondTable->num_entries == 0){																					//If it's the first extension, we do a calloc
				secondTable->table = (unsigned short int*) calloc(256, sizeof(unsigned short int));  
				int i = 0;
				for( i = secondTable->num_entries; i<(secondTable->num_entries+256); i++){  									//We fill the 256 entries with the value that was previously stored in the main table
					secondTable->table[i] = mainTable->table[entrie];
				} 
				mainTable->table[entrie] = 0 + pow(2, 15); 																		//we fill the mainTable with the reference to the second table and we put bit 16 to 1
			}else{																												//If it's not the first extension we do a realloc
				secondTable->table = (unsigned short int*) 	realloc(secondTable->table,  2*(256+secondTable->num_entries));
				int i = 0;
				for( i = secondTable->num_entries; i<(secondTable->num_entries+256); i++){  									//We fill the 256 entries with the value that was previously stored in the main table
					secondTable->table[i] = mainTable->table[entrie];
				}
				mainTable->table[entrie] = (secondTable->num_entries)/256 + pow(2, 15);     									//we fill the mainTable with the reference to the second table and we put bit 16 to 1
			}
			
			secondTable->num_entries = secondTable->num_entries + 256;															//We increase the number of entries of the second table
		}
		
		firstEntrie = (mainTable->table[entrie]-pow(2, 15))*256 + (*ro->IP_prefix & 0xFF);  									//We compute the first entrie of the second table and we substract pow(2,15) to put bit16 as zero,
		lastEntrie = firstEntrie + pow(2, (32-*ro->mask));																		//We compute the last entrie
		int i = 0;
		for(i = firstEntrie; i<lastEntrie; i++){																				//We store the out. interfaces in the second table
			secondTable->table[i] = *ro->outInterface;					
		}			
			
	}
	
}

void lookUp(ftable *mainTable, ftable *secondTable, address *add){
	int entrie = *add->IPaddress>>8;																							//In this variable we store the entrie of the main table corresponding to the first 24 bits
	int entrie2 = 0;																											//The same as previous variable but of the second table, if it's necessary
	clock_gettime(CLOCK_REALTIME, add->startLU);																				//We store the value of the time before starting the lookup																		
	if ((mainTable->table[entrie] & BIT_16) == 0){																				//If bit 16 is 0, we store the value of the entrie of the first table
		*add->outIfc = mainTable->table[entrie];																				//Only one table accessed
		*add->accesedTables = 1;
	}else{																														//If it's 1, we compute entrie2 and store the value in that entrie taking into account the reference given by the main table
		entrie2 = *add->IPaddress & 0xFF;		
		*add->outIfc = secondTable->table[(mainTable->table[entrie]-32768)*256+entrie2];
		*add->accesedTables = 2;																								//Two table accesses
	}
	clock_gettime(CLOCK_REALTIME, add->endLU);																					//We store the value of the time right after the look up
}

void freeFtable(ftable* fwtable){
	free(fwtable->table);
}

void freeRoute(struct route *ro){
	free(ro->IP_prefix);
	free(ro->mask);
	free(ro->outInterface);
}

void freeAddress(struct address *add){
	free(add->IPaddress);
	free(add->outIfc);
	free(add->accesedTables);
	free(add->startLU);
	free(add->endLU);
	free(add->searchingTime);
}




