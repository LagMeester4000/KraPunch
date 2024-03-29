#include "KraPunchClient/StunClient.h"
#include <chrono>
#include <thread>
#include <iostream>

sf::IpAddress ServerIp = "167.172.35.168";
uint16_t ServerPort = 31417;

void SmallSleep()
{
	std::chrono::milliseconds sec(10);
	std::this_thread::sleep_for(sec);
}

kra::stun::StunClient::StunClient()
	: SessionCode(0), Type(ClientType::None), State(ConnectionState::Disconnected)
{
}

void kra::stun::StunClient::StartHost()
{
	// Send initial packet to server
	OutPack.clear();
	uint32_t SendState = Host;
	OutPack << SendState;
	Sock.send(OutPack, ServerIp, ServerPort);
	State = ConnectionState::Connecting;
	SmallSleep();

	while (SessionCode == 0)
	{
		sf::IpAddress SIP;
		unsigned short SPort;
		if (Sock.receive(InPack, SIP, SPort) == sf::Socket::Status::Done)
		{
			uint32_t PType;
			InPack >> PType;
			if (PType == Host)
			{
				InPack >> SessionCode;
				State = ConnectionState::Hosting;
				break;
			}
		}

		// Send packet again
		OutPack.clear();
		SendState = Host;
		OutPack << SendState;
		Sock.send(OutPack, ServerIp, ServerPort);

		SmallSleep();
	}
	
	Sock.setBlocking(false);
}

void kra::stun::StunClient::StartJoin(uint32_t SessionCodee)
{
	SessionCode = SessionCodee;

	// Send initial packet to server
	OutPack.clear();
	uint32_t SendState = Join;
	OutPack << SendState;
	OutPack << SessionCode;
	Sock.send(OutPack, ServerIp, ServerPort);
	State = ConnectionState::Connecting;
	SmallSleep();

	while (!HasOtherAddress())
	{
		sf::IpAddress SIP;
		unsigned short SPort;
		if (Sock.receive(InPack, SIP, SPort) == sf::Socket::Status::Done)
		{
			uint32_t PType;
			InPack >> PType;
			if (PType == Join)
			{
				std::string IPStr;
				InPack >> IPStr;
				OtherIP = sf::IpAddress(IPStr.c_str());
				InPack >> OtherPort;
				State = ConnectionState::Joining;
				PrintOtherAddress();
				break;
			}
		}

		// Send packet again
		OutPack.clear();
		SendState = Join;
		OutPack << SendState;
		OutPack << SessionCode;
		Sock.send(OutPack, ServerIp, ServerPort);

		SmallSleep();
	}

	Sock.setBlocking(false);
}

void kra::stun::StunClient::Update()
{
	switch (State)
	{
	case ConnectionState::Hosting:
	{
		// Send KeepAlive packet
		{
			OutPack.clear();
			uint32_t SendState = KeepAlive;
			OutPack << SendState;
			OutPack << SessionCode;
			Sock.send(OutPack, ServerIp, ServerPort);
		}

		// Receive pack
		{
			sf::IpAddress SIP;
			unsigned short SPort;
			if (Sock.receive(InPack, SIP, SPort) == sf::Socket::Status::Done)
			{
				uint32_t RecState;
				InPack >> RecState;
				if (RecState == PacketType::KeepAlive)
				{
					// Nothing
				}
				else if (RecState == PacketType::UpdateHost)
				{
					std::string IPStr;
					InPack >> IPStr;
					OtherIP = sf::IpAddress(IPStr.c_str());
					InPack >> OtherPort;
					State = ConnectionState::Joining;
					PrintOtherAddress();
				}
			}
		}
	} break;
	case ConnectionState::Joining:
	{
		{
			// Send message to client
			OutPack.clear();
			uint32_t SendState = JoinClient;
			OutPack << SendState;
			Sock.send(OutPack, OtherIP, OtherPort);
		}

		{
			sf::IpAddress SIP;
			unsigned short SPort;
			if (Sock.receive(InPack, SIP, SPort) == sf::Socket::Status::Done)
			{
				uint32_t RecState;
				InPack >> RecState;
				if (RecState == JoinClient)
				{
					// SUCCESS
					State = ConnectionState::Connected;

					std::cout << "SUCCESS, connection between 2 clients has been established!" << std::endl;
				}
			}
		}
	} break;
	default:
		break;
	}

	SmallSleep();
}

bool kra::stun::StunClient::HasOtherAddress() const
{
	return State == ConnectionState::Joining || State == ConnectionState::Connected;
}

uint32_t kra::stun::StunClient::GetSessionCode() const
{
	return SessionCode;
}

void kra::stun::StunClient::PrintOtherAddress()
{
	std::cout << "Other client address: " << OtherIP.toString() << ":" << OtherPort << std::endl;
}
