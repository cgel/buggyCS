#pragma once
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include "joystick.hh"

using namespace std;
using asio::ip::udp;

static using input_t = boost::array<short int, 2>;

// asynchronously updates the contents of the inut holder with the commands
// given by joysticks or driving wheels
class InputUI {
 public:
  InputUI(asio::io_service& io_s)
      : sample_period(5), sample_input_timer(io_s, sample_period) {
    sample_input_timer.async_wait(boost::bind(&Interface::sample, this));
    // default input
    open_controller();
    set_default_input();
  }

  ~InputUI() {
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

  input_t input_holder;

 private:
  asio::deadline_timer sample_input_timer;
  boost::posix_time::milliseconds sample_period;
  Joystick* controller;
};
