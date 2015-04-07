#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#define SEMPERM 0600
#define TRUE 1
#define FALSE 0
typedef struct union_semun {
             int val;
             struct semid_ds *buf;
             ushort *array;
             } semun;

int i, ret, number, lastNumber;
FILE *fp;

// 세마포어를 초기화하는 함수
int initsem (key_t semkey, int n)
{
   int status = 0, semid;
   if ((semid = semget (semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
   {
       if (errno == EEXIST)
                semid = semget (semkey, 1, 0);
   }
   else
   {
       semun arg;
       arg.val = n;
       status = semctl(semid, 0, SETVAL, arg);
   }
   if (semid == -1 || status == -1)
   {
       perror("initsem failed");
       return (-1);
   }
   return (semid);
}

// P() 연산 함수
int p (int semid)
{
struct sembuf p_buf;
p_buf.sem_num = 0;
p_buf.sem_op = -1;
p_buf.sem_flg = SEM_UNDO;
if (semop(semid, &p_buf, 1) == -1)
{
perror ("p(semid) failed");
exit(1);
}
return (0);
}

// V() 연산 함수
int v (int semid)
{
struct sembuf v_buf;
v_buf.sem_num = 0;
v_buf.sem_op = 1;
v_buf.sem_flg = SEM_UNDO;
if (semop(semid, &v_buf, 1) == -1)
{
perror ("v(semid) failed");
exit(1);
}
return (0);
}

// 파일의 마지막 숫자를 읽어서 1씩 증가하는 100개의 숫자를 파일에 기록하는 일을
// semaphore를 이용해 mutual exclusive하게 진행
void add1 (key_t skey)
{
   int semid;
   pid_t pid = getpid();
   if ((semid = initsem(skey,1)) < 0)
      exit(1);
   printf("\nprocess %d before critical section\n", pid);
   p(semid);
   printf("process %d in critical section\n",pid);
   
	/* 실제로 무언가 한다 */
   	/* 화일에서 읽어서 1 더하기 */
   
	// data.txt를 읽기모드로 연다.
	FILE *fp = fopen("data.txt", "rt");

	while(1)
	{
		ret = fscanf(fp, "%d ", &number);
		if(ret == EOF)	// 파일의 끝부분에 다다를 때까지 숫자를 읽어들임
			break;
		lastNumber = number; // 맨 끝의 숫자
	}
	fclose(fp);

	// data.txt를 닫기모드로 연다.
	fp = fopen("data.txt", "at");
	fputc('\n', fp);
	
	for(i = 0; i < 100; i++)	// lastNumber를 1씩 증가, 화일에 기록
	{
		lastNumber++;
		fprintf(fp, "%d ", lastNumber);
	}
	fclose(fp);
   
   printf("process %d leaving critical section\n", pid);
   v(semid);
   printf("process %d exiting\n",pid);
   exit(0);
}


main()
{
   key_t semkey = 0x200;
   int j;
   for (j=0; j<4; j++) // 함수를 4번 호출(semaphore를 4번 이용)
   {
      if (fork() == 0)
           add1(semkey);
   }
}