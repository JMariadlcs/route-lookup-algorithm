
#include "main.h"
#include "io.h"
#include "tables.h"

int error;

struct forwarding_table table1;
struct forwarding_table table2;


int main(int argc,char **argv){
	
	error = 0;
	
	start_ftable(&table1, 16777216);   //We intialize tables
	start_ftable(&table2, 0);
	
	if(argc != 3){										//checking number of arguments is correct
		puts("Invalid number of input arguments");
		return -1;
	}
	
	error = initializeIO(argv[1], argv[2]);				//Initializing input files
	if(error!=0){
		puts("Error:" );
		printIOExplanationError(error); //This method manages the error messages
		return -1;
	}
	
	//We create
	struct route route_aux;						//We create an auxiliar route to go through all the preffixes in the routing table and then we initialize it
	initializeRoute(&route_aux);	
	
	do{
		error = readFIBLine(route_aux.IP_prefix, route_aux.mask, route_aux.outInterface); //reading the line and storing the information of each one
		fillFtable(&table1, &table2, &route_aux);										  //We fill the table with the information stored previously
	}while(error==0);
	
	
	address add_aux;							//We create an auxiliar address to go through all the IP address of the second input file and the initialize it
	initializeAddress(&add_aux);
	
	int nPackets = 0;							//Initialization of variables that we will use in the Look Up
	double total_table_accesses = 0.0;
	double total_time_processing = 0.0;
	error = 0;
	error = readInputPacketFileLine(add_aux.IPaddress);
	
	while(error==0){
		lookUp(&table1, &table2, &add_aux);		//We look for the appropiate output interface and store some information that is printed in the next line
		printOutputLine(*add_aux.IPaddress, *add_aux.outIfc, add_aux.startLU, add_aux.endLU, add_aux.searchingTime, *add_aux.accesedTables); 
		nPackets ++;
		total_table_accesses += *add_aux.accesedTables;
		total_time_processing += *add_aux.searchingTime;
		error = readInputPacketFileLine(add_aux.IPaddress);
	}
	
	double avg_table_accesses = total_table_accesses/nPackets;		//We compute the averages
	double avg_time_processing = total_time_processing/nPackets;
	
	printSummary(nPackets, avg_table_accesses, avg_time_processing);		//We print the summary
	
	freeFtable(&table1);			//frees for each malloc
	freeFtable(&table2);
	freeIO();
	freeRoute(&route_aux);
	freeAddress(&add_aux);
	
	
	
	
	
}
