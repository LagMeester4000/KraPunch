#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <thread>

class ForceExitScopeTest {
public:
	ForceExitScopeTest() 
	{
		std::cout << "created" << std::endl;
	}

	~ForceExitScopeTest()
	{
		std::cout << "destroyed" << std::endl;
	}
};

int main()
{
	using namespace std::chrono;

	ForceExitScopeTest o;

	// runs for 20 seconds
	for (size_t i = 0; i < 20; ++i)
	{

		seconds sec(1);
		std::this_thread::sleep_for(sec);
	}

	return 1;
}