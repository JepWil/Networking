// networking.cc

#define _CRT_SECURE_NO_WARNINGS 1
#include "networking.h"

#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

time time::now() {
   static LARGE_INTEGER start = {};
   static int64 factor = 0;
   if (!factor)
   {
      LARGE_INTEGER f = {};
      QueryPerformanceFrequency(&f);
      factor = f.QuadPart / 1000;
      QueryPerformanceCounter(&start);
   }

   LARGE_INTEGER now = {};
   QueryPerformanceCounter(&now);

   return time((now.QuadPart - start.QuadPart) / factor);
}

time::time()
   : tick_(0)
{
}

time::time(int64 tick)
   : tick_(tick)
{
}

time time::operator+(const time &rhs) {
   return time(tick_ + rhs.tick_);
}

time time::operator-(const time &rhs) {
   return time(tick_ - rhs.tick_);
}

float time::as_seconds() const {
   return tick_ * 0.001f;
}

float time::as_milliseconds() const {
   return tick_ * 1.0f;
}

namespace network {
	sockaddr_in to_sockaddr(const ip_address &addr) {
      sockaddr_in result = {};
      result.sin_family = AF_INET;
      result.sin_addr.s_addr = htonl(addr.host_);
      result.sin_port = htons(addr.port_);
      return result;
   }

   ip_address from_sockaddr(const sockaddr_in &addr) {
      return ip_address(ntohl(addr.sin_addr.s_addr), htons(addr.sin_port));
   }

   bool init() {
      WSADATA data = {};
      int result = WSAStartup(MAKEWORD(2, 2), &data);
      return result == 0;
   }

   void shut() {
      WSACleanup();
   }

   namespace error {
      network_error_code get_error() {
         int error_code = WSAGetLastError();
         switch (error_code) {
            case 0:
               return NETERR_NO_ERROR;
            case WSA_INVALID_HANDLE:
               return NETERR_INVALID_HANDLE;
            case WSA_NOT_ENOUGH_MEMORY:
               return NETERR_NOT_ENOUGH_MEMORY;
            case WSA_INVALID_PARAMETER:
               return NETERR_INVALID_PARAMETER;
            case WSA_OPERATION_ABORTED:
               return NETERR_OPERATION_ABORTED;
            case WSAEINTR:
               return NETERR_INTERUPTED_CALL;
            case WSAEBADF:
               return NETERR_BAD_FILE_HANDLE;
            case WSAEACCES:
               return NETERR_SOCKET_ACCESS_DENIED;
            case WSAEFAULT:
               return NETERR_BAD_ADDRESS;
            case WSAEINVAL:
               return NETERR_INVALID_ARGUMENT;
            case WSAEMFILE:
               return NETERR_TOO_MANY_OPEN_FILES;
            case WSAEWOULDBLOCK:
               return NETERR_WOULD_BLOCK;
            case WSAEINPROGRESS:
               return NETERR_IN_PROGRESS;
            case WSAEALREADY:
               return NETERR_ALREADY_IN_PROGRESS;
            case WSAENOTSOCK:
               return NETERR_HANDLE_NON_SOCKET;
            case WSAEDESTADDRREQ:
               return NETERR_DESTINATION_ADDRESS_REQUIRED;
            case WSAEMSGSIZE:
               return NETERR_MESSAGE_TOO_LONG;
            case WSAEPROTOTYPE:
               return NETERR_WRONG_PROTOTYPE;
            case WSAENOPROTOOPT:
               return NETERR_BAD_PROTOCOL_OPTION;
            case WSAEPROTONOSUPPORT:
               return NETERR_PROTOCOL_NOT_SUPPORTED;
            case WSAESOCKTNOSUPPORT:
               return NETERR_SOCKET_TYPE_NOT_SUPPORTED;
            case WSAEOPNOTSUPP:
               return NETERR_OPERATION_NOT_SUPPORTED;
            case WSAEPFNOSUPPORT:
               return NETERR_PROTOCOL_FAMILY_NOT_SUPPORT;
            case WSAEAFNOSUPPORT:
               return NETERR_ADDRESS_FAMILY_NOT_SUPPORT;
            case WSAEADDRINUSE:
               return NETERR_ADDRESS_IN_USE;
            case WSAEADDRNOTAVAIL:
               return NETERR_ADDRESS_NOT_AVAILABLE;
            case WSAENETDOWN:
               return NETERR_NETWORK_DOWN;
            case WSAENETUNREACH:
               return NETERR_NETWORK_UNREACHABLE;
            case WSAENETRESET:
               return NETERR_NETWORK_DROPPED_CONNECTION;
            case WSAECONNABORTED:
               return NETERR_CONNECTION_RESET_BY_SOFTWARE;
            case WSAECONNRESET:
               return NETERR_CONNECTION_RESET_BY_PEER;
            case WSAENOBUFS:
               return NETERR_NO_BUFFER_SPACE_AVAIABLE;
            case WSAEISCONN:
               return NETERR_ALREADY_CONNECTED;
            case WSAENOTCONN:
               return NETERR_NOT_CONNECTED;
            case WSAESHUTDOWN:
               return NETERR_SEND_SHUTDOWN;
            case WSAETOOMANYREFS:
               return NETERR_TOO_MANY_REFS;
            case WSAETIMEDOUT:
               return NETERR_CONNECTION_TIMED_OUT;
            case WSAECONNREFUSED:
               return NETERR_CONNECTION_REFUSED;
            case WSAELOOP:
               return NETERR_TRANSLATE_NAME;
            case WSAENAMETOOLONG:
               return NETERR_NAME_TOO_LONG;
            case WSAEHOSTDOWN:
               return NETERR_HOST_DOWN;
            case WSAEHOSTUNREACH:
               return NETERR_HOST_UNREACHABLE;
            case WSAEPROCLIM:
               return NETERR_PROCESS_LIMIT;
            case WSASYSNOTREADY:
               return NETERR_NETWORK_SYSTEM_NOT_READY;
            case WSAVERNOTSUPPORTED:
               return NETERR_VERSION_NOT_SUPPORTED;
            case WSANOTINITIALISED:
               return NETERR_NETWORK_NOT_INITIALISED;
            case WSAEDISCON:
               return NETERR_DISCONNECT_IN_PROGRESS;
            case WSAEINVALIDPROCTABLE:
               return NETERR_INVALIDPROCTABLE;
            case WSAEINVALIDPROVIDER:
               return NETERR_INVALIDPROVIDER;
            case WSAEPROVIDERFAILEDINIT:
               return NETERR_PROVIDERFAILEDINIT;
            case WSASYSCALLFAILURE:
               return NETERR_SYSCALLFAILURE;
            case WSAHOST_NOT_FOUND:
               return NETERR_HOST_NOT_FOUND;
            case WSATRY_AGAIN:
               return NETERR_TRY_AGAIN;
            case WSANO_RECOVERY:
               return NETERR_NO_RECOVERY;
            case WSANO_DATA:
               return NETERR_NO_DATA;
         }

         return NETERR_UNKNOWN;
      }

