/*************************************************************************
	> File Name: quicksort_lib.c
	> Author: logos
	> Mail: 838341114@qq.com 
	> Created Time: 2019年04月25日 星期四 16时04分14秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<pthread.h>
#include<immintrin.h>

#define N (1000000000)
#define seed (0.3)

#define MAX(a,b) (((a)>(b)?:(a):(b)))

int max(const void *a, const void *b){
	return (*(int*)a-*(int*)b);
}

void sort_gen(int *d){
	srand(seed);
	for(int i=0;i<N;i++){
		d[i]=rand();
	}
}

void print_array(int *a){
	int end=100;
	for(int cnt=0;cnt<end;cnt++){
		printf("%d ",a[cnt]);
		if(cnt%10==0) printf("\n");
	}
}

int main(int argc, char *argv[]){

	//parameter initiation
	//const int N	= atoi(argv[1]); 
	//float seed = atof(argv[2]);

	int *a=(int *)malloc(N*sizeof(int));

	//array generation
	sort_gen(a);

	//start time
	struct timeval start;
	gettimeofday(&start,NULL);

	qsort(a,N,sizeof(int),max);

	//end time
	struct timeval end;
	gettimeofday(&end,NULL);

	//time calculation
	printf("time=%d, number=%d",(int)end.tv_usec-(int)start.tv_usec,a[N/2]);
	
	//print_array(a,N);	
	free(a);
}
