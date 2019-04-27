/*************************************************************************
	> File Name: radix_baseline.c
	> Author: logos
	> Mail: 838341114@qq.com 
	> Created Time: 2019年04月26日 星期五 14时58分01秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<pthread.h>
#include<immintrin.h>
#include<string.h>

//#define N 1000000000 //the size of unsorted array
#define seed 1 //the random seed
#define B 256  //divide the int into 4 parts, the size of each part is 256
#define NUM 4  //the number of parts divided, the size of int is 4 byte
#define AVX_SIZE 8
#define SSE_SIZE 4
#define SIZE AVX_SIZE

int max(const void*a, const void *b);
void sort_gen(int *d, int N);
void print_array(int *a, int N);

int max(const void *a, const void *b){
	return (*(int*)a-*(int*)b);
}

void sort_gen(int *d,int N){
	srand(seed);
	for(int i=0;i<N;i++){
		d[i]=rand();
	}
}

void print_result(int *a,int N){
	int end=1000;
	for(int cnt=0;cnt<end-1;cnt++){
		printf("%d ",a[cnt]<a[cnt+1]? 1:0);
		if(cnt%10==0) printf("\n");
	}
}

void print_array(int *a, int N){
	for(int cnt=0;cnt<N;cnt++){
		printf("%d ",a[cnt]);
	
		if(cnt%30==0) printf("\n");
	}
	printf("\n");
}


int main(int argc, char *argv[]){

	//parameter initiation
	const int N	= atoi(argv[1]); 
	//float seed = atof(argv[2]);

	int *a=(int *)malloc(N*sizeof(int));

	//array generation
	sort_gen(a,N);

	//start time
	struct timeval start;
	gettimeofday(&start,NULL);

	//radix sort without thread-parallel & cache optimization

	for(int cnt=0;cnt<NUM;cnt++){ 
		int *result=(int *)malloc(N*sizeof(int));
		int n[B]={0}; // n[i] record the size of the ith bucket
		int P[B]={0}; // P[i] record the head address of the ith bucket
		int b[B]={0};

		int and,shift;
		if(cnt==0){
			and=0x000000ff;
			shift=0;
		}
		else if(cnt==1){
			and=0x0000ff00;
			shift=8;
		}
		else if(cnt==2){
			and=0x00ff0000;
			shift=16;
		}
		else if(cnt==3){
			and=0xff000000;
			shift=24;
		}

		//scan unsort array, calculating  n[B]
		for(int i=0;i<N;i++){
			int value= (a[i]&and)>>(shift);
			int index= value%B; //the index (th) hash bucket
			n[index]+=1;
		}

		int address=0;
		//caclulating P[B]
		for(int i=0;i<B;i++){
			if(n[i]==0){ //empty hash bucket
				P[i]=-1;
				continue;
			}
			P[i]=address;
			address+=n[i];
		}

		//put the value into bucket
		for(int i=0;i<N;i++){
			int value = (a[i]&and)>>shift;
			int index=value%B;
			result[P[index]+b[index]]=a[i];
			b[index]++;
		}

		memcpy(a,result,N*sizeof(int));
		free(result);
	}

	//end time
	struct timeval end;
	gettimeofday(&end,NULL);

	//print_result(a,N);
	//time calculation
	printf("time=%d, number=%d",(int)end.tv_usec-(int)start.tv_usec,a[N/2]);
	

	free(a);
}