      const char *as_string(network_error_code error_code) {
         switch (error_code) {
            case NETERR_NO_ERROR:
               return "NETERR_NO_ERROR";
            case NETERR_INVALID_HANDLE:
               return "NETERR_INVALID_HANDLE";
            case NETERR_NOT_ENOUGH_MEMORY:
               return "NETERR_NOT_ENOUGH_MEMORY";
            case NETERR_INVALID_PARAMETER:
               return "NETERR_INVALID_PARAMETER";
            case NETERR_OPERATION_ABORTED:
               return "NETERR_OPERATION_ABORTED";
            case NETERR_INTERUPTED_CALL:
               return "NETERR_INTERUPTED_CALL";
            case NETERR_BAD_FILE_HANDLE:
               return "NETERR_BAD_FILE_HANDLE,";
            case NETERR_SOCKET_ACCESS_DENIED:
               return "NETERR_SOCKET_ACCESS_DENIED";
            case NETERR_BAD_ADDRESS:
               return "NETERR_BAD_ADDRESS";
            case NETERR_INVALID_ARGUMENT:
               return "NETERR_INVALID_ARGUMENT";
            case NETERR_TOO_MANY_OPEN_FILES:
               return "NETERR_TOO_MANY_OPEN_FILES";
            case NETERR_WOULD_BLOCK:
               return "NETERR_WOULD_BLOCK";
            case NETERR_IN_PROGRESS:
               return "NETERR_IN_PROGRESS";
            case NETERR_ALREADY_IN_PROGRESS:
               return "NETERR_ALREADY_IN_PROGRESS";
            case NETERR_HANDLE_NON_SOCKET:
               return "NETERR_HANDLE_NON_SOCKET";
            case NETERR_DESTINATION_ADDRESS_REQUIRED:
               return "NETERR_DESTINATION_ADDRESS_REQUIRED";
            case NETERR_MESSAGE_TOO_LONG:
               return "NETERR_MESSAGE_TOO_LONG";
            case NETERR_WRONG_PROTOTYPE:
               return "NETERR_WRONG_PROTOTYPE";
            case NETERR_BAD_PROTOCOL_OPTION:
               return "NETERR_BAD_PROTOCOL_OPTION";
            case NETERR_PROTOCOL_NOT_SUPPORTED:
               return "NETERR_PROTOCOL_NOT_SUPPORTED";
            case NETERR_SOCKET_TYPE_NOT_SUPPORTED:
               return "NETERR_SOCKET_TYPE_NOT_SUPPORTED";
            case NETERR_OPERATION_NOT_SUPPORTED:
               return "NETERR_OPERATION_NOT_SUPPORTED";
            case NETERR_PROTOCOL_FAMILY_NOT_SUPPORT:
               return "NETERR_PROTOCOL_FAMILY_NOT_SUPPORT";
            case NETERR_ADDRESS_FAMILY_NOT_SUPPORT:
               return "NETERR_ADDRESS_FAMILY_NOT_SUPPORT";
            case NETERR_ADDRESS_IN_USE:
               return "NETERR_ADDRESS_IN_USE";
            case NETERR_ADDRESS_NOT_AVAILABLE:
               return "NETERR_ADDRESS_NOT_AVAILABLE";
            case NETERR_NETWORK_DOWN:
               return "NETERR_NETWORK_DOWN";
            case NETERR_NETWORK_UNREACHABLE:
               return "NETERR_NETWORK_UNREACHABLE";
            case NETERR_NETWORK_DROPPED_CONNECTION:
               return "NETERR_NETWORK_DROPPED_CONNECTION";
            case NETERR_CONNECTION_RESET_BY_SOFTWARE:
               return "NETERR_CONNECTION_RESET_BY_SOFTWARE";
            case NETERR_CONNECTION_RESET_BY_PEER:
               return "NETERR_CONNECTION_RESET_BY_PEER";
            case NETERR_NO_BUFFER_SPACE_AVAIABLE:
               return "NETERR_NO_BUFFER_SPACE_AVAIABLE";
            case NETERR_ALREADY_CONNECTED:
               return "NETERR_ALREADY_CONNECTED";
            case NETERR_NOT_CONNECTED:
               return "NETERR_NOT_CONNECTED";
            case NETERR_SEND_SHUTDOWN:
               return "NETERR_SEND_SHUTDOWN";
            case NETERR_TOO_MANY_REFS:
               return "NETERR_TOO_MANY_REFS";
            case NETERR_CONNECTION_TIMED_OUT:
               return "NETERR_CONNECTION_TIMED_OUT";
            case NETERR_CONNECTION_REFUSED:
               return "NETERR_CONNECTION_REFUSED";
            case NETERR_TRANSLATE_NAME:
               return "NETERR_TRANSLATE_NAME";
            case NETERR_NAME_TOO_LONG:
               return "NETERR_NAME_TOO_LONG";
            case NETERR_HOST_DOWN:
               return "NETERR_HOST_DOWN";
            case NETERR_HOST_UNREACHABLE:
               return "NETERR_HOST_UNREACHABLE";
            case NETERR_PROCESS_LIMIT:
               return "NETERR_PROCESS_LIMIT";
            case NETERR_NETWORK_SYSTEM_NOT_READY:
               return "NETERR_NETWORK_SYSTEM_NOT_READY";
            case NETERR_VERSION_NOT_SUPPORTED:
               return "NETERR_VERSION_NOT_SUPPORTED";
            case NETERR_NETWORK_NOT_INITIALISED:
               return "NETERR_NETWORK_NOT_INITIALISED";
            case NETERR_DISCONNECT_IN_PROGRESS:
               return "NETERR_DISCONNECT_IN_PROGRESS";
            case NETERR_INVALIDPROCTABLE:
               return "NETERR_INVALIDPROCTABLE";
            case NETERR_INVALIDPROVIDER:
               return "NETERR_INVALIDPROVIDER";
            case NETERR_PROVIDERFAILEDINIT:
               return "NETERR_PROVIDERFAILEDINIT";
            case NETERR_SYSCALLFAILURE:
               return "NETERR_SYSCALLFAILURE";
            case NETERR_HOST_NOT_FOUND:
               return "NETERR_HOST_NOT_FOUND";
            case NETERR_TRY_AGAIN:
               return "NETERR_TRY_AGAIN";
            case NETERR_NO_RECOVERY:
               return "NETERR_NO_RECOVERY";
            case NETERR_NO_DATA:
               return "NETERR_NO_DATA";
            case NETERR_REMOTE_DISCONNECT:
               return "NETERR_REMOTE_DISCONNECT";
            case NETERR_SELECT_TIMEOUT:
               return "NETERR_SELECT_TIMEOUT";
         }

         return "NETERR_UNKNOWN";
      }

