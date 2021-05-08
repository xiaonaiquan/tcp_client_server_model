#include "tcptrans.h"

int main(int argc, char* argv[])
{
  try
  {
    // if (argc != 3)
    // {
    //   std::cerr << "Usage: client <host> <port>\n";
    //   return 1;
    // }
    //io_service放在另外一个线程中执行

    boost::asio::io_service io_service;
    tcp::resolver r(io_service);
    client c(io_service);

    c.start(r.resolve(tcp::resolver::query("127.0.0.1", "4567")));

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}