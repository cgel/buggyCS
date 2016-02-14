#include <iostream>
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>
#include <string>
#include "joystick.hh"

#define DEF_BUGGY_IP "127.0.0.1"
//#define BUGGY_IP "192.168.1.165"
#define PORT 6000

using namespace std;
using asio::ip::udp;

using Input = boost::array<short int, 2>;

// connects and communicates with the buggy
class Comms {
 public:
  Comms(asio::io_service& io_s, string ip, const Input& input_holder_)
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
  const Input& input_holder;
};

// asynchronously updates the contents of the inut holder with the commands
// given by joysticks or driving wheels
class Interface {
 public:
  Interface(asio::io_service& io_s)
      : sample_period(5), sample_input_timer(io_s, sample_period) {
    sample_input_timer.async_wait(boost::bind(&Interface::sample, this));
    // default input
    open_controller();
    set_default_input();
  }

  ~Interface() {
    if (controller != nullptr) {
      delete controller;
    }
  }
  void sample() {
    if (controller != nullptr) {
      JoystickEvent event;

      if (controller->sample(&event)) {
        // an event
        //        cout << "event - " << (int)event.number << endl;
        if (event.number == 0 || event.number == 13) {
          // an event of interest
          int val = (int)(event.value * (256.0 / 32767.0));
          if (event.number == 13) {
            input_holder[0] = val;
          } else {
            input_holder[1] = val;
          }
        }
      }
    } else {
      set_default_input();
    }
    // async callback to sample in the next sample_period
    sample_input_timer.expires_at(sample_input_timer.expires_at() +
                                  sample_period);
    sample_input_timer.async_wait(boost::bind(&Interface::sample, this));
  }

  void set_default_input() {
    input_holder[0] = 0;
    input_holder[1] = 0;
  }

  void open_controller() {
    // Joystic class sucks so i have to do this hack
    // if Joystic is assigned it does not sample
    auto joy = new Joystick;
    if (joy->isFound()) {
      cout << "Controller Connected" << endl;
      controller = joy;
    } else {
      cout << "failed openning" << endl;
      delete joy;
    }
  }

  Input input_holder;

 private:
  asio::deadline_timer sample_input_timer;
  boost::posix_time::milliseconds sample_period;
  Joystick* controller;
};

int main(int argc, char* argv[]) {
  string buggy_ip;
  cout << "argc " << argc << endl;
  if (argc > 1) {
    buggy_ip = argv[1];
  } else {
    buggy_ip = DEF_BUGGY_IP;
  }
  asio::io_service io_s;
  cout << "the buggy ip is: " << buggy_ip << endl;
  try {
    Interface interface(io_s);
    Comms comms(io_s, buggy_ip, interface.input_holder);
    // asio::deadline_timer no_input_failsafe_timer(io_s,
    //                                             boost::posix_time::seconds(1));
    io_s.run();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
