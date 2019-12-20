#pragma once
#include <SFML/Network.hpp>
#include <map>
#include <string>
#include <stdint.h>
#include <chrono>

namespace kra {
	namespace stun {
		struct Address {
			unsigned short Port;
			sf::IpAddress IP;
		};

		struct StunElement {
			Address InitialAddress;
			Address OtherAddress;
			bool ClientConnected = false;
			std::chrono::time_point<std::chrono::high_resolution_clock> LastTime;
		};

		class StunServer {
		public:
			enum PacketType {
				// Host connects to stun server, sends back SessionCode uint32_t
				// Server receives: (u32 State)
				// Server sends: (u32 State, u32 SessionCode)
				Host, 

				// Client joins with SessionCode uint32_t
				// Server receives: (u32 State, u32 SessionCode)
				// Server sends: (u32 State, string IP, u16 Port)
				Join, 

				// Send new Joined address back to host
				// Server receives: not applicable
				// Server sends: (u32 State, string IP, u16 Port)
				UpdateHost, 

				// Send packet to keep connection alive. If UpdateHost Has been sent and this packet is still received, send back an UpdateHost packet
				// Server receives: (u32 State, u32 SessionCode)
				// Server sends: (u32 State) OR (u32 State, string IP, u16 Port)
				KeepAlive,

				//Destroy, // Receives SessionCode uint32_t and destroys it
			};
		public:
			StunServer();

			// Initialize the server
			bool Init();

			// Parse packets
			void Update();

		private:
			void OnPackHost(Address);
			void OnPackJoin(Address);
			void OnPackKeepAlive(Address);
			void OnPackDestroy(Address);

			void Cleanup();

		private: // Net
			sf::UdpSocket Sock;
			sf::Packet InPack, OutPack;

		private: // Database
			// Map with all the sessions
			std::map<uint32_t, StunElement> Sessions;

			// Maps an IP to a session ID
			std::map<std::string, uint32_t> IpToSession;
		};
	}
}