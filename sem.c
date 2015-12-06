/* when compile,should link lib pthread.
like this: $(CC) sem.c -o sem -lpthread
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/semaphore.h>// this is on mac,on ubuntu is #include <semaphore.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h> // sleep()


#define SECOND_USEC (1000000)
static sem_t * s;

void ossal_usleep(unsigned long usec)
{
    struct timeval tv;

    tv.tv_sec = (time_t) (usec / SECOND_USEC);
    tv.tv_usec = (long) (usec % SECOND_USEC);
    select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &tv);
}

int ossal_sem_take(sem_t*   b, int usec)
{
	sem_t *s = (sem_t *) b;
	int err;
	int time_wait = 1;

	/* Retry algorithm with exponential backoff */
	for (;;) {
		if (sem_trywait(s) == 0) {
			err = 0;
			break;
		}

		if (errno != EAGAIN && errno != EINTR) {
			err = errno;
			break;
		}

		if (time_wait > usec) {
			time_wait = usec;
		}

		ossal_usleep(time_wait);

		usec -= time_wait;

		if (usec == 0) {
			err = ETIMEDOUT;
			break;
		}

		if ((time_wait *= 2) > 100000) {
			time_wait = 100000;
		}
	}

	return err ? -1 : 0;
}

void * thr_fn(void *arg)
{
	printf("thread 1 start\n");
	
	while(1)
	{
		printf("像风一样自由\n");
		sleep(2);
		sem_post(s);
		break;
	}

	pthread_exit((void*)0);
}

int main (int argc, char *argv[])
{
	int ret;
	char *sem_name = "sem_file";
	pthread_t tid;

	s = sem_open(sem_name, O_CREAT, 0644, 0);
	if(s == SEM_FAILED)
	{
		perror("sem_open");
		exit(-1);
	}

	ret = pthread_create(&tid, NULL, thr_fn, NULL);
	if (ret != 0)
	{
		perror("pthread_create");
		exit(-1);
	}

	printf("start wait the sem\n");
	ret = ossal_sem_take(s, 5*1000*1000);
	printf("1.ret = %d\n", ret);

	sem_close(s);
	if(0 != sem_unlink(sem_name))
	{
		perror("sem_unlink");
	}

    return EXIT_SUCCESS;
}
