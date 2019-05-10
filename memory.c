#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tlbData
{
	int page;
	int frame;
};

int main(int argc, char *argv[]) 
{
	FILE *input = fopen(argv[1], "r");
	FILE *backing = fopen("BACKING_STORE.bin", "r");

	float faultCount = 0;
	float faultRate = 0;
	float hitCount = 0;
	float hitRate = 0;

	int logicalAddress = 0;
	int inputCount = 0;
	int physicalAddress = 0;
	int pageNumber = 0;
	int offset = 0;
	int frame = 0;
	int hit = 0;
	char buf[256];
	int tlbSize = 0;
	int lastPage = 0;
        int lastFrame = 0;
	
	int pageTable[256];
	memset(pageTable, -1, 256*sizeof(int));

	struct tlbData tlb[16];
	int physicalMemory[65536];

	while(fscanf(input, "%d", &logicalAddress) == 1) 
	{
		inputCount++;
		hit = -1;
		pageNumber = logicalAddress & 0x0000FF00;
		pageNumber = pageNumber >> 8;
		offset = logicalAddress & 0xFF;	
		
		for(int i = 0; i < tlbSize; i++) 
		{
  			if(tlb[i].page == pageNumber) 
			{
        		hit = tlb[i].frame;
        		physicalAddress = hit*256 + offset;
      			}
		}
		if(hit != -1) 
			hitCount++;
		else if(pageTable[pageNumber] == -1) 
		{
			faultCount++;
			fseek(backing, pageNumber*256, SEEK_SET);
			fread(buf, sizeof(char), 256, backing);
			pageTable[pageNumber] = frame;
			for(int i = 0; i < 256; i++) 
			{
				physicalMemory[frame*256 + i] = buf[i];
			}
			frame++;

			lastPage = tlb[1].page;
			lastFrame = tlb[1].frame;
			tlb[1].page = tlb[0].page;
			tlb[1].frame = tlb[0].frame;
			for(int i = 1; i < tlbSize; i++) 
			{
				int tempPage = tlb[i+1].page;
				int tempFrame = tlb[i+1].frame;
				tlb[i+1].page = lastPage;
				tlb[i+1].frame = lastFrame;
				lastPage = tempPage;
				lastFrame = tempFrame;
			}

			if(tlbSize < 16)
				tlbSize++;

			tlb[0].page = pageNumber;
			tlb[0].frame = pageTable[pageNumber];
			physicalAddress = pageTable[pageNumber]*256 + offset;
		} 
		else 
		{
			physicalAddress = pageTable[pageNumber]*256 + offset;
		}	
		printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, physicalMemory[physicalAddress]);
	}

	faultRate = faultCount / inputCount;
	hitRate = hitCount / inputCount;
	printf("Page Fault Rate = %f\n", faultRate);
        printf("TLB Hit Rate %f\n", hitRate);

	fclose(input);
	fclose(backing);
	return 0;
}
