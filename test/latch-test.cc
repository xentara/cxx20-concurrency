#include <c++20-latch.h>

int main()
{
	std::latch latch(5);

	latch.count_down();
	latch.count_down(2);

	[[maybe_unused]] bool b = latch.try_wait();

	latch.arrive_and_wait(1);
	latch.wait();

	return 0;
}