      bool is_non_critical(network_error_code error_code) {
         switch (error_code) {
            case NETERR_NO_ERROR:
            case NETERR_WOULD_BLOCK:
            case NETERR_SELECT_TIMEOUT:
               return true;
         }
         return false;
      }
   } // !error

   namespace util {
      bool message_box(const char *format, ...) {
         char message_[2048] = {};
         va_list args;
         va_start(args, format);
         vsprintf_s(message_, sizeof(message_), format, args);
         va_end(args);

         return MessageBoxA(NULL, message_, "Info", MB_OKCANCEL | MB_ICONINFORMATION) == IDOK;
      }
   } // !util
} // !network

ip_address::ip_address()
   : host_(0)
   , port_(0)
{
}

ip_address::ip_address(uint32 host, uint16 port)
   : host_(host)
   , port_(port)
{
}

ip_address::ip_address(uint8 a, uint8 b, uint8 c, uint8 d, uint16 port)
   : host_(0)
   , port_(port)
{
   host_ |= ((uint32)a << 24);
   host_ |= ((uint32)b << 16);
   host_ |= ((uint32)c << 8);
   host_ |= ((uint32)d << 0);
}

bool ip_address::operator==(const ip_address &rhs) const {
   return host_ == rhs.host_;
}

void ip_address::set_host(uint8 a, uint8 b, uint8 c, uint8 d) {
   host_  = 0;
   host_ |= ((uint32)a << 24);
   host_ |= ((uint32)b << 16);
   host_ |= ((uint32)c << 8);
   host_ |= ((uint32)d << 0);
}

