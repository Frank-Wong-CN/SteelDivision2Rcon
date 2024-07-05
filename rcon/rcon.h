#ifndef RCONCH_H
#define RCONCH_H

#include <cstdint>
#include <string>
#include <map>
#include <functional>
#include <condition_variable>
#include <thread>
#include <stop_token>

using namespace std::chrono_literals;

#include <winsock2.h>

#define RCON_PACKET_SIZE 4096u
#define RCON_HEADER_SIZE 8
#define RCON_BODY_SIZE RCON_PACKET_SIZE - RCON_HEADER_SIZE
#define RCON_SOCKET_TIMEOUT 4
#define RCON_IOTHREAD_POLLING_RATE 33ms

enum RconError : int32_t {
	RconAuthTimeout = 100,
	RconAuthInvalidCred = 101,
	RconSizeInvalid = 110,
	RconInternetProtocolNotSupported = 120
};

enum RconPacketType : int32_t {
	SERVERDATA_AUTH = 3,
	SERVERDATA_AUTH_RESPONSE = 2,
	SERVERDATA_EXECCOMMAND = 2,
	SERVERDATA_RESPONSE_VALUE = 0
};

class RconMessage {
	friend class RconClient;

    std::string m_Request;
    std::string m_Reply;
	// std::condition_variable m_ReplyReceived;
    int32_t m_ID;
    int32_t m_Errno;
	RconPacketType m_Type;
    bool m_Responded;
	bool m_MultiPacketEnd;

public:
	RconMessage():
		m_Request(),
		m_Reply(),
		m_ID(0),
		m_Errno(0),
		m_Type(RconPacketType::SERVERDATA_EXECCOMMAND),
		m_Responded(false),
		m_MultiPacketEnd(true) {}

    inline bool Responded() const { return m_Responded && m_MultiPacketEnd; }
    inline int32_t GetLastError() const { return m_Errno; }
	inline std::string Data() const { return m_Reply; }

	inline void WaitData() {
		// m_ReplyReceived.wait_for(mtx, RCON_SOCKET_TIMEOUT * 1000ms);
		uint32_t waited_for = 0;
		while (!(m_MultiPacketEnd && m_Responded) && waited_for < RCON_SOCKET_TIMEOUT * 1000) {
            waited_for += 100;
			std::this_thread::sleep_for(100ms);
		}
	}
};

class RconClient {

    std::map<int32_t, RconMessage> m_RequestIndex;

    std::string m_Addr;
	std::string m_AddrIP;
    std::string m_Password;
    std::function<void(std::string)> m_LogCallback;
	
	std::jthread m_IOThread;
	std::condition_variable m_SocketInited;

	RconMessage *m_AuthMessage;

    int32_t m_CurrentID;
	int32_t m_LastError;

    int m_SocketFd;
	int m_InetFamily;
    uint16_t m_Port;
    std::atomic<bool> m_Connected;
	std::atomic<bool> m_IOThreadStarted;

	union RconPacket {
		struct {
			char m_Buffer[RCON_PACKET_SIZE];
		} Raw;
		struct {
			// int32_t m_Size, but size is not included in the 4096 maximum packet size
			int32_t m_ID;
			int32_t m_Type;
		} Header;
		struct {
			uint32_t __padding[RCON_HEADER_SIZE / 4];
			char m_Data[RCON_BODY_SIZE];
		} Body;
	};

	inline int32_t SetLastError(int32_t error) { if (error != 0) m_LastError = error; return error; }

	int32_t CreateSocket();
	int32_t AuthRcon();
    int32_t SendRconMessage(RconMessage &msg, bool multi);

	void ThreadWorker(std::stop_token token); // Remember to notify the m_ReplyReceived conditional variable in the struct

public:
    RconClient(std::string addr, uint16_t port, std::string pass, std::function<void(std::string)> on_log);
    ~RconClient();

	RconClient(const RconClient &) = delete;
	RconClient &operator=(const RconClient &) = delete;

    inline bool IsConnected() { return m_Connected; }
	inline int32_t GetLastError() { int32_t tmp = m_LastError; m_LastError = 0; return tmp; }

    void DoneWithMessage(RconMessage &msg);

	void Reconnect(std::string pass);
    [[nodiscard]] RconMessage &Send(std::string command, bool multi = false, RconPacketType type = RconPacketType::SERVERDATA_EXECCOMMAND);
};

#endif // RCONCH_H
