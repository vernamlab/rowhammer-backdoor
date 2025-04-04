#define _GNU_SOURCE		// For O_DIRECT
#include <stdio.h>
#include <stdint.h>		// For uint64_t
#include <fcntl.h>		// For O_RDONLY in get_physical_addr fn
#include <unistd.h>		// For pread in get_physical_addr fn
#include <sys/mman.h>	// For mmap
#include <sys/sysinfo.h> // For sysinfo()
#include <sys/stat.h>   // for file size and fstat()

#include <assert.h>
#define PAGE_SIZE 4096
#define METHOD 1

// get_physical_addr function has been taken from
// https://github.com/IAIK/flipfloyd/blob/master/waylaying/check.c

static uint64_t get_physical_addr(uint64_t virtual_addr) {
  static int g_pagemap_fd = -1;
  uint64_t value;
  
  // open the pagemap
  if(g_pagemap_fd == -1) {
      g_pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
  }
  if(g_pagemap_fd == -1) return 0;
  
  // read physical address
  off_t offset = (virtual_addr / 4096) * sizeof(value);
  int got = pread(g_pagemap_fd, &value, sizeof(value), offset);
  if(got != 8) return 0;

  // Check the "page present" flag.
  if(!(value & (1ULL << 63))) return 0;

  // return physical address
  uint64_t frame_num = value & ((1ULL << 55) - 1);
  return (frame_num * 4096) | (virtual_addr & (4095));
}
void printmemsize(char *str, unsigned long ramsize) {
        printf("%s: %ld in bytes / %ld in KB / %ld in MB / %ld in GB\n",str, ramsize, ramsize/1024, (ramsize/1024)/1024, ((ramsize/1024)/1024)/1024);
}
unsigned long print_sysinfo(){
	struct sysinfo info;
	sysinfo(&info);
	printf("uptime: %ld\n", info.uptime);
	// print total ram size
	printmemsize("totalram", info.totalram);
	printmemsize("freeram", info.freeram);
	printmemsize("sharedram", info.sharedram);
	printmemsize("bufferram", info.bufferram);
	printmemsize("freeswap", info.freeswap);
	printf("current running processes: %d\n\n", info.procs);
	return info.freeram;
}
// A utility function to swap to integers
void swap (int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

// A function to generate a random permutation of arr[]
void randomize ( int arr[], int n )
{
	// Use a different seed value so that we don't get same
	// result each time we run this program
	srand ( time(NULL) );

	// Start from the last element and swap one by one. We don't
	// need to run for the first element that's why i > 0
	for (int i = n-1; i > 0; i--)
	{
		// Pick a random index from 0 to i
		int j = rand() % (i+1);

		// Swap arr[i] with the element at random index
		swap(&arr[i], &arr[j]);
	}
}
void printArray (int arr[], int n)
{
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

void main() {
	unsigned long freeram = print_sysinfo();
	uint8_t * buffer;
	FILE* model = open("resnet20-12fca82f.th",O_RDONLY);
	// FILE* model = open("nullbytes",O_RDONLY);

	struct stat sb;

	fstat(model, &sb);
	//posix_fadvise(model, 0, sb.st_size, POSIX_FADV_RANDOM);

	int num_chunks = 1;//	PAGE_COUNT/512;
	long int PAGE_COUNT =  (long int)(sb.st_size/PAGE_SIZE);

	printf("%ld chunks\n",num_chunks);
	printf("%ld pages\n",PAGE_COUNT);
	printf("____________________________________________________\n");
	printf("\n			MAP\n");
	printf("____________________________________________________\n");
	// Warmup loop
	// for(int i=0;i<100000;i++){
	// 	uint8_t *  buffer1 = mmap(NULL, num_chunks * PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE,  MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE , -1, 0);			
	// 	munmap(buffer1,num_chunks * PAGE_COUNT * PAGE_SIZE);
	// 	}
	// Mapping all pages at once.
	buffer = mmap(NULL, num_chunks * PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE,  MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE , -1, 0);			
	if(buffer==-1){
		perror("mmap failed:");
	}
	// Access the buffer to ensure it is really loaded
   	char a;
    for(int i=0; i<PAGE_COUNT; i++)
    	a = ((char *)buffer)[i*PAGE_SIZE]; 


	FILE *fptr = fopen("buffer_addr.txt","w");

	for (int i = 0; i < num_chunks*PAGE_COUNT; i++) {
		if(i>0)
			fprintf(fptr,"\n");
		fprintf(fptr, "%lx", get_physical_addr((uint64_t)&buffer[i*PAGE_SIZE]));
	}
	printf("____________________________________________________\n");
	printf("\n			MUNMAP & READ FILE\n");
	printf("____________________________________________________\n");
	#if METHOD
		uint8_t * buff;
		printf("%d\n",num_chunks*PAGE_COUNT);

		int idx[num_chunks*PAGE_COUNT];
		for (int i = 0; i < num_chunks*PAGE_COUNT; i++)
  			idx[i] = i;
	
		randomize(idx, num_chunks*PAGE_COUNT);
		// Save unmapped addresses
		fptr = fopen("unmapped_addr.txt","w");
		for (int i = PAGE_COUNT-1; i >= 0  ; i--) {
			if(i<PAGE_COUNT-1)
				fprintf(fptr,"\n");
			fprintf(fptr, "%lx", get_physical_addr((uint64_t)(buffer + idx[i]*PAGE_SIZE)));
		}

		for (int i = 0; i < PAGE_COUNT ; i++)
			munmap(buffer + idx[i]*PAGE_SIZE,  PAGE_SIZE);			// Unmapping one page at a time.
		
		buff = mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ,  MAP_PRIVATE, model, 0);	
		// access file to ensure it is really loaded
		for(int i=0; i<(PAGE_COUNT); i++){
			a = ((char *)buff)[i*PAGE_SIZE]; 
			// posix_fadvise(model, 0, sb.st_size, POSIX_FADV_RANDOM);
		}
		printf("%lx\n",buff);
		printf("%lx\n",buffer + (num_chunks-1)*PAGE_COUNT*PAGE_SIZE);

		if(buff==-1){
			perror("mmap failed:");
		}


		/*
		// Unmapping all pages at once.
		uint8_t * buff;
		assert(munmap(buffer,  PAGE_COUNT*PAGE_SIZE) == 0);	
		// MAP_ANONYMOUS: The mapping is not backed by any file; its contents are
		// initialized to zero.  The fd argument is ignored
		buff = mmap(buffer , PAGE_COUNT*PAGE_SIZE, PROT_READ ,  MAP_PRIVATE, model, 0);	
		if(buff==-1){
			perror("mmap failed:");
		}
		// access file to ensure it is really loaded
		for(int i=0; i<(PAGE_COUNT); i++)
			a = ((char *)buff)[i*PAGE_SIZE]; 
	*/
	#else
		// Unmapping 512 pages at a time in reverse order. (2MB chunks)
		//for (int idx = num_chunks-1; idx >= 0 ; idx--)

		uint8_t * buff;
		assert(munmap(buffer + (num_chunks-1)*PAGE_COUNT*PAGE_SIZE,  PAGE_COUNT*PAGE_SIZE) == 0);	
		
		// MAP_ANONYMOUS: The mapping is not backed by any file; its contents are
		// initialized to zero.  The fd argument is ignored
		buff = mmap(buffer + (num_chunks-1)*PAGE_COUNT*PAGE_SIZE , PAGE_COUNT*PAGE_SIZE, PROT_READ ,  MAP_PRIVATE, model, 0);	
		printf("%lx\n",buff);
		printf("%lx\n",buffer + (num_chunks-1)*PAGE_COUNT*PAGE_SIZE);
		
		if(buff==-1){
			perror("mmap failed:");
		}
		// access file to ensure it is really loaded
		for(int i=0; i<(PAGE_COUNT); i++){
			a = ((char *)buff)[i*PAGE_SIZE];
		}
		//for(int i=0; i<(507); i++)
		//	a = ((char *)buffer)[i*PAGE_SIZE]; 
		

	#endif
	fptr = fopen("file_addr.txt","w");


	for (int i = 0; i <PAGE_COUNT  ; i++) {
		if(i>0)
			fprintf(fptr,"\n");
		fprintf(fptr, "%lx", get_physical_addr((uint64_t)(buff+i*PAGE_SIZE)));

	}
	printf("\nfile size is %d\n",sb.st_size);	
	return;}