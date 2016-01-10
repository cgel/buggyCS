#include <iostream>
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>
#include <string>
#include "joystick.hh"

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

class Command {
public:
  Command() : joystick("/dev/input/js0") {}

  void sample() {
    int count = 0;
    while (count < 20) {
      ++count;
      usleep(100);
      JoystickEvent event;
      if (joystick.sample(&event)) {
        // if (event.isAxis() && (event.number == 0 || event.number == 13))
        if (event.number == 0 || event.number == 13) {
          int val = (int)(event.value * (256.0 / 32767.0));
          if (event.number == 13)
            input[0] = val;
          else
            input[1] = val;
        }
      }
    }
  }

  boost::array<short int, 2> input;

private:
  Joystick joystick;
};

int main(int argc, char *argv[]) {
  Comms comms;
  Command cmd;
  try {
    while (1) {
      cmd.sample();
      comms.send(cmd.input);
      usleep(10000);
      std::cout << cmd.input[0] << " - " << cmd.input[1] << std::endl;
    }
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
