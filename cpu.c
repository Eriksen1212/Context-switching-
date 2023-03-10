#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>

#define ROW (100)
#define COL ROW

struct sched_attr {
   uint32_t size;              
   uint32_t sched_policy;     
   uint64_t sched_flags;       
   int32_t  sched_nice;       
   uint32_t sched_priority;    

   uint64_t sched_runtime;
   uint64_t sched_deadline;
   uint64_t sched_period;
};
 
static int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
    return syscall(SYS_sched_setattr, pid, attr, flags);
}



int calc(int time, int cpu) {
	int matrixA[ROW][COL];
	int matrixB[ROW][COL];
	int matrixC[ROW][COL];
	int i, j, k;

	struct timespec current_t, last_t;
	long nsec = 0;
	time_t  sec = 0;
	int msec = 0;
	double result;

	int sum = 0;   // 지금까지 계산한 시간을 확인하기 위한 변수

	int cpuid;
	long int count = 0;
	
	int checktime;
	checktime = time * 1000; //ms단위와 비교하기 위해

	cpuid = cpu;

	clock_gettime(CLOCK_REALTIME, &last_t);  //계산을 하기 전에 기준이 되는 시간을 만든다

	while(1) {
		
		for(i=0; i<ROW; i++) {
			for(j=0; j<COL; j++){
				for(k=0; k<COL; k++) {
					matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
				}
			}
		}
		
		count++;   //얼마나 계산이 일어났는지를 확인하기 위해

		clock_gettime(CLOCK_REALTIME, &current_t);  //이전에 기준으로 잡은 시간과의 비교를 위해 현재 시간을 확인 
		sec = (current_t.tv_sec - last_t.tv_sec);
		nsec =  (current_t.tv_nsec - last_t.tv_nsec);
		result = sec + nsec / 1e9;  //두 개의 변수를 가지므로 두 개를 하나의 단위로 합쳐준다

		msec = result * 1000;  //ms의 단위로 비교하는 것이 편하므로 s에서 ms의 단위로 바꿔준다


		if(msec % 100 == 0) {
							printf("PROCESS #%02d count = %02ld %d\n", cpuid, count, 100);  //지금까지의 결과를 출력
							sum += 100; 		
							
				
							clock_gettime(CLOCK_REALTIME, &last_t); //출력을 했다면 새로운 기준이 될 시간이 필요하기 때문에 새 기준을 만들어준다
						}
		if(sum >= checktime) {break;}  // 지금까지 계산한 시간이 진행해야 할 시간에 도달했다면 while문을 break한다.

	}

	printf("DONE!! PROCESS #%02d : %06ld %d\n", cpuid, count, sum);  //while문을 탈출한 뒤, 끝났음을 알려준다.

	return 0;
}


int main(int argc, char *argv[]) {
	

	int result1;

	struct sched_attr attr;
    // 초기화
    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(struct sched_attr);
 
    attr.sched_priority = 10;  
    attr.sched_policy = SCHED_RR;


///////////////////////////////////

	int num = atoi(argv[1]);   //생성할 프로세스의 수를 정수 형태로 받아준다.

	pid_t pids;		//fork를 위한 변수를 만든다
	
	int t = atoi(argv[2]);		//프로세스를 진행할 시간을 정수 형태로 받아준다.
	
	int check[num];
	for(int i=0; i<num; i++){
		check[i] = 0;
	}

	sched_setattr(getpid(), &attr, 0);
		

	for(int j=0; j<num; j++){

		pids = fork();		//fork를 진행한다.

							
		if(pids == 0 && check[j] == 0) {
			check[j]++;
			printf("Creating Process: #%d\n", j);
			calc(t, j);
			break;		//break를 하지 않으면 그 다음에 fork를 할 때 프로세스가 두 개 생성되기 때문에 필수적이다.
		}
		else
			{				
				continue;  //부모 프로세스의 경우 그냥 지나간다.
				}
	}
	
	return 0;

}