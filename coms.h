#pragma once
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#define DEF_BUGGY_IP "127.0.0.1"
//#define BUGGY_IP "192.168.1.165"
#define PORT 6000

using namespace std;
using asio::ip::udp;

using input_t = boost::array<short int, 2>;

// connects and communicates with the buggy
class Comms {
 public:
  Comms(asio::io_service& io_s, string ip, const input_t& input_holder_)
      : socket(io_s),
        send_period(boost::posix_time::milliseconds(30)),
        input_holder(input_holder_),
        send_input_timer(io_s, send_period) {
    // setup connection
    udp::resolver resolver(io);
    buggy_endpoint = *resolver.resolve(udp::resolver::query(
                          udp::v4(), ip, std::to_string(PORT)));
    socket.open(udp::v4());

    // setup async callback
    send_input_timer.async_wait(boost::bind(&Comms::send, this));
  }

  void send() {
    socket.send_to(asio::buffer(input_holder), buggy_endpoint);
    // setup async callback
    send_input_timer.expires_at(send_input_timer.expires_at() + send_period);
    send_input_timer.async_wait(boost::bind(&Comms::send, this));
    cout << "sending: " << input_holder[0] << " - " << input_holder[1] << endl;
  }

 private:
  asio::deadline_timer send_input_timer;
  boost::posix_time::milliseconds send_period;
  asio::io_service io;
  udp::socket socket;
  udp::endpoint buggy_endpoint;
  const input_t& input_holder;
};


