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

// ������� �ʱ�ȭ�ϴ� �Լ�
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

// P() ���� �Լ�
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

// V() ���� �Լ�
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

// ������ ������ ���ڸ� �о 1�� �����ϴ� 100���� ���ڸ� ���Ͽ� ����ϴ� ����
// semaphore�� �̿��� mutual exclusive�ϰ� ����
void add1 (key_t skey)
{
   int semid;
   pid_t pid = getpid();
   if ((semid = initsem(skey,1)) < 0)
      exit(1);
   printf("\nprocess %d before critical section\n", pid);
   p(semid);
   printf("process %d in critical section\n",pid);
   
	/* ������ ���� �Ѵ� */
   	/* ȭ�Ͽ��� �о 1 ���ϱ� */
   
	// data.txt�� �б���� ����.
	FILE *fp = fopen("data.txt", "rt");

	while(1)
	{
		ret = fscanf(fp, "%d ", &number);
		if(ret == EOF)	// ������ ���κп� �ٴٸ� ������ ���ڸ� �о����
			break;
		lastNumber = number; // �� ���� ����
	}
	fclose(fp);

	// data.txt�� �ݱ���� ����.
	fp = fopen("data.txt", "at");
	fputc('\n', fp);
	
	for(i = 0; i < 100; i++)	// lastNumber�� 1�� ����, ȭ�Ͽ� ���
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
   for (j=0; j<4; j++) // �Լ��� 4�� ȣ��(semaphore�� 4�� �̿�)
   {
      if (fork() == 0)
           add1(semkey);
   }
}