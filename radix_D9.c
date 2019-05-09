/*************************************************************************
	> File Name: radix_parallel_cache.c
	> Author: logos
	> Mail: 838341114@qq.com 
	> Created Time: 2019年04月27日 Sat 21时46分01秒
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
#define BASE 8 
#define NUM_THREADS 20
#define NUM 4  //the number of parts divided, the size of int is 4 byte
#define M 64  //define the size of cache

pthread_mutex_t mutex;

int max(const void*a, const void *b);
void sort_gen(int *d, int N);
void print_array(int *a, int N);
void* counting(void *arg);
void* hashing(void *arg);
void* set(void *arg);

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
		printf("%d ",a[cnt]>a[cnt+1]? 0:1);
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
	if(cnt==NUM_THREADS-1)
		end_index=N;

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
		num[cnt*B+index]++;
	}
}

void* set(void *arg){

	struct parameter *p;
	p=(struct parameter *)arg;

	int cnt=p->cnt;
	int *result=p->result;
	int *a=p->a;
	int N=p->N;

	int size=N/NUM_THREADS;
	int start_index=size*cnt;
	int end_index=start_index+size;
	
	if(cnt==NUM_THREADS-1){
		end_index=N;
	}
	
	for(int i=start_index;i<end_index;i++){
		if(i>=N||i<0) printf("how?\n");
		a[i]=result[i];
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
	if(cnt==NUM_THREADS-1)
		end_index=N;

	int buffer[M*B]={0};
	int record[B]={0};

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

		buffer[index*M+record[index]]=a[i]; //insert to the buffer
		record[index]++;
		if(record[index]>=M){ //the bucket is full

			//put the buffer into result bucket
			for(int j=0;j<M;j++){
				result[D[cnt*B+index]]=buffer[index*M+j];
				buffer[index*M+j]=0;
				D[cnt*B+index]++;
			}

			//empty the record array
			record[index]=0;
		}

	}

	for(int i=0;i<B;i++){
		if(record[i]>0){//not empty
			for(int j=0;j<record[i];j++){
				result[D[cnt*B+i]]=buffer[i*M+j];
				D[cnt*B+i]++;
			}
		}
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

		struct parameter parameters_three[NUM_THREADS];
		pthread_t threads_three[NUM_THREADS];
		for(int cnt=0;cnt<NUM_THREADS;cnt++){
			parameters_three[cnt].a=a;
			parameters_three[cnt].result=result;
			parameters_three[cnt].cnt=cnt;
			parameters_three[cnt].N=N;

			pthread_create(&threads_three[cnt],NULL,set,&parameters_three[cnt]);
		}
		for(int cnt=0;cnt<NUM_THREADS;cnt++){
			pthread_join(threads_three[cnt],NULL);
		}

		//memset(result,0,(N*sizeof(int)));
	}

	//end time
	struct timeval end;
	gettimeofday(&end,NULL);

	long duration=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000;
	//print_array(a,N);
	//print_result(a,N);
	//time calculation
	printf("time=%ld, number=%d",duration,a[N/2]);

	pthread_mutex_destroy(&mutex);

	free(result);
	free(a);
}
