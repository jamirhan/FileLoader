#pragma once
#include <string>
#include <boost/asio.hpp>
#include <ProtocolPackage.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem.hpp>

class FileClient {
public:
    FileClient(boost::asio::io_service& io_service, std::string host, size_t port,
               std::string server_file_path, std::string local_file_path);

    void FetchFileChunk();
private:
    void RequestId();
    Protocol::ProtocolPackage ReceivePackage();
    void RequestFilePosition();
    void SendPackage(const Protocol::ProtocolPackage& package);
    void SendFileName();
private:
    Protocol::ID id_{0};
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf request_;
    boost::asio::streambuf response_;
    std::string server_file_path_;
    std::string local_file_path_;
    std::string host_;
    size_t file_pos_{0};
    std::ofstream file_;
};


class Client
{
public:
    Client(size_t threads_num, boost::asio::io_service& io_service, std::string host, size_t port,
           std::string server_file_path, std::string local_file_path);

private:

private:
    boost::asio::io_service& io_service_;

};
