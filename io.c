#include "io.h"
#include "utils.h"



/***********************************************************************
 * Static variables for the input/output files
 ***********************************************************************/
static FILE *routingTable;
static FILE *inputFile;
static FILE *outputFile;

/***********************************************************************
 * Write the input to the specified file (f) and the standard output
 *
 * Use as fprintf(FILE *stream, const char *format, ...)
 *
 ***********************************************************************/
void tee(FILE *f, char const *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
}

/********************************************************************
 * Initialize file descriptors
 *
 * routingTableName contains FIB info (argv[1] of main function)
 * inputFileName contains IP addresses (argv[2] of main function)
 *
 ***********************************************************************/
int initializeIO(char *routingTableName, char *inputFileName){

	char outputFileName[100];

  routingTable = fopen(routingTableName, "r");
  if (routingTable == NULL) return ROUTING_TABLE_NOT_FOUND;

  inputFile = fopen(inputFileName, "r");
  if (inputFile == NULL) {
  	fclose(routingTable);
   	return INPUT_FILE_NOT_FOUND;
 	}

  sprintf(outputFileName, "%s%s", inputFileName, OUTPUT_NAME);
  outputFile = fopen(outputFileName, "w");
  if (outputFile == NULL) {
    fclose(routingTable);
    fclose(inputFile);
    return CANNOT_CREATE_OUTPUT;
  }

  return OK;

}


/***********************************************************************
 * Close the input/output files
 ***********************************************************************/
void freeIO() {

	fclose(inputFile);
  fclose(outputFile);
  fclose(routingTable);

}


/***********************************************************************
 * Write explanation for error identifier (verbose mode)
 ***********************************************************************/
void printIOExplanationError(int result){

	switch(result) {
    case ROUTING_TABLE_NOT_FOUND:
      printf("Routing table not found\n");
      break;
    case INPUT_FILE_NOT_FOUND:
      printf("Input file not found\n");
      break;
    case BAD_ROUTING_TABLE:
      printf("Bad routing table structure\n");
      break;
    case BAD_INPUT_FILE:
      printf("Bad input file structure\n");
      break;
    case PARSE_ERROR:
      printf("Parse error\n");
      break;
    case CANNOT_CREATE_OUTPUT:
      printf("Cannot create output file\n");
      break;
    default:
      printf("Unknown error\n");
      break;
  }

}


/***********************************************************************
 * Read one entry in the FIB
 *
 * It should be noted that prefix, prefixLength and outInterface are
 * pointers since they are used as output parameters
 *
 ***********************************************************************/
int readFIBLine(uint32_t *prefix, int *prefixLength, int *outInterface){

	int n[4], result;

	result = fscanf(routingTable, "%i.%i.%i.%i/%i\t%i\n", &n[0], &n[1], &n[2], &n[3], prefixLength, outInterface);
	if (result == EOF) return REACHED_EOF;
  else if (result != 6) return BAD_ROUTING_TABLE;
  else{
 		//remember that pentium architecture is little endian
 		*prefix = (n[0]<<24) + (n[1]<<16) + (n[2]<<8) + n[3];
		//*prefix = n[0]*pow(2,24) + n[1]*pow(2,16) + n[2]*pow(2,8) + n[3];
		return OK;
	}

}


/***********************************************************************
 * Read one entry in the input packet file
 *
 * Again, it should be noted that IPAddress is a pointer since it is used
 * as output parameter
 *
 ***********************************************************************/
int readInputPacketFileLine(uint32_t *IPAddress){

  int n[4], result;

	result = fscanf(inputFile, "%i.%i.%i.%i\n", &n[0], &n[1], &n[2], &n[3]);
	if (result == EOF) return REACHED_EOF;
  else if (result != 4) return BAD_INPUT_FILE;
  else{
 		//remember that pentium architecture is little endian
  	*IPAddress = (n[0]<<24) + (n[1]<<16) + (n[2]<<8) + n[3];
		//*IPAddress = n[0]*pow(2,24) + n[1]*pow(2,16) + n[2]*pow(2,8) + n[3];
		return OK;
	}

}


/***********************************************************************
 * Print a line to the output file
 *
 * clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime) must be called right before the lookup function
 *
 * clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime) must be called right after the lookup function
 *
 * The lookup function must return (either as output parameter or as return value)
 * the number of hash tables that have been accessed for every IP address
 *
 ***********************************************************************/
 void printOutputLine(uint32_t IPAddress, int outInterface, struct timespec *initialTime, struct timespec *finalTime,
                        double *searchingTime, int numberOfTableAccesses) {

  long sec, nsec;

  nsec = finalTime->tv_nsec - initialTime->tv_nsec;
    if (nsec < 0){
    	initialTime->tv_sec += 1;
  	  nsec = 1e9 - nsec;
    }
    sec = finalTime->tv_sec - initialTime->tv_sec;

    *searchingTime = 1e9*sec + nsec;

	//remember that output interface equals 0 means no matching
	//remember that if no matching but default route is specified in the FIB, the default output interface
	//must be stored to avoid dropping the packet (i.e., MISS)
  if (!outInterface)
    tee(outputFile,"%i.%i.%i.%i;%s;%i;%.0lf\n",IPAddress >> 24, (IPAddress >> 16) & 0x000000ff, (IPAddress >> 8) & 0x000000ff, IPAddress & 0x000000ff , "MISS",numberOfTableAccesses, *searchingTime);
	else
	tee(outputFile,"%i.%i.%i.%i;%i;%i;%.0lf\n",IPAddress >> 24, (IPAddress >> 16) & 0x000000ff, (IPAddress >> 8) & 0x000000ff, IPAddress & 0x000000ff , outInterface,numberOfTableAccesses, *searchingTime);

}


/***********************************************************************
 * Print execution summary to the output file
 *
 * It should be noted that:
 *
 * 		averageTableAccesses = totalTableAccesses/processedPackets
 *
 *		averagePacketProcessingTime = totalPacketProcessingTime/processedPackets
 *
 ***********************************************************************/
void printSummary(int processedPackets, double averageTableAccesses, double averagePacketProcessingTime){
	tee(outputFile, "\nPackets processed= %i\n", processedPackets);
  tee(outputFile, "Average table accesses= %.2lf\n", averageTableAccesses);
  tee(outputFile,"Average packet processing time (nsecs)= %.2lf\n", averagePacketProcessingTime);
	printMemoryTimeUsage();

}


/***********************************************************************
 * Print memory and CPU time
 *
 * For more info: man getrusage
 *
 ***********************************************************************/
void printMemoryTimeUsage(){

	float    user_time, system_time;
  long int memory;
  struct rusage usage;

  if (getrusage (RUSAGE_SELF, &usage)){
    printf("Resource measurement failed.\n");
  }
  else{
  	user_time = (float)usage.ru_utime.tv_sec+(float)usage.ru_utime.tv_usec/1000000.0;
  	system_time  = (float)usage.ru_stime.tv_sec+(float)usage.ru_stime.tv_usec/1000000.0;
  	memory = usage.ru_maxrss;

  	tee(outputFile, "Memory (Kbytes) = %ld\n", memory );
  	tee(outputFile, "CPU Time (secs)= %.6f\n\n", user_time+system_time);
  }

}

//RL Lab 2020 Switching UC3M
