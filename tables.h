#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>


#define BIT_16 0x8000

typedef struct forwarding_table{
	unsigned int num_entries;
	unsigned short int* table; 
}ftable;

struct route{
	uint32_t *IP_prefix;
	int *mask;
	int *outInterface;
};

typedef struct address{
	uint32_t *IPaddress;
	int *outIfc;
	unsigned char *accesedTables;
	struct timespec *startLU;
	struct timespec *endLU;
	double *searchingTime;
}address;




/*********************************************************************************************************************************************
 * Summary: Initialize fields of struct forwarfing tables
 * 
 * Parammeters: struct *forwarfing_table: struct that you want to initalize
 * 				unsigned int entries: number of entries you want your table to have. If you want your table to be dinamyc, you should put 0.
 * 
 *********************************************************************************************************************************************/
 
void start_ftable(struct forwarding_table* fwtable, unsigned int entries);

/*********************************************************************************************************************************************
 * Summary: Initialize fields of struct route
 * 
 * Parammeters: struct *route: struct route that you want to initialize
 * 				
 * 
 *********************************************************************************************************************************************/
 
void initializeRoute(struct route* ro);

/*********************************************************************************************************************************************
 * Summary: Initialize fields of struct Address
 * 
 * Parammeters: address* add: struct address that you want to initialize
 * 				
 * 
 *********************************************************************************************************************************************/

void initializeAddress(address* add);


/*********************************************************************************************************************************************
 * Summary: Given a route, it fills the forwarding table. Depending on the size of the mask, it will store the output interface in the main or 
 * 			in the secondary table. If it stores them in the second table, it will put a reference to the position in the main one. 
 * 			
 * 
 * Parammeters: ftable *mainTable:  Main table of the forwarding table, the static one that handles the first 24 bits of the addresses. 
 * 				ftable *secondTable: secondary table of the forwarding table, the dynamic one that handles the last 8 bits of the addresses
 * 				struct route *ro: struct route that stores the necessary information (preffix, mask and output interface) to fulfill the tables		
 * 
 *********************************************************************************************************************************************/

void fillFtable(ftable *mainTable, ftable *secondTable, struct route *ro);

/*********************************************************************************************************************************************
 * Summary: Given an address, it looks for its output interface and compute the number of accessed tables and teh time taken to do this.
 * 
 * Parammeters: ftable *mainTable: Main table of the forwarding table, the static one that handles the first 24 bits of the addresses. 
 * 				ftable *secondTable: secondary table of the forwarding table, the dynamic one that handles the last 8 bits of the addresses
 * 				address *add: Struct address that gives the necessary address to compute the lookup and store others parammeters such as 
 * 							  accessed tables or computing time
 *********************************************************************************************************************************************/

void lookUp(ftable *mainTable, ftable *secondTable, address *add);


/*********************************************************************************************************************************************
 * Summary: It fills the memory of a struct ftable
 * 
 * Parammeters: ftable* : struct forwarding table whose memory will be released
 * 				
 *********************************************************************************************************************************************/
 
void freeFtable(ftable* fwtable);


/*********************************************************************************************************************************************
 * Summary: It fills the memory of a struct route
 * 
 * Parammeters: struct route *ro : struct route whose memory will be released
 * 				
 *********************************************************************************************************************************************/

void freeRoute(struct route *ro);


/*********************************************************************************************************************************************
 * Summary: It fills the memory of a struct address
 * 
 * Parammeters: struct address : struct address whose memory will be released
 * 				
 *********************************************************************************************************************************************/
 

void freeAddress(struct address *add);
