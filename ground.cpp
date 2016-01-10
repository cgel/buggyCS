#include <iostream>
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>
#include <string>

#define BUGGY_IP "127.0.0.1"
#define PORT 6000

using asio::ip::udp;

class Comms {
public:
  Comms() : socket(io) {
    // initialize connection
    udp::resolver resolver(io);
    buggy_endpoint = *resolver.resolve(udp::resolver::query(
                          udp::v4(), BUGGY_IP, std::to_string(PORT)));
    socket.open(udp::v4());
  }
  void send(boost::array<short int, 2> &msg) {
    socket.send_to(asio::buffer(msg), buggy_endpoint);
  }

private:
  asio::io_service io;
  udp::socket socket;
  udp::endpoint buggy_endpoint;
};

int main(int argc, char *argv[]) {
  Comms comms;
  try {
    while (1) {
      boost::array<short int, 2> msg = { 0, 1 };
      comms.send(msg);
      sleep(1);
    }
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
