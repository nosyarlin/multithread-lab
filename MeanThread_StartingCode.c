
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 524294

void *get_temporal_mean(void *params);	/* thread that computes mean values from the original array*/
void *get_global_mean(void *params);	/* thread that computes global mean from each sub mean*/

int array[SIZE] = {};
int count = 0;
double *temp;
double globalMean = 0.0;                 /* final mean value for the whole array*/
pthread_mutex_t mutex; 

int main (int argc, const char * argv[])
{
	FILE *fp;
	int index = 0;
	int *buf = malloc(sizeof(int));
	int start;
	int end;
	int chunksize;
	// TODO: read data from external file and store it in an array
	// Note: you should pass the file as a first command line argument at runtime.
	fp = fopen(argv[1], "r");
	while (index != SIZE) {
		fscanf(fp, "%d", buf);
		array[index] = buf[0];
		index++;
	}
	fclose(fp);

	// define number of threads
    int number_of_threads = atoi(argv[2]); 
    temp = malloc(number_of_threads*sizeof(int));
    //this way, you can pass number of threads as 
	// a second command line argument at runtime. 

    // TODO: partition the array list into N sub-arrays, where N is the number of threads
    chunksize = (SIZE/number_of_threads);
    int subarray[number_of_threads][chunksize];

    for (int i = 0; i < number_of_threads; i++) {
    	start = i*chunksize;
    	end = start + chunksize -1;
    	if (i == number_of_threads-1) {
    		end = SIZE-1;
    	}

    	for (int j = start; j <= end; ++j) {
    		subarray[i][j%chunksize] = array[j];
    	}
    }

    // TODO: create a list of threads using pthread_t that computes temporal means. E.g.,
    pthread_t workers[number_of_threads];

    // TODO: start recording time
    clock_t t0 = clock();

    // TODO: start threads by passing the sub-array they need to process and the function they execute
	for (int i = 0; i < number_of_threads; i++) {
		pthread_create(&workers[i], NULL, get_temporal_mean, subarray[i]);
	}

	/* now wait for the threads to finish */
	for (int i = 0; i < number_of_threads; i++) {
		pthread_join(workers[i], NULL);
	}

	// TODO: printout temporal mean values computed by each thread
	for (int i = 0; i < number_of_threads; ++i) {
		printf("%f\n", temp[i]);
	}

	// TODO: establish the final mean computing thread
	pthread_t findMean;

	pthread_create(&findMean, NULL, get_global_mean, temp);

	//wait for the final mean computing thread to finish
	pthread_join(findMean, NULL);

	// TODO: stop recording time and compute the elapsed time
	clock_t t1 = clock();
	double time_elapsed = t1 - t0;

	// TODO: printout the global mean value
	printf("%s %f\n", "Global mean =",globalMean);
	// TODO: printout the execution time
	printf("%s %f\n", "Time elapsed =", time_elapsed);

    return 0;
}

//TODO: get temporal mean values of the sub arrays
void *get_temporal_mean(void *params) {
	// TODO: compute temporal mean values for each sub array of the original array with N threads
	int total = 0;
	int num_ints;
	int *input;
	double mean;

	input = (int*) params;
	num_ints = sizeof(input)/sizeof(*input);
	for (int i = 0; i < num_ints; ++i) {
		total = total + input[i];
	}
	mean = total/num_ints;

	pthread_mutex_lock(&mutex);
	temp[count] = mean;
	count++;
	pthread_mutex_unlock(&mutex);
	//TODO: store the temporal mean values to another array, e.g., temp_array[]
	pthread_exit(NULL);
}

//TODO: get global mean value
void *get_global_mean(void *params) {
	//TODO: compute the global mean from the temp_array[]
	int total = 0;
	int num_ints;
	int *input;

	input = (int*) params;
	num_ints = sizeof(input)/sizeof(*input);
	for (int i = 0; i < num_ints; ++i) {
		total = total + input[i];
	}
	globalMean = total/num_ints;
	pthread_exit(NULL);
}


