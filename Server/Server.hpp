#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <fstream>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <Session.hpp>


class Server
{
    void start_accept();
    void handle_accept(const boost::system::error_code& error, Session* session);
public:

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;
    Server(boost::asio::io_service& io_service, std::string  directory, size_t port);

private:
    std::string directory_;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
};
