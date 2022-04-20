#include <linux/futex.h>

int main()
{
	static_assert(FUTEX_PRIVATE_FLAG == 128);

	return 1;
}
