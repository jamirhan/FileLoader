#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <fstream>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <ProtocolPackage.hpp>
#include <unordered_map>
#include <vector>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <atomic>
#include <mutex>

template <typename Writable>
void write_error(const Writable& ex);



class Session;

class SessionOperator {
public:
    void AddSession(Session* session, Protocol::ID id);
    void OpenGate(Protocol::ID id);
    Protocol::ID GetUniqueID();
    static SessionOperator* GetInstance();
private:
    static SessionOperator* instance_;
    std::unordered_map<Protocol::ID, std::vector<Session*>> session_map;
    std::atomic<Protocol::ID> unique_id{0};
};

class Session {
public:
    Session(boost::asio::io_service& io_service, std::string file_name);
    // get the socket
    boost::asio::ip::tcp::socket& Socket();
    // start the session
    void Start();
    std::string RequestFileName();
    void StartSending(size_t file_pos, size_t chunk_size);
    std::string GetDirectory();
    void SetDirectory(std::string directory);
private:
    void ProcessPackage(const Protocol::ProtocolPackage& package);
    void SendFilePos(size_t file_pos);
    void SendPackage(Protocol::ProtocolPackage& package);
    void EndSession();
    void SendChunk();
    Protocol::ProtocolPackage ReadPackage();
    void SendID(Protocol::ID id);
    static SessionOperator* SessionOperator();
private:
    boost::asio::io_service& io_service_;
    std::string directory_;
    size_t sent_size_{0};
    size_t chunk_size_{0};
    boost::asio::streambuf buffer_;
    boost::asio::ip::tcp::socket sock_;
    std::fstream file_;
};
