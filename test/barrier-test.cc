#include <c++20-barrier.h>

int main()
{
	std::barrier<> barrier(5);

	auto a1 = barrier.arrive();
	auto a2 = barrier.arrive(2);
	barrier.arrive_and_drop();
	barrier.arrive_and_wait();

	barrier.wait(std::move(a1));
	barrier.wait(std::move(a2));

	return 0;
}
