#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

const int RMAX = 100000;
int thread_count;

/* Headers for helper functions */
void print_list(int a[], int n, char* title);
void generate_list(int a[], int n);
void usage();
void serial_quicksort(int numbers[], int left, int right);
int check_list(int* array, int size);

/* Merge sorted arrays A and B */
void merge(int* A, int size_a, int* B, int size_b)
{
	int i = 0, j = 0, k = 0;
	int size_tmp = size_a + size_b;
	int* tmp = malloc(sizeof(int)*size_tmp);

	/* merge A and B into tmp in sorted order */
	while (k < size_tmp) {
		/* inside A and B, copy smaller value */
		if (i < size_a && j < size_b) {
			tmp[k++] = (A[i] <= B[j]) ? A[i++] : B[j++];
		}
		/* past end of A => copy rest of B */
		else if (i >= size_a) {
			while (k < size_tmp) {
				tmp[k++] = B[j++];
			}
		}
		/* past end of B => copy rest of A */
		else if (j >= size_b) {
			while (k < size_tmp) {
				tmp[k++] = A[i++];
			}
		}
	}
	
	for (i = 0; i < size_tmp; i++)
		A[i] = tmp[i];
	free(tmp);
}

void merge_arrays(int* array, int size, int* indices)
{	
	int i = 0;
	int N = thread_count;
	while (N > 1)
	{
		for (i = 0; i < N; i++)
			indices[i] = (i*size)/N;
		indices[N] = size;
		#pragma omp parallel for
		for (i = 0; i < N; i+=2)
			merge(array+indices[i],indices[i+1]-indices[i],array+indices[i+1],indices[i+2]-indices[i+1]);
			
		N/=2;
	}
}

void serialQuickSort(int numbers[], int array_size)
{
	serial_quicksort(numbers, 0, array_size - 1);
}

int main(int argc, char **argv)
{
	int n;
	int i;
	double start, finish;
	
	if (argc != 3)
		usage();
	
	/* read parameters from command line */
	thread_count = strtol(argv[1], NULL, 10);
	n = strtol(argv[2], NULL, 10);
	
	printf ("Sorting an array of %d random elements using %d threads.\n", n, thread_count);
	
	omp_set_num_threads(thread_count);
	
	/* the initial unsorted array */
	int *array = malloc(sizeof(int)*n);
	
	/* indices demarcating the starts of each thread's data */
	int *indices = malloc(sizeof(int)*(thread_count+1));
	
	generate_list(array, n);
	#ifdef DEBUG
	print_list(array, n, "Unsorted");
	#endif
	
	/* divide the array in chunks among the threads */
	for (i = 0; i < thread_count; i++) indices[i] = (i*n)/thread_count;
	indices[thread_count] = n;
	
	start = omp_get_wtime();
	#pragma omp parallel for
	for (i = 0; i < thread_count; i++) {
		serialQuickSort(array+indices[i], indices[i+1]-indices[i]);
	}	
	merge_arrays(array, n, indices);
	
	finish = omp_get_wtime();
	
	#ifdef DEBUG
	print_list(array, n, "Sorted");
	printf("Validating... ");
	if (check_list(array, n)) printf("Array is sorted.\n");
	else printf("Array is not sorted.\n");
	#endif
	printf("*************************************************************\n");
	printf("Sorting %d elements using quicksort on %d threads took %f s\n", n, thread_count, (finish-start));
	
	FILE *fp;
	fp = fopen("results.dat", "a");
	if (fp == NULL) {
		printf("results.dat could not be opened for writing.\n");
		exit(0);
	} else {
		fprintf (fp, "%d %d %f\n", thread_count, n, (finish-start));
	}

	return 0;
}

/* standard implementation of a quicksort done serially */
void serial_quicksort(int numbers[], int left, int right)
{
	int pivot, l_hold, r_hold;
 
	l_hold = left;
	r_hold = right;
	pivot = numbers[left];
	while (left < right) {
		while ((numbers[right] >= pivot) && (left < right)) right--;
		if (left != right) {
			numbers[left] = numbers[right];
			left++;
		}
		while ((numbers[left] <= pivot) && (left < right)) left++;
		if (left != right) {
			numbers[right] = numbers[left];
			right--;
		}
	}
	numbers[left] = pivot;
	pivot = left;
	left = l_hold;
	right = r_hold;

	if (left < pivot) serial_quicksort(numbers, left, pivot-1);
	if (right > pivot) serial_quicksort(numbers, pivot+1, right);

}

void print_list(int a[], int n, char* title) {
   int i;

   printf("%s:\n", title);
   for (i = 0; i < n; i++)
      printf("%d ", a[i]);
   printf("\n\n");
}  /* print_list */

void generate_list(int a[], int n) {
   int i;

   srandom(1);
   for (i = 0; i < n; i++)
      a[i] = random() % RMAX;
}  /* generate_list */

int check_list(int* array, int size)
{
	int i = 0;
	for (i = 1; i < size; i++)
	{
		if (array[i] < array[i-1])
			return 0;
	}
	return 1;
}

void usage()
{
	printf ("Usage: ./quicksort <thread_count> <n>, where\n<thread_count> is the number of threads to use, and\n<n> is the number of random elements to to sort.\n");
	exit(0);
}
