/*************************************************************************
	> File Name: radix_AVX.c
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
void divide(int *a, int N, int *output, int part);
//void counting_sort(int *a, int N, int *result, int *part);

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


void divide(int *a, int N, int *output, int part){
		
	const int NUM_PARTS=N/SIZE;
	int *a0=a;
	int *result=output;
	int d_one = 0xff000000; // do operation & and logic right shift 24 bits
	int d_two = 0x00ff0000; //do operation & and logic right shift 16 bits
	int d_three = 0x0000ff00; //do operation & and logic right shift 8 bits
	int d_four = 0x000000ff; //do operation & and logic right shift 0 bits

	int one_shift=24;
	int two_shift=16;
	int three_shift=8;
	int four_shift=0;

	if(part==1){
		int *b0=result;
		for(int cnt=0;cnt<NUM_PARTS;cnt++){
			__m256i and_data_one=_mm256_set1_epi32(d_one);

			__m256i data= _mm256_loadu_si256((__m256i const *)a0); a0+=SIZE;

			//__mm256i one_and = _mm256_and_si256(data,and_data_one); //there is no need to do & operation

			_mm256_storeu_si256((__m256i *)b0,_mm256_srli_epi32(data,one_shift)); b0+=SIZE;

		}
	}
	else if(part==2){
		int *c0=result;
		for(int cnt=0;cnt<NUM_PARTS;cnt++){
	
			__m256i and_data_two=_mm256_set1_epi32(d_two);

			__m256i data= _mm256_loadu_si256((__m256i const *)a0); a0+=SIZE;

			__m256i two_and = _mm256_and_si256(data,and_data_two);

			_mm256_storeu_si256((__m256i *)c0,_mm256_srli_epi32(two_and,two_shift)); c0+=SIZE;
		}
	}
	else if(part==3){
		int *d0=result;
		for(int cnt=0;cnt<NUM_PARTS;cnt++){
			__m256i and_data_three=_mm256_set1_epi32(d_three);

			__m256i data= _mm256_loadu_si256((__m256i const *)a0); a0+=SIZE;

			__m256i three_and = _mm256_and_si256(data,and_data_three);

			_mm256_storeu_si256((__m256i *)d0,_mm256_srli_epi32(three_and,three_shift)); d0+=SIZE;
		}
	}
	else if(part==4){
		int *e0=result;
		for(int cnt=0;cnt<NUM_PARTS;cnt++){

			__m256i and_data_four=_mm256_set1_epi32(d_four);

			__m256i data= _mm256_loadu_si256((__m256i const *)a0); a0+=SIZE;

			__m256i four_and = _mm256_and_si256(data,and_data_four);

			_mm256_storeu_si256((__m256i *)e0,four_and); e0+=SIZE; // there is no need to do right shift
		}
	}
	else{
		printf("the number must be in 1~4.\n");
		return;
	}
	
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
		
		/*
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
		*/
		int *part=(int *)malloc(N*sizeof(int));
		divide(a,N,part,NUM-cnt);

		//scan unsort array, calculating  n[B]
		for(int i=0;i<N;i++){
			int value= part[i];
			//printf("a[%d]=%d\n",i,value);
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
			int value = part[i];
			int index=value%B;
			result[P[index]+b[index]]=a[i];
			b[index]++;
		}

		//printf("a=\n");print_array(a,N);
		//printf("result=\n");print_array(result,N);

		memcpy(a,result,N*sizeof(int));
		//printf("a=\n");print_array(a,N);
		free(result);
	}

	//end time
	struct timeval end;
	gettimeofday(&end,NULL);

	//print_result(a,N);
	//(a,N);
	//time calculation
	printf("time=%d, number=%d",(int)end.tv_usec-(int)start.tv_usec,a[N/2]);
	

	free(a);
}
