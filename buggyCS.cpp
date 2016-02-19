#include <iostream>
#include <string>
#include "coms.h"
#include "input_ui.h"

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
    InputUI ui(io_s);
    Comms comms(io_s, buggy_ip, ui.input_holder);
    io_s.run();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
