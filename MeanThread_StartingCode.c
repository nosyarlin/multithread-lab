
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1048576
#define DELIMITER " \t"

void *get_temporal_mean(void *params);	/* thread that computes mean values from the original array*/
void *get_global_mean(void *params);	/* thread that computes global mean from each sub mean*/

int array[SIZE] = {};
int count = 0;
int number_of_threads;
double *temp;
double globalMean = 0.0;                 /* final mean value for the whole array*/
pthread_mutex_t mutex; 

int main (int argc, const char * argv[])
{
	FILE *fp;
	int start;
	int end;
	number_of_threads = atoi(argv[2]);
	int chunksize = SIZE/number_of_threads; 
	int start_end[number_of_threads][2];
	temp = malloc((number_of_threads+1) * sizeof(double));

	//read data from external file and store it in an array
	fp = fopen(argv[1], "r");
	read_to_array(fp);
	fclose(fp);

 	//partition the array list into N sub-arrays, where N is the number of threads
    for (int i = 0; i < number_of_threads; i++) {
    	start = i*chunksize;
    	end = start + chunksize -1;
    	if (i == number_of_threads-1) {
    		end = SIZE-1;
    	}
    	start_end[i][0] = start;
    	start_end[i][1] = end;
    }

    //create a list of threads using pthread_t that computes temporal means.
    pthread_t workers[number_of_threads];

 	//start recording time
    clock_t t0 = clock();

    //start threads by passing the sub-array they need to process and the function they execute
	for (int i = 0; i < number_of_threads; i++) {
		pthread_create(&workers[i], NULL, get_temporal_mean, start_end[i]);
	}

	// now wait for the threads to finish 
	for (int i = 0; i < number_of_threads; i++) {
		pthread_join(workers[i], NULL);
	}

	//printout temporal mean values computed by each thread
	for (int i = 0; i < number_of_threads; ++i) {
		printf("Temp mean %d = %f\n", i, temp[i]);
	}

	//establish the final mean computing thread
	pthread_t findMean;

	pthread_create(&findMean, NULL, get_global_mean, temp);

	//wait for the final mean computing thread to finish
	pthread_join(findMean, NULL);

	// stop recording time and compute the elapsed time
	clock_t t1 = clock();
	double time_elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC;

	// printout the global mean value
	printf("%s %f\n", "Global mean =",globalMean);
	// printout the execution time
	printf("%s %fs\n", "Time elapsed =", time_elapsed);

    return 0;
}

//TODO: get temporal mean values of the sub arrays
void *get_temporal_mean(void *params) {
	// compute temporal mean values for each sub array of the original array with N threads
	long total = 0;
	int num_ints;
	int *input;
	double mean;

	// add up all the numbers and divide by number of inputs
	input = (int*) params;
	num_ints = input[1] - input[0] + 1;
	for (int i = input[0]; i <= input[1]; ++i) {
		total = total + array[i];
	}
	mean = (double) total/num_ints;

	// save the temporarily mean in temp
	pthread_mutex_lock(&mutex);
	temp[count] = mean;
	count++;
	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);
}

void *get_global_mean(void *params) {
	//compute the global mean from the temp_array[]
	double total = 0;
	
	for (int i = 0; i < number_of_threads; ++i) {
		total = total + temp[i];
	}
	globalMean = total/number_of_threads;
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

