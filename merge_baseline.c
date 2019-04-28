/*************************************************************************
	> File Name: merge_baseline.c
	> Author: logos
	> Mail: 838341114@qq.com 
	> Created Time: 2019年04月28日 星期日 15时24分56秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<pthread.h>
#include<string.h>
#include<immintrin.h>

#define seed 1
#define NUM_THREADS 16
#define MAX(a,b) (((a)>(b)?:(a):(b)))
#define M 64

void max(const void *a, const void *b);
void sort_gen(int *d, int N, int s);
void print_array(int *d, int  N);
void print_result(int *d, int N);
void* sort(void *arg);
void* merge_small(void *arg);
void* merge_large(void* arg);

void max(const void *a, const void *b){
	return (*(int*)a-*(int*)b);
}

void sort_gen(int *d, int N,int s){
	srand(s);
	for(int i=0;i<N;i++){
		d[i]=rand();
	}
}

void print_array(int *d, int N){
	for(int i=0;i<N;i++){
		if(i%10==0) printf("\n");
		printf("%d ",d[i]);
	}
	printf("\n");
}

void print_array(int *d, int N){
	for(int i=0;i<N-1;i++){
		if(i%10==0) printf("\n");
		printf("%d ",d[i]>d[i+1]?0:1);
	}
	printf("\n");
}

struct paramter{
	int N;
	int *a; //unsorted array
	int cnt;
	int num; //the num block
}

void* sort(void *arg){

	struct parameter *p;
	p=(struct parameter *)arg;
	
	int N=p->N;
	int cnt=p->cnt;
	int *a=p->a;
	int num=p->num;

	int thread_size=M/NUM_THREADS;
	int start_index=num*M+thread_size*cnt;
	int end_index=start_index+thread_size;

	//sort a[start_index-end_index]
	
}

void* merge_small(void *arg){
	
}

void* merge_large(void *arg){

}

int main(int argc,char *argv[]){

	//parameter init
	const int N=atoi(argv[1]);
	//float seed=atof(argv[2]);
	
	int *a=(int *)malloc(N*sizeof(int));

	//array generation
	sort_gen(a,N,seed);

	//start time
	struct timeval start;
	gettimeofday(&start,NULL);

	//merge sort with threads-parallel
	const int block_num=N/M; //divide the array into  a[N/M] array 
	for(int i=0;i<block_num;i++){
		//the i block
		
		//init threads parameters
		pthread_t threads[NUM_THREADS0];
		struct parameter parameters[NUM_THREADS];

		//divide the M block into NUM_THREADS * thread_size block
		int thread_size=M/NUM_THREADS;
		for(int cnt=0;cnt<NUM_THREADS;cnt++){
			
			//init parameter
			parameters[cnt].N=N;
			parameters[cnt].a=a;
			parameters[cnt].cnt=cnt;
			parameters[cnt].num=i;

			pthread_create(&threads[cnt],NULL,sort,&parameters[cnt]);
		}
		for(int cnt=0;cnt<NUM_THREADS;cnt++){
			pthread_join(threads[cnt],NULL);
		}


		//merge T sorted array
		for(int cnt=0;cnt<NUM_THREADS;cnt++){

			//init parameter
			
			pthread_create(&threads[cnt],NULL,merge_small,&parameters[cnt]);
		}
		for(int cnt=0;cnt<NUM_THREADS;cnt++){
			pthread_join(threads[cnt],NULL);
		}

		//merge N/M sorted array
		for(int cnt=0;cnt<NUM_THREADS;cnt++){

			//init parameter
			
			pthread_create(&threads[cnt],NULL,merge_large,&parameters[cnt]);
		}
		for(int cnt=0;cnt<NUM_THREADS;cnt++){
			pthread_join(threads[cnt],NULL);
		}
	}

	//end time
	struct timeval end;
	gettimeofday(&end,NULL);
	
	long duration=end.tv_sec-start.tv_sec;

	printf("time=%ld, number=%d\n",duration,a[N/2]);
	free(a);
}

