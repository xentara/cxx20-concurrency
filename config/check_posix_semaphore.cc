#include <unistd.h>
#include <semaphore.h>
#include <limits.h>

int main()
{
    #if !defined _POSIX_TIMEOUTS || _POSIX_TIMEOUTS <= 0
    #	error "POSIX Timeouts option not supported"
    #elif !defined _POSIX_SEMAPHORES || _POSIX_SEMAPHORES <= 0
    #	error "POSIX Semaphores option not supported"
    #else
    #	if defined SEM_VALUE_MAX
    constexpr int sem_value_max = SEM_VALUE_MAX;
    #	elif defined _POSIX_SEM_VALUE_MAX
    constexpr int sem_value_max = _POSIX_SEM_VALUE_MAX;
    #	else
    #		error "SEM_VALUE_MAX not available"
    #	endif
    sem_t sem;
    sem_init(&sem, 0, sem_value_max);
    struct timespec ts = { 0 };
    sem_timedwait(&sem, &ts);
    #endif
    
	return 1;
}
