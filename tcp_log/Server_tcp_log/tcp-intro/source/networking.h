// networking.h

#include <string>
using string = std::string;

typedef unsigned long long uint64;
typedef   signed long long int64;
typedef unsigned int       uint32;
typedef   signed int       int32;
typedef unsigned short     uint16;
typedef   signed short     int16;
typedef unsigned char      uint8;
typedef   signed char      int8;

struct time {
   static time now();

   time();
   explicit time(int64 tick);

   time operator+(const time &rhs);
   time operator-(const time &rhs);

   float as_seconds() const;
   float as_milliseconds() const;

   int64 tick_;
};

enum network_error_code {
   NETERR_NO_ERROR,
   NETERR_INVALID_HANDLE,
   NETERR_NOT_ENOUGH_MEMORY,
   NETERR_INVALID_PARAMETER,
   NETERR_OPERATION_ABORTED,
   NETERR_INTERUPTED_CALL,
   NETERR_BAD_FILE_HANDLE,
   NETERR_SOCKET_ACCESS_DENIED,
   NETERR_BAD_ADDRESS,
   NETERR_INVALID_ARGUMENT,
   NETERR_TOO_MANY_OPEN_FILES,
   NETERR_WOULD_BLOCK,
   NETERR_IN_PROGRESS,
   NETERR_ALREADY_IN_PROGRESS,
   NETERR_HANDLE_NON_SOCKET,
   NETERR_DESTINATION_ADDRESS_REQUIRED,
   NETERR_MESSAGE_TOO_LONG,
   NETERR_WRONG_PROTOTYPE,
   NETERR_BAD_PROTOCOL_OPTION,
   NETERR_PROTOCOL_NOT_SUPPORTED,
   NETERR_SOCKET_TYPE_NOT_SUPPORTED,
   NETERR_OPERATION_NOT_SUPPORTED,
   NETERR_PROTOCOL_FAMILY_NOT_SUPPORT,
   NETERR_ADDRESS_FAMILY_NOT_SUPPORT,
   NETERR_ADDRESS_IN_USE,
   NETERR_ADDRESS_NOT_AVAILABLE,
   NETERR_NETWORK_DOWN,
   NETERR_NETWORK_UNREACHABLE,
   NETERR_NETWORK_DROPPED_CONNECTION,
   NETERR_CONNECTION_RESET_BY_SOFTWARE,
   NETERR_CONNECTION_RESET_BY_PEER,
   NETERR_NO_BUFFER_SPACE_AVAIABLE,
   NETERR_ALREADY_CONNECTED,
   NETERR_NOT_CONNECTED,
   NETERR_SEND_SHUTDOWN,
   NETERR_TOO_MANY_REFS,
   NETERR_CONNECTION_TIMED_OUT,
   NETERR_CONNECTION_REFUSED,
   NETERR_TRANSLATE_NAME,
   NETERR_NAME_TOO_LONG,
   NETERR_HOST_DOWN,
   NETERR_HOST_UNREACHABLE,
   NETERR_PROCESS_LIMIT,
   NETERR_NETWORK_SYSTEM_NOT_READY,
   NETERR_VERSION_NOT_SUPPORTED,
   NETERR_NETWORK_NOT_INITIALISED,
   NETERR_DISCONNECT_IN_PROGRESS,
   NETERR_INVALIDPROCTABLE,
   NETERR_INVALIDPROVIDER,
   NETERR_PROVIDERFAILEDINIT,
   NETERR_SYSCALLFAILURE,
   NETERR_HOST_NOT_FOUND,
   NETERR_TRY_AGAIN,
   NETERR_NO_RECOVERY,
   NETERR_NO_DATA,
   NETERR_REMOTE_DISCONNECT,
   NETERR_SELECT_TIMEOUT,
   NETERR_UNKNOWN,
};

namespace network {
   bool init();
   void shut();

   namespace error {
      network_error_code get_error();
      const char *as_string(network_error_code error_code);
      bool is_non_critical(network_error_code error_code);
   } // !error

   namespace util {
      bool message_box(const char *format, ...);
   } // !util
} // !network

struct ip_address {
   ip_address();
   explicit ip_address(uint32 host, uint16 port);
   explicit ip_address(uint8 a, uint8 b, uint8 c, uint8 d, uint16 port = 0);

   bool operator==(const ip_address &rhs) const;

   void set_host(uint8 a, uint8 b, uint8 c, uint8 d);
   void set_port(uint16 port);
   bool as_string(string &str) const;

   uint32 host_;
   uint16 port_;
};

struct tcp_socket {
   struct result {
      result(int32 length);
      result(network_error_code code);

      bool is_success() const;
      bool is_failure() const;

      network_error_code code_{};
      int32 length_{};
   };

   tcp_socket();
   explicit tcp_socket(uint32 handle);

   bool is_valid() const;
   void close();

   result connect(const ip_address &address);
   result send(const int32 size, const uint8 *src);
   result receive(const int32 size, uint8 *dst);

   uint32 handle_;
};

struct tcp_listener {
   struct result {
      result();
      result(network_error_code code);

      bool is_success() const;
      bool is_failure() const;

      network_error_code code_{};
      ip_address address_;
      tcp_socket socket_;
   };

   tcp_listener();
   ~tcp_listener();

   bool open(const ip_address &address);
   void close();

   result accept();
   uint32 handle_;
};

struct udp_socket {
   udp_socket();

   bool is_valid() const;
   bool open(ip_address &address);
   void close();

   bool send_to(const ip_address &addr, const int32 size, const uint8 *src);
   bool recv_from(ip_address &addr, const int32 size, uint8 *dst);
   bool address_of(ip_address &address);

   uint32 handle_;
};
