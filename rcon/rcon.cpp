#include "rcon.h"

#include <ws2tcpip.h>

#include <regex>
#include <format>

static inline char *GetStringError(unsigned int code) {
	char *s = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&s, 0, NULL);
	return s;
}

#define WSAERRNO _wsaerrno

#define WSAERRFATAL(X, L, C, R) if (X) { \
	int WSAERRNO = WSAGetLastError(); \
	if (WSAERRNO != WSAEWOULDBLOCK) { \
		L(std::format("[WSA ERROR ({}:{})] {}: {}", __FILE__, __LINE__, WSAERRNO, GetStringError(WSAERRNO))); \
		C; \
		return R; \
	} \
}

#define WSAERR(X, L, C) if (X) { \
	int WSAERRNO = WSAGetLastError(); \
	if (WSAERRNO != WSAEWOULDBLOCK) { \
		L(std::format("[WSA ERROR ({}:{})] {}: {}", __FILE__, __LINE__, WSAERRNO, GetStringError(WSAERRNO))); \
		C; \
	} \
}

int32_t RconClient::CreateSocket() {
	std::regex ipv6_regex("^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]).){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]).){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$");
	std::regex ipv4_regex("^((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])$");

	m_AddrIP.clear();
	
	if (std::regex_match(m_Addr, ipv4_regex)) {
		m_InetFamily = AF_INET;
		m_AddrIP = m_Addr;
	}
	else if (std::regex_match(m_Addr, ipv6_regex)) {
		m_InetFamily = AF_INET6;
		m_AddrIP = m_Addr;
	}
	else {
		struct addrinfo *result = nullptr;
		struct sockaddr_in *sockaddr = nullptr;
		char ipaddrbuffer[128] = { 0 };
		unsigned long ipaddrbufferlen = 128;

		WSAERRFATAL(0 != getaddrinfo(m_Addr.c_str(), NULL, NULL, &result), m_LogCallback,, WSAERRNO);

		m_InetFamily = result->ai_family;
		if (m_InetFamily == AF_INET) {
			m_AddrIP.append(inet_ntoa(((struct sockaddr_in *)result->ai_addr)->sin_addr));
		}
		else if (m_InetFamily == AF_INET6) {
			WSAERRFATAL(0 != WSAAddressToStringA(result->ai_addr, result->ai_addrlen, NULL, ipaddrbuffer, &ipaddrbufferlen), m_LogCallback,, WSAERRNO);
			m_AddrIP.append(ipaddrbuffer);
		}
		else {
			m_LogCallback("[ERROR] Host uses an unsupported Internet Protocol.");
			return RconError::RconInternetProtocolNotSupported;
		}

		freeaddrinfo(result);
	}

	struct sockaddr *sock_addr = nullptr;
	size_t sock_addr_len = 0;
	struct sockaddr_in sock_addr_in4 = { 0 };
	struct sockaddr_in6 sock_addr_in6 = { 0 };

	if (m_InetFamily == AF_INET) {
		sock_addr = (struct sockaddr *)&sock_addr_in4;
		sock_addr_in4.sin_family = AF_INET;
		sock_addr_in4.sin_port = htons(m_Port);
		sock_addr_len = sizeof(sock_addr_in4);
		sock_addr_in4.sin_addr.s_addr = inet_addr(m_AddrIP.c_str());
	}
	else {
		sock_addr = (struct sockaddr *)&sock_addr_in6;
		sock_addr_in6.sin6_family = AF_INET6;
		sock_addr_in6.sin6_port = htons(m_Port);
		sock_addr_len = sizeof(sock_addr_in6);
		inet_pton(m_InetFamily, m_AddrIP.c_str(), (void *)sock_addr);
	}

	m_SocketFd = socket(m_InetFamily, SOCK_STREAM, 0);
	WSAERRFATAL(m_SocketFd < 0, m_LogCallback,, WSAERRNO);

    u_long nonblock = 1;
    WSAERRFATAL(0 != ioctlsocket(m_SocketFd, FIONBIO, &nonblock), m_LogCallback, closesocket(m_SocketFd), WSAERRNO);

    uint32_t timeout = RCON_SOCKET_TIMEOUT * 1000;
    setsockopt(m_SocketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(m_SocketFd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    fd_set connectSet, errSet;
    FD_ZERO(&connectSet);
    FD_ZERO(&errSet);
    FD_SET(m_SocketFd, &connectSet);
    FD_SET(m_SocketFd, &errSet);

    WSAERRFATAL(connect(m_SocketFd, sock_addr, sock_addr_len), m_LogCallback, closesocket(m_SocketFd), WSAERRNO);

    TIMEVAL tv;
    tv.tv_sec = RCON_SOCKET_TIMEOUT;
    tv.tv_usec = 0;
    select(0, NULL, &connectSet, &errSet, &tv);
    WSAERRFATAL(FD_ISSET(m_SocketFd, &errSet), m_LogCallback, closesocket(m_SocketFd), WSAERRNO);

	return 0;
}

/*
int32_t RconClient::CreateSocket() {
	m_InetFamily = AF_INET;
	struct sockaddr *sock_addr = nullptr;
	size_t sock_addr_len = 0;
	struct sockaddr_in sock_addr_in4 = { 0 };

	sock_addr = (struct sockaddr *)&sock_addr_in4;
	sock_addr_in4.sin_addr.s_addr = inet_addr(m_Addr.c_str());
	sock_addr_in4.sin_family = m_InetFamily;
	sock_addr_in4.sin_port = htons(m_Port);
	sock_addr_len = sizeof(sock_addr_in4);
	// inet_pton(m_InetFamily, m_Addr.c_str(), (void *)sock_addr);

	m_SocketFd = socket(m_InetFamily, SOCK_STREAM, 0);
	WSAERRFATAL(m_SocketFd < 0, m_LogCallback,, WSAERRNO);

	u_long nonblock = 0;
	WSAERRFATAL(0 != ioctlsocket(m_SocketFd, FIONBIO, &nonblock), m_LogCallback,, WSAERRNO);

	uint32_t timeout = RCON_SOCKET_TIMEOUT * 1000;
	setsockopt(m_SocketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	WSAERRFATAL(connect(m_SocketFd, sock_addr, sock_addr_len), m_LogCallback,, WSAERRNO);

	return 0;
}
*/

int32_t RconClient::AuthRcon() {
	if (m_AuthMessage) {
		delete m_AuthMessage;
	}

	RconMessage *msg = new RconMessage();
	msg->m_ID = m_CurrentID++;
	msg->m_Request = m_Password;
	msg->m_Type = RconPacketType::SERVERDATA_AUTH;
	m_AuthMessage = msg;
	
	SetLastError(SendRconMessage(*m_AuthMessage, false));
	if (msg->m_Errno != 0) {
		return msg->m_Errno;
	}
	
	msg->WaitData();
	if (!msg->m_Responded) {
        m_LogCallback("[ERROR] Authentication timed out.");
        delete m_AuthMessage;
        m_AuthMessage = nullptr;
		return RconError::RconAuthTimeout;
	}

	RconPacket *packet = (RconPacket *)msg->m_Reply.c_str();
	if (packet->Header.m_ID == -1) {
        m_LogCallback("[ERROR] Authentication failed.");
        delete m_AuthMessage;
        m_AuthMessage = nullptr;
		return RconError::RconAuthInvalidCred;
	}

    m_Connected = true;

    delete m_AuthMessage;
    m_AuthMessage = nullptr;
	return 0;
}

int32_t RconClient::SendRconMessage(RconMessage &msg, bool multi) {
	RconPacket packet;
	memset(packet.Raw.m_Buffer, 0, RCON_PACKET_SIZE);
	packet.Header.m_ID = msg.m_ID;
	packet.Header.m_Type = msg.m_Type;
	// Rcon packet has at least two null-terminator at the end, thus plus 2
	uint32_t size = RCON_HEADER_SIZE + std::min(strlen(msg.m_Request.c_str()), (size_t)RCON_BODY_SIZE - 2) + 2;
	strncpy(packet.Body.m_Data, msg.m_Request.c_str(), size);

	WSAERRFATAL(0 > send(m_SocketFd, (const char *)&size, 4, 0), m_LogCallback, msg.m_Errno = WSAERRNO, WSAERRNO);
	WSAERRFATAL(0 > send(m_SocketFd, packet.Raw.m_Buffer, size, 0), m_LogCallback, msg.m_Errno = WSAERRNO, WSAERRNO);
    // printf("Send packet:\n");
    // Mem16(packet.Raw.m_Buffer, size);

	if (multi)
	{
		// Multi-packet
		msg.m_MultiPacketEnd = false;

		RconPacket packet;
		memset(packet.Raw.m_Buffer, 0, RCON_PACKET_SIZE);
		packet.Header.m_ID = msg.m_ID;
		packet.Header.m_Type = RconPacketType::SERVERDATA_RESPONSE_VALUE;
		uint32_t size = RCON_HEADER_SIZE + 2;

		WSAERRFATAL(0 > send(m_SocketFd, (const char *)&size, 4, 0), m_LogCallback, msg.m_Errno = WSAERRNO, WSAERRNO);
		WSAERRFATAL(0 > send(m_SocketFd, packet.Raw.m_Buffer, size, 0), m_LogCallback, msg.m_Errno = WSAERRNO, WSAERRNO);

        // printf("Send follow-up packet:\n");
        // Mem16(packet.Raw.m_Buffer, size);
	}

	return 0;
}

void RconClient::ThreadWorker(std::stop_token token) {
	std::mutex mtx;
	std::unique_lock lock(mtx);
	m_IOThreadStarted = true;
	m_SocketInited.wait(lock);

	while (!token.stop_requested()) {
		std::this_thread::sleep_for(RCON_IOTHREAD_POLLING_RATE);

		u_long available;
		ioctlsocket(m_SocketFd, FIONREAD, &available);
		if (available <= 0) continue;

		int32_t size;
		WSAERR(0 > recv(m_SocketFd, (char *)&size, 4, 0), m_LogCallback, SetLastError(WSAERRNO); continue);
		if (size < 10 || size > 4096) {
			m_LogCallback(std::format("[RCON Error] Received invalid size of {}.", size));
			continue;
		}
		RconPacket packet;
		WSAERR(0 > recv(m_SocketFd, packet.Raw.m_Buffer, size, 0), m_LogCallback, SetLastError(WSAERRNO); continue);

        // printf("Receive packet:\n");
        // Mem16(packet.Raw.m_Buffer, size);

		int32_t id = packet.Header.m_ID;
		// TODO: Change m_AuthMessage to m_AuthMessageID
		if (m_AuthMessage && !m_AuthMessage->m_Responded && m_AuthMessage->m_ID == id) {
			// Auth successful
			m_AuthMessage->m_Type = static_cast<RconPacketType>(packet.Header.m_Type);
			m_AuthMessage->m_Responded = true;
			continue;
		}
		if (id == -1) {
			// Auth failure
			if (!m_AuthMessage) {
				m_LogCallback("[RCON Error] Authentication failed, and the authentication request message is not found. This is an internal error.");
				continue;
			}
			m_AuthMessage->m_ID = -1;
			m_AuthMessage->m_Type = static_cast<RconPacketType>(packet.Header.m_Type);
			m_AuthMessage->m_Responded = true;
			// m_AuthMessage->m_ReplyReceived.notify_all();
			continue;
		}

        if (!m_RequestIndex.contains(id)) {
            if (m_AuthMessage && m_AuthMessage->m_ID != id) {
                m_LogCallback(std::format("[RCON Error] Request ID {} matches no request in the memory. The rest of the packet will be discarded.", id));
            }
			continue;
		}

		RconMessage &msg = m_RequestIndex[id];
		if (msg.m_MultiPacketEnd && msg.m_Responded) {
			continue;
		}

		int32_t type = packet.Header.m_Type;
		if (type != 0 && type != 2 && type != 3) {
			m_LogCallback(std::format("[RCON Error] Request ID {} received packet of invalid type {}. The rest of the packet will be discarded.", id, type));
			msg.m_Responded = true;
			msg.m_MultiPacketEnd = true;
			// msg.m_ReplyReceived.notify_all();
			continue;
		}

		msg.m_Type = static_cast<RconPacketType>(type);
		if (type == RconPacketType::SERVERDATA_AUTH_RESPONSE) {
			for (int32_t i = 0; i < size; ++i) {
				msg.m_Reply.push_back(packet.Raw.m_Buffer[i]);
			}
			msg.m_Responded = true;
			msg.m_MultiPacketEnd = true;
			// msg.m_ReplyReceived.notify_all();
			continue;
		}
		else if (type == RconPacketType::SERVERDATA_RESPONSE_VALUE) {
			if (size == 10) {
				msg.m_MultiPacketEnd = true;
				// msg.m_ReplyReceived.notify_all();
				continue;
			}
			for (int32_t i = 0; i < size - RCON_HEADER_SIZE; ++i) {
				msg.m_Reply.push_back(packet.Body.m_Data[i]);
			}
			msg.m_Responded = true;
		}
	}
}

RconClient::RconClient(std::string addr, uint16_t port, std::string pass, std::function<void(std::string)> on_log):
    m_Addr(addr),
	m_Port(port),
	m_Password(pass),
	m_LogCallback(on_log),
	m_IOThread(std::bind(&RconClient::ThreadWorker, this, std::placeholders::_1)),
	m_AuthMessage(nullptr),
	m_CurrentID(0),
	m_LastError(0),
	m_SocketFd(0),
	m_Connected(false),
	m_IOThreadStarted(false) {

    if (0 != SetLastError(CreateSocket())) return;
	while (!m_IOThreadStarted) {
		std::this_thread::sleep_for(50ms);
	}
	m_SocketInited.notify_all();

	if (0 != SetLastError(AuthRcon())) return;
}

RconClient::~RconClient() {
    if (m_AuthMessage) {
        delete m_AuthMessage;
    }
	m_IOThread.request_stop();
	m_IOThread.join();
    closesocket(m_SocketFd);
}

void RconClient::DoneWithMessage(RconMessage &msg) {
    if (m_RequestIndex.contains(msg.m_ID)) {
        m_RequestIndex.erase(msg.m_ID);
    }
}

void RconClient::Reconnect(std::string pass) {
	m_Password = pass;
	SetLastError(AuthRcon());
}

RconMessage &RconClient::Send(std::string command, bool multi, RconPacketType type) {
    while (m_RequestIndex.contains(m_CurrentID)) ++m_CurrentID;
	m_RequestIndex.insert(std::make_pair(m_CurrentID, RconMessage()));
	RconMessage &ref = m_RequestIndex[m_CurrentID];
	ref.m_ID = m_CurrentID;
	ref.m_Type = type;
	ref.m_Request = command;
	m_CurrentID += 1;
    if (m_CurrentID < 0) m_CurrentID = 0;
    if (m_AuthMessage && m_CurrentID == m_AuthMessage->m_ID) m_CurrentID += 1;

    SetLastError(SendRconMessage(ref, multi));

	return ref;
}
