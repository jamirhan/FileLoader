#include <Client.hpp>


Client::Client(size_t threads_num, boost::asio::io_service &io_service, std::string host,
               size_t port, std::string server_file_path, std::string local_file_path): io_service_(io_service) {
    for (size_t thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
        io_service_.post([this, host = std::move(host), port,
                          server_file_path = std::move(server_file_path),
                          local_file_path = std::move(local_file_path)]() mutable{
            FileClient worker(io_service_, std::move(host), port, std::move(server_file_path),
                              std::move(local_file_path));
            worker.FetchFileChunk();
        });
    }
}

FileClient::FileClient(boost::asio::io_service &io_service, std::string host, size_t port, std::string server_file_path,
                       std::string local_file_path): socket_(io_service),
                                                     host_(std::move(host)),
                                                     server_file_path_(std::move(server_file_path)),
                                                     local_file_path_(std::move(local_file_path)) {
    using namespace boost::asio::ip;
    tcp::endpoint endpoint(address::from_string(host_), port);
    socket_.connect(endpoint);
}

void FileClient::FetchFileChunk() {
    file_.open(local_file_path_, std::ios::binary | std::ios::out);
    RequestId();
    RequestFilePosition();
    SendFileName();

    // read from socket and write to file while there is data
    file_.seekp((long long) file_pos_);
    while (true) {
        boost::system::error_code error;
        size_t bytes_transferred = boost::asio::read(socket_, response_, boost::asio::transfer_at_least(1), error);
        if (error == boost::asio::error::eof) {
            break;
        } else if (error) {
            throw boost::system::system_error(error);
        }
        std::string response_str(boost::asio::buffer_cast<const char *>(response_.data()), bytes_transferred);
        file_ << response_str;
        file_.flush();
    }
}

void FileClient::RequestId() {
    using namespace Protocol;
    ProtocolPackage request_id_package{ProtocolStep::RequestID, 0, 0};
    SendPackage(request_id_package);
    auto response = ReceivePackage();
    id_ = response.id;
}

Protocol::ProtocolPackage FileClient::ReceivePackage() {
    boost::asio::read(socket_, response_,
                      boost::asio::transfer_at_least(sizeof(Protocol::ProtocolPackage)));

    // deserialize object
    Protocol::ProtocolPackage package{};
    std::istream is(&response_);
    boost::archive::binary_iarchive ia(is);
    ia >> package;

    return package;
}


void FileClient::SendPackage(const Protocol::ProtocolPackage &package) {
    // serialize object
    std::ostream os(&request_);
    boost::archive::binary_oarchive oa(os);
    oa << package;
    // send object
    boost::asio::write(socket_, request_,
                       boost::asio::transfer_at_least(sizeof(Protocol::ProtocolPackage)));

}

void FileClient::RequestFilePosition() {
    using namespace Protocol;
    ProtocolPackage request_file_position_package{ProtocolStep::RequestFilePos, id_, 0};
    SendPackage(request_file_position_package);
    auto response = ReceivePackage();
    file_pos_ = response.file_pos;
}

void FileClient::SendFileName() {
    using namespace Protocol;
    auto pack = ReceivePackage();
    if (pack.step != ProtocolStep::RequestFileName) {
        throw std::runtime_error("Expected RequestFileName");
    }
    ProtocolPackage file_name_package{ProtocolStep::RequestFileName, id_,
                                      0, server_file_path_.size()};
    SendPackage(file_name_package);
    boost::asio::write(socket_, boost::asio::buffer(server_file_path_),
                       boost::asio::transfer_exactly(server_file_path_.size()));
}
