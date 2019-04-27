/*************************************************************************
	> File Name: radix_parallel.c
	> Author: logos
	> Mail: 838341114@qq.com 
	> Created Time: 2019年04月26日 星期五 23时23分01秒
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
#define BASE 8
#define NUM_THREADS 25

int max(const void*a, const void *b);
void sort_gen(int *d, int N);
void print_array(int *a, int N);
void* counting(void *arg);
void* hashing(void *arg);

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
	for(int cnt=0;cnt<N-1;cnt++){
		printf("%d ",a[cnt]<a[cnt+1]? 1:0);
		if(cnt%10==0) printf("\n");
	}
}

void print_array(int *a, int N){
	for(int cnt=0;cnt<N;cnt++){
		if(cnt%10==0) printf("\n");
		printf("%d ",a[cnt]);
	
	}
	printf("\n");
}

struct parameter{
	int N;
	int *num;
	int *D;
	int *a;
	int *result;
	int part;
	int cnt;
};

void* counting(void *arg){
	
	struct parameter *p;
	p=(struct parameter *) arg;

	int N=p->N;
	int *num=p->num;
	int *a=p->a;
	int part=p->part;
	int cnt=p->cnt;

	int size=N/NUM_THREADS;
	int start_index=cnt*size;
	int end_index=start_index+size;

	int and;
	int shift;
	if(part==0){
		and=0x000000ff;
		shift=0;
	}
	else if(part==1){
		and=0x0000ff00;
		shift=8;
	}
	else if(part==2){
		and=0x00ff0000;
		shift=16;
	}
	else if(part==3){
		and=0xff000000;
		shift=24;
	}
	
	
	//scan unsort array, calculating num[B]
	for(int i=start_index;i<end_index;i++){
		int value= (a[i] & (and))>>(shift);
		int index= value%B; //the index (th) hash bucket
		//printf("index=%d\n",index);
		num[cnt*B+index]++;
	}
}

void* hashing(void *arg){

	struct parameter *p;
	p=(struct parameter *) arg;

	int N=p->N;
	int *num=p->num;
	int *D=p->D;
	int *a=p->a;
	int *result=p->result;
	int part=p->part;
	int cnt=p->cnt;

	int size=N/NUM_THREADS;
	int start_index=size*cnt;
	int end_index=start_index+size;

	int and;
	int shift;
	if(part==0){
		and=0x000000ff;
		shift=0;
	}
	else if(part==1){
		and=0x0000ff00;
		shift=8;
	}
	else if(part==2){
		and=0x00ff0000;
		shift=16;
	}
	else if(part==3){
		and=0xff000000;
		shift=24;
	}

	for(int i=start_index;i<end_index;i++){
		int value = (a[i]&and)>>shift;
		int index=value%B;
		result[D[cnt*B+index]]=a[i];
		D[cnt*B+index]++;
	}

}

int main(int argc, char *argv[]){

	//parameter initiation
	const int N	= atoi(argv[1]); 
	//float seed = atof(argv[2]);

	int *a=(int *)malloc(N*sizeof(int));
	int *result=(int *)malloc(N*sizeof(int));

	//array generation
	sort_gen(a,N);

	//start time
	struct timeval start;
	gettimeofday(&start,NULL);

	//radix sort with thread-parallel, without cache optimization

	//init thread parameters
	pthread_t threads_one[NUM_THREADS];
	struct parameter parameters_one[NUM_THREADS];
	int size=N/NUM_THREADS;

	for(int part=0;part<NUM;part++){ 

		//the first step
		int n[B]={0}; // the size of the ith bucket
		int D[NUM_THREADS*B]={0}; // D[i][j] represents the address of thread i in bucket j
		int num[NUM_THREADS*B]={0};
		for(int cnt=0;cnt<NUM_THREADS;cnt++){

			parameters_one[cnt].N=N;
			parameters_one[cnt].num=num;
			parameters_one[cnt].a=a;
			parameters_one[cnt].result=result;
			parameters_one[cnt].cnt=cnt;
			parameters_one[cnt].part=part;

			pthread_create(&threads_one[cnt],NULL,counting,&parameters_one[cnt]);
		}

		for(int i=0;i<NUM_THREADS;i++){
			pthread_join(threads_one[i],NULL);
		}

		
		for(int i=0;i<B;i++){
			for(int j=0;j<NUM_THREADS;j++){
				n[i]+=num[j*B+i];
			}
		}
		

		//the second step
		int address=0;
		for(int j=0;j<B;j++){
			if(n[j]==0) continue;
			int offset=0;
			for(int i=0;i<NUM_THREADS;i++){
				if(num[i*B+j]==0) continue;
				D[i*B+j]=address+offset;
				offset+=num[i*B+j];
			}
			address+=n[j];
		}

		/*
		 * print D matrix & num & n matrix
		*/
	
		/*
		for(int i=0;i<NUM_THREADS;i++){
			printf("the %d thread:",i);
			for(int j=0;j<B;j++){
				printf("num[%d]=%d;",j,num[i*B+j]);
				printf("n[%d]=%d;",j,n[j]);
				printf("D[%d][%d]=%d\n",i,j,D[i*B+j]);
			}
			printf("\n");
		}
		*/

		//the third step
		struct parameter parameters_two[NUM_THREADS];
		pthread_t threads_two[NUM_THREADS];
		for(int cnt=0;cnt<NUM_THREADS;cnt++){

			parameters_two[cnt].N=N;
			parameters_two[cnt].D=D;
			parameters_two[cnt].a=a;
			parameters_two[cnt].result=result;
			parameters_two[cnt].cnt=cnt;
			parameters_two[cnt].part=part;

			pthread_create(&threads_two[cnt],NULL,hashing,&parameters_two[cnt]);

		}
	
		for(int i=0;i<NUM_THREADS;i++){
			pthread_join(threads_two[i],NULL);
		}

		
		memcpy(a,result,N*sizeof(int));
		memset(result,0,(N*sizeof(int)));
	}

	//end time
	struct timeval end;
	gettimeofday(&end,NULL);

	//print_array(a,N);
	//print_result(a,N);
	//time calculation
	printf("time=%d, number=%d",(int)end.tv_usec-(int)start.tv_usec,a[N/2]);
	
	free(result);
	free(a);
}
