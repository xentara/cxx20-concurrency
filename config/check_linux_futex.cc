#include <sys/syscall.h>
#include <unistd.h>

int main()
{
	int dummy;

	syscall(SYS_gettid);
	syscall(SYS_futex, &dummy, 0, 0, 0);

	return 1;
}