void ip_address::set_port(uint16 port) {
   port_ = port;
}

bool ip_address::as_string(string &str) const {
   char buf[128] = {};
   sockaddr_in inaddr = network::to_sockaddr(*this);
   if (inet_ntop(AF_INET, &inaddr.sin_addr, buf, sizeof(buf))) {
      size_t len = strlen(buf);
      sprintf(buf + len, ":%d", port_);
      str = buf;
      return true;
   }
   return false;
}

tcp_socket::result::result(int32 length)
   : code_(network::error::get_error())
   , length_(length)
{
}

tcp_socket::result::result(network_error_code code)
   : code_(code)
   , length_(0)
{
}

bool tcp_socket::result::is_success() const {
   return code_ == NETERR_NO_ERROR;
}

bool tcp_socket::result::is_failure() const {
   return code_ != NETERR_NO_ERROR;
}

tcp_socket::tcp_socket()
   : handle_(~0u)
{
}

tcp_socket::tcp_socket(uint32 handle)
   : handle_(handle)
{
}

bool tcp_socket::is_valid() const {
   return handle_ != ~0u;
}

void tcp_socket::close() {
   if (!is_valid()) {
      return;
   }

   // note: disable both send and receive
   shutdown(handle_, SD_BOTH);

   // note: release the socket handle
   closesocket(handle_);

   handle_ = ~0u;
}

tcp_socket::result tcp_socket::connect(const ip_address &address) {
   if (is_valid()) {
      close();
   }

   // note: create a stream socket
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock == INVALID_SOCKET) {
      return result(0);
   }

   // note: windows uses SOCKET as the type for a socket descriptor
   //       it is safe to cast to int or even uint32 per the 
   //       documentation on microsoft docs.
   handle_ = (uint32)sock;

   // note: open a connection to remote address
   sockaddr_in remote = network::to_sockaddr(address);
   if (::connect(handle_, (const sockaddr *)& remote, sizeof(remote)) != 0) {
      return result(0);
   }

   // note: non-blocking mode
   u_long non_blocking = 1;
   if (ioctlsocket(handle_, FIONBIO, &non_blocking) != 0) {
      return result(0);
   }

   return result(0);
}


tcp_socket::result tcp_socket::send(const int32 size, const uint8 *src) {
	int32 sent_amount = 0;// the amount it has send over all
	int32 send_bytes = 0; // the amount it has sent from each ::send
	while (sent_amount < size) {
		send_bytes = ::send(handle_, (const char*)src + sent_amount, size - sent_amount, 0);
		if (send_bytes < 0) {
			return network::error::get_error();
		}
		sent_amount += send_bytes;
	}

	return result(send_bytes);
}

