#include <pthread.h>

int main()
{

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	
	struct timespec ts;
	int n = pthread_cond_clockwait(&cond, &mutex, 0, &ts);

	return 1;
}
