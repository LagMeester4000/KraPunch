#include "KraPunch/StunServer.h"
#include <iostream>
#include <random>
#include <ctime>
#include <vector>

unsigned short ServerPort = 31417;

kra::stun::StunServer::StunServer()
{
}

bool kra::stun::StunServer::Init()
{
	if (Sock.bind(ServerPort) != sf::Socket::Status::Done)
	{
		std::cout << "Failed to bind socket" << std::endl;
		return false;
	}

	Sock.setBlocking(false);
	srand(time(NULL));
	return true;
}

void kra::stun::StunServer::Update()
{
	Address Add;
	while (Sock.receive(InPack, Add.IP, Add.Port) == sf::Socket::Status::Done)
	{
		uint32_t State;
		InPack >> State;
		switch (State)
		{
		case Host:
			OnPackHost(Add);
			break;
		case Join:
			OnPackJoin(Add);
			break;
		case KeepAlive:
			OnPackKeepAlive(Add);
			break;
		}
	}

	// Clean old packet
	Cleanup();
}

void kra::stun::StunServer::OnPackHost(Address Add)
{
	// First check if session already exists
	auto IpString = Add.IP.toString();

	// Send code back
	auto FindIp = IpToSession.find(IpString);
	if (FindIp != IpToSession.end())
	{
		// Session already exists
		OutPack.clear();
		uint32_t State = Host;
		OutPack << State;
		OutPack << FindIp->second;
		Sock.send(OutPack, Add.IP, Add.Port);

		std::cout << "Sent back already created Session Code {" << FindIp->second << "} to IP {" << IpString << "} and port {" << Add.Port << "}" << std::endl;
		return;
	}

	// Make new code
	{
		uint32_t NewCode = rand();
		while (Sessions.find(NewCode) != Sessions.end() || NewCode == 0)
		{
			NewCode = rand();
		}

		// Setup new element
		StunElement El;
		El.InitialAddress = Add;
		El.LastTime = std::chrono::high_resolution_clock::now();
		Sessions[NewCode] = El;
		IpToSession[IpString] = NewCode;

		// Send packet with code back
		OutPack.clear();
		uint32_t State = Host;
		OutPack << State;
		OutPack << NewCode;
		Sock.send(OutPack, Add.IP, Add.Port);

		std::cout << "Sent back new created Session Code {" << NewCode << "} to IP {" << IpString << "} and port {" << Add.Port << "}" << std::endl;
		return;
	}
}

void kra::stun::StunServer::OnPackJoin(Address Add)
{
	// Get the SessionCode
	uint32_t SessionCode;
	InPack >> SessionCode;

	// Check if it exists
	auto Find = Sessions.find(SessionCode);
	if (Find != Sessions.end())
	{
		// Found
		// Check if someone is already connected
		if (Find->second.ClientConnected)
		{
			if (Find->second.OtherAddress.IP == Add.IP &&
				Find->second.OtherAddress.Port == Add.Port)
			{
				// Resend packet because it might not have been received
				uint32_t State = Join;
				OutPack << State;
				std::string IPStr = Find->second.InitialAddress.IP.toString();
				OutPack << IPStr;
				OutPack << Find->second.InitialAddress.Port;
				Sock.send(OutPack, Add.IP, Add.Port);
			}
			else
			{
				// Error
				std::cout << "Session " << SessionCode << " already has a client connected and it is not " << Add.IP.toString() << ":" << Add.Port << std::endl;
			}

			return;
		}

		// Register new IP
		Find->second.ClientConnected = true;
		Find->second.OtherAddress = Add;

		{
			// Send pack to new client
			OutPack.clear();
			uint32_t State = Join;
			OutPack << State;
			std::string IPStr = Find->second.InitialAddress.IP.toString();
			OutPack << IPStr;
			OutPack << Find->second.InitialAddress.Port;
			Sock.send(OutPack, Add.IP, Add.Port);
		}

		{
			// Send pack to old client
			OutPack.clear();
			uint32_t State = UpdateHost;
			OutPack << State;
			std::string IPStr = Add.IP.toString();
			OutPack << IPStr;
			OutPack << Add.Port;
			Sock.send(OutPack, Find->second.InitialAddress.IP, Find->second.InitialAddress.Port);
		}
		return;
	}
}

void kra::stun::StunServer::OnPackKeepAlive(Address Add)
{
	// Get the SessionCode
	uint32_t SessionCode;
	InPack >> SessionCode;

	// Check if it exists
	auto Find = Sessions.find(SessionCode);
	if (Find != Sessions.end())
	{
		Find->second.LastTime = std::chrono::high_resolution_clock::now();

		// Check if someone is already connected
		if (Find->second.ClientConnected)
		{
			// Send back UpdateHost packet
			OutPack.clear();
			uint32_t State = UpdateHost;
			OutPack << State;
			std::string IPStr = Find->second.OtherAddress.IP.toString();
			OutPack << IPStr;
			OutPack << Find->second.OtherAddress.Port;
			Sock.send(OutPack, Add.IP, Add.Port);
			return;
		}

		OutPack.clear();
		uint32_t State = KeepAlive;
		OutPack << State;
		Sock.send(OutPack, Add.IP, Add.Port);
		return;
	}
}

void kra::stun::StunServer::OnPackDestroy(Address Add)
{
}

void kra::stun::StunServer::Cleanup()
{
	struct Ent {
		std::string s;
		uint32_t c;
	};
	std::vector<Ent> ToDestroy;

	// Find inactive sessions
	for (auto& It : IpToSession)
	{
		auto& Other = Sessions[It.second]; 
		if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - Other.LastTime).count() > 10)
		{
			ToDestroy.push_back(Ent{ It.first, It.second });
		}
	}

	// Destroy inactive sessions
	for (auto& It : ToDestroy)
	{
		std::cout << "Erased client with session ID: " << It.c << std::endl;
		IpToSession.erase(It.s);
		Sessions.erase(It.c);
	}
}