tcp_socket::result tcp_socket::receive(const int32 size, uint8 *dst) {
	int32 recive_amount = 0; // the amount it has recived over all
	int32 recive_bytes = 0;  // the amount it has recived from each recv
	int32 success_countdown = 2; // how many times the algoritme checks if it gets another part of the message

	while (recive_amount < success_countdown) {
		recive_bytes = ::recv(handle_, (char*)dst + recive_amount, size - recive_amount, 0);
		recive_amount += recive_bytes;

		if (recive_bytes == 0) {
			return result(NETERR_REMOTE_DISCONNECT);
		}
		else if (recive_amount < 0) {
			return  result(0);
		}
		if (recive_bytes > 0) {
			success_countdown = recive_amount - success_countdown;
		}
	}
	return result(network_error_code::NETERR_NO_ERROR);
}

tcp_listener::result::result()
   : code_(network::error::get_error())
{
}

tcp_listener::result::result(network_error_code code)
   : code_(code)
{
}

bool tcp_listener::result::is_success() const {
   return code_ == NETERR_NO_ERROR;
}

bool tcp_listener::result::is_failure() const {
   return code_ != NETERR_NO_ERROR;
}

tcp_listener::tcp_listener()
   : handle_(~0u)
{
}

tcp_listener::~tcp_listener() {
   close();
}

bool tcp_listener::open(const ip_address &address) {
   // note: create a stream socket
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock == INVALID_SOCKET) {
      return false;
   }

   // note: windows uses SOCKET as the type for a socket descriptor
   //       it is safe to cast to int or even uint32 per the 
   //       documentation on microsoft docs.
   handle_ = (uint32)sock;

   // note: disable blocking mode
   u_long non_blocking = 1;
   if (ioctlsocket(handle_, FIONBIO, &non_blocking) != 0) {
      return false;
   }

   // note: enable address reuse
   char reuse = TRUE;
   if (setsockopt(handle_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
      return false;
   }

   // note: bind socket to defined address
   sockaddr_in local = network::to_sockaddr(address);
   if (bind(handle_, (const sockaddr *)& local, sizeof(local)) != 0) {
      return false;
   }

   // note: start listening for new connections
   if (listen(handle_, SOMAXCONN) != 0) {
      return false;
   }

   return true;
}

void tcp_listener::close() {
   // note: release the socket handle
   closesocket(handle_);

   handle_ = ~0u;
}

tcp_listener::result tcp_listener::accept() {
   fd_set fds = {};
   FD_ZERO(&fds);
   FD_SET(handle_, &fds);
   timeval tv = {};
   tv.tv_usec = 100;
   int rval = select(handle_, &fds, NULL, NULL, &tv);
   if (rval == -1) {
      return result();
   }

   if (rval == 0) {
      return result(NETERR_SELECT_TIMEOUT);
   }

   sockaddr_in remote = {};
   int remote_len = sizeof(remote);
   SOCKET sock = ::accept(handle_, (sockaddr *)&remote, &remote_len);
   if (sock == INVALID_SOCKET) {
      return result();
   }

   tcp_listener::result result;
   result.socket_ = tcp_socket((uint32)sock);
   result.address_ = network::from_sockaddr(remote);
   return result;
}



udp_socket::udp_socket()
   : handle_(~0u)
{
}

bool udp_socket::is_valid() const {
   return handle_ != ~0u;
}

bool udp_socket::open(ip_address &addr) {
   // note: create a datagram socket
   SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock == INVALID_SOCKET) {
      return false;
   }

   // note: windows uses SOCKET as the type for a socket descriptor
   //       it is safe to cast to int or even uint32 per the 
   //       documentation on microsoft docs.
   handle_ = (uint32)sock;

   // note: disable blocking mode
   u_long non_blocking = 1;
   if (ioctlsocket(handle_, FIONBIO, &non_blocking) != 0) {
      return false;
   }

   // note: enable address reuse
   char reuse = TRUE;
   if (setsockopt(handle_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == 0) {
      return false;
   }

   // note: bind socket to defined address
   sockaddr_in addr_in = network::to_sockaddr(addr);
   if (bind(handle_, (const sockaddr *)& addr_in, sizeof(addr_in)) != 0) {
      return false;
   }

   return true;
}

void udp_socket::close() {
   closesocket(handle_);
   handle_ = ~0u;
}

bool udp_socket::send_to(const ip_address &addr, const int32 size, const uint8 *src) {
   return true;
}
bool udp_socket::recv_from(ip_address &addr, const int32 size, uint8 *dst) {
   return true;
}

bool udp_socket::address_of(ip_address &address) {
   return false;
}
