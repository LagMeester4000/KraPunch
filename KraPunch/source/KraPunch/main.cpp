#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#if _WIN32
#include<stdio.h>
#include<conio.h>
#elif __linux__
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}
#endif


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
	
	// Run until any key is pressed
	while (!kbhit())
	{

		seconds sec(1);
		std::this_thread::sleep_for(sec);
	}

	return 1;
}