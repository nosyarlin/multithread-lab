
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define SIZE	1048576
#define DELIMITER " \t"
//#define NUMBER_OF_THREADS	128

void *sorter(void *params);	/* thread that performs sorting for subthread*/
void *merger(void *params);	/* thread that performs merging for final result */
void *find_median(void *params);	/* thread that computes median of the sorted array */
int read_to_array(FILE *fp);
void merge(int* input);
void mergeSort(int* input);

int array[SIZE] = {};
int number_of_threads;
int chunksize;

double median = 0.0;

int main (int argc, const char * argv[]) 
{
	FILE *fp;
	int start;
	int end;
	number_of_threads = atoi(argv[2]);
	chunksize = SIZE/number_of_threads;
	int start_end[number_of_threads][2];
	int ends[number_of_threads];

	// read data from external file and store it in an array
	fp = fopen(argv[1], "r");
	read_to_array(fp);
	fclose(fp);

    // partition the array list into N sub-arrays, where N is the number of threads
    for (int i = 0; i < number_of_threads; i++) {
    	start = i*chunksize;
    	end = start + chunksize -1;
    	if (i == number_of_threads-1) {
    		end = SIZE-1;
    	}
    	start_end[i][0] = start;
    	start_end[i][1] = end;
    	ends[i] = end;
    }

    // create a list of threads using pthread_t that sorts sub-arrays.
    pthread_t workers[number_of_threads];

    // start recording time
    clock_t t0 = clock();

    // start threads by passing the sub-array they need to sort and the function they execute
   	for (int i = 0; i < number_of_threads; i++) {
   		pthread_create(&workers[i], NULL, sorter, start_end[i]);
   	}

   	/* now wait for the threads to finish */
   	for (int i = 0; i < number_of_threads; i++) {
   		pthread_join(workers[i], NULL);
   	}

 	// establish the merger thread
	pthread_t merging_thread;

	pthread_create(&merging_thread, NULL, merger, ends);

	// wait for the final mean computing thread to finish
	pthread_join(merging_thread, NULL);
	
	// find median
	median = array[SIZE/2];

	// stop recording time and compute the elapsed time
	clock_t t1 = clock();
	double time_elapsed = (double) (t1-t0)/CLOCKS_PER_SEC;

	// printout the final sorted array
	for (int i = 0; i < SIZE; ++i)
	{
		printf("%d\n", array[i]);
	}

	// print out time elapsed
	printf("Time elapsed = %fms\n", time_elapsed*1000); 

	// printout median
	printf("Median of dataset = %f\n", median);

    return 0;
}


// You can use any sorting algorithm
void *sorter(void *params) {
	int *input;

	input = (int*) params;
	mergeSort(input);
	pthread_exit(NULL);	
}

// You can use any merging algorithm
void *merger(void *params) {
	int *input;
	int middle;
	int right;

	// prepare params
	input = (int*) params;

	// merge 
	for (int i = 1; i < number_of_threads; ++i)
	{
		right = input[i];
		middle = right-chunksize;
		int final_merge[3] = {0,middle,right};
		merge(final_merge);
	}
	
	pthread_exit(NULL);
}

void *find_median(void *params) {

	pthread_exit(NULL);
}

int read_to_array(FILE *fp) {
	char str[SIZE*6];
	char *number;
	int index = 0;

	if (fp == NULL)
	{
		perror("Error opening file");
    	return(-1);
	}
	else {
		fgets(str, SIZE*6, fp);
		number = strtok(str, DELIMITER);
		while (number != NULL){
			array[index] = atoi(number);
			index++;
			number = strtok(NULL, DELIMITER);
		}
		return 1;
	}
}

void mergeSort(int *input){
	int left;
	int right;
	int middle;

	left = input[0];
	right = input[1];
	if (left < right)
	{
		// split arr by the middle
		middle = left + (right-left)/2;
		int split_left[2] = {left, middle};
		int split_right[2] = {middle+1, right};
		int join[3] = {left, middle, right};
		
		// sort left and right, then merge
		mergeSort(split_left);
		mergeSort(split_right);
		merge(join);
	}
	return; 
}

void merge(int *input){
	int i, j, k;
	int left, middle, right;
	int left_size, right_size;

	// prepare params
	left = input[0];
	middle = input[1];
	right = input[2];

	left_size = middle - left + 1;
	right_size = right - middle;

	// creating temporary arrays
	int L[left_size], R[right_size];

	// copy data from array for reference later
	for (int i = 0; i < left_size; ++i)
	{
		L[i] = array[left+i];
	}
	for (int i = 0; i < right_size; ++i)
	{
		R[i] = array[middle+i+1];
	}

	// Merge L and R
	i = 0;
	j = 0;
	k = left; // for writing into array

	while (i < left_size && j < right_size){
		if (L[i] <= R[j])
		{
			array[k] = L[i];
			i++;
		}
		else
		{
			array[k] = R[j];
			j++;
		}
		k++;
	}

	// Copy remaining of L if any
	while (i < left_size){
		array[k] = L[i];
		i++;
		k++;
	}

	// Copy remaining of K if any
	while (j < right_size){
		array[k] = R[j];
		j++;
		k++;
	}
	return;
}

