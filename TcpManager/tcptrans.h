#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "def.h"
#include <thread>
#include <memory>

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

class client
{
public:
  client(boost::asio::io_service &io_service)
      : stopped_(false),
        socket_(io_service),
        deadline_(io_service),
        heartbeat_timer_(io_service)
  {
    headersize = sizeof(PACKET_HEADER);
  }

  void start(tcp::resolver::iterator endpoint_iter)
  {

    start_connect(endpoint_iter);

    deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

  void stop()
  {
    stopped_ = true;
    socket_.close();
    deadline_.cancel();
    heartbeat_timer_.cancel();
  }

private:
  void start_connect(tcp::resolver::iterator endpoint_iter)
  {
    if (endpoint_iter != tcp::resolver::iterator())
    {
      std::cout << "Trying " << endpoint_iter->endpoint() << "\n";
      deadline_.expires_from_now(boost::posix_time::seconds(60));
      socket_.async_connect(endpoint_iter->endpoint(),
                            boost::bind(&client::handle_connect,
                                        this, _1, endpoint_iter));
    }
    else
    {
      stop();
    }
  }

  void handle_connect(const boost::system::error_code &ec,
                      tcp::resolver::iterator endpoint_iter)
  {
    if (stopped_)
      return;

    if (!socket_.is_open())
    {
      std::cout << "Connect timed out\n";
      sleep(1000*3);
      start_connect(endpoint_iter);
      //start_connect(++endpoint_iter);
    }
    else if (ec)
    {
      std::cout << "Connect error: " << ec.message() << "\n";
      socket_.close();
      sleep(3);
      start_connect(endpoint_iter);
      //start_connect(++endpoint_iter);
    }
    else
    {
      std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
      start_read_head();
      start_write();
    }
  }

  void start_read_head()
  {
    deadline_.expires_from_now(boost::posix_time::seconds(30));
    input_buffer_.clear();
    input_buffer_.resize(headersize);
    socket_.async_read_some(boost::asio::buffer(input_buffer_.data(), headersize), boost::bind(&client::handle_read_head, this, _1, _2));
  }

  void handle_read_head(const boost::system::error_code &ec,std::size_t bytes)
  {
    //std::cout << "read:" << reinterpret_cast<PACKET_HEADER*>(input_buffer_.data())->Command << std::endl;

    // if (0 == reinterpret_cast<PACKET_HEADER*>(input_buffer_.data())->DataLength) {
    //       uint16_t _cmd = reinterpret_cast<PACKET_HEADER*>(input_buffer_.data())->Command;
    //       std::cout << "aa" << _cmd << std::endl;
    // }
    
    if (stopped_)
      return;

    if (!ec)
    {
      uint16_t _cmd = reinterpret_cast<PACKET_HEADER *>(input_buffer_.data())->Command;
      if (_cmd == 0x1201)
      {
        std::cout << "收到心跳回执" << std::endl;
      }
      uint16_t _size = reinterpret_cast<PACKET_HEADER *>(input_buffer_.data())->DataLength;
      if (_size > 0)
      {
        start_read_body(_cmd,_size);
      }
    }
    else
    {
      std::cout << "head Error on receive: " << ec.message() << "\n";
      stop();
    }
  }

  void start_read_body(uint16_t cmd,uint16_t size)
  {
    if (stopped_)
      return;
    std::vector<char> data = input_buffer_;

    data.resize(size + data.size());
    boost::system::error_code errcode;
    socket_.read_some(boost::asio::buffer(data.data() + sizeof (PACKET_HEADER), data.size() - sizeof (PACKET_HEADER)), errcode);
    if(!errcode)
    {
      std::cout << "read body size:" << data.size() - headersize << std::endl;
    }
    else
    {
      std::cout << "body Error on receive: " << errcode.message() << "\n";
      stop();
    }
    
    start_read_head();
  }

  void start_write()
  {
    if (stopped_)
      return;

    std::shared_ptr<PACKET_HEADER> data = std::make_shared<PACKET_HEADER>();
    data->Command = 0x1201;
    size_t size = sizeof(PACKET_HEADER);
    boost::asio::async_write(socket_, boost::asio::buffer(data.get(), size),
                             boost::bind(&client::handle_write, this, _1));
  }

  void handle_write(const boost::system::error_code &ec)
  {
    if (stopped_)
      return;

    if (!ec)
    {
      std::cout << "发送心跳成功" << std::endl;
      heartbeat_timer_.expires_from_now(boost::posix_time::seconds(4));
      heartbeat_timer_.async_wait(boost::bind(&client::start_write, this));
    }
    else
    {
      std::cout << "Error on heartbeat: " << ec.message() << "\n";
      stop();
    }
  }

  void send_msg(const std::shared_ptr<char> data, size_t size)
  {
    if (stopped_)
      return;
    boost::asio::async_write(socket_, boost::asio::buffer(data.get(), size), [](boost::system::error_code errCode, std::size_t) {
      if (!errCode)
      {
        std::cout << "发送数据" << std::endl;
      }
      else
      {
        //notiResult(false, pProtocol->m_nCommand);
      }
    });
  }

  void check_deadline()
  {
    if (stopped_)
      return;

    if (deadline_.expires_at() <= deadline_timer::traits_type::now())
    {

      socket_.close();

      deadline_.expires_at(boost::posix_time::pos_infin);
    }

    deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

private:
  bool stopped_;
  tcp::socket socket_;
  std::vector<char> input_buffer_;
  deadline_timer deadline_;
  deadline_timer heartbeat_timer_;

  int headersize;
};