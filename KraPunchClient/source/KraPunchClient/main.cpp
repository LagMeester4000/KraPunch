#include "KraPunchClient/StunClient.h"
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

int main()
{
	kra::stun::StunClient Client;
	
	std::cout << "Enter 0 for host and 1 for join" << std::endl;
	int H;
	std::cin >> H;

	if (H == 0)
	{
		// Host
		Client.StartHost();
		std::cout << "Host code: " << Client.GetSessionCode() << std::endl;
	}
	else if (H == 1)
	{
		// Join
		std::cout << "Enter the session code" << std::endl;
		uint32_t Code;
		std::cin >> Code;

		Client.StartJoin(Code);
	}

	while (!kbhit())
	{
		Client.Update();

		std::chrono::milliseconds sec(10);
		std::this_thread::sleep_for(sec);
	}

	return 1;
}