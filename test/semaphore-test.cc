#include <c++20-semaphore.h>

#include <chrono>

int main()
{
	using namespace std::literals;

	std::counting_semaphore<> semaphore(20);

	semaphore.release(10);
	semaphore.release();

	semaphore.acquire();
	semaphore.try_acquire();
	semaphore.try_acquire_for(10s);
	semaphore.try_acquire_for(1min);

	semaphore.try_acquire_until(std::chrono::system_clock::now() + 10s);
	semaphore.try_acquire_until(std::chrono::system_clock::now() + 1min);
	semaphore.try_acquire_until(std::chrono::steady_clock::now() + 10s);
	semaphore.try_acquire_until(std::chrono::steady_clock::now() + 1min);

	return 0;
}
