#include <Server.hpp>


Server::Server(boost::asio::io_service &io_service, std::string directory, size_t port):
    io_service_(io_service),
    acceptor_(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    directory_(std::move(directory))
{
    start_accept();
}

void Server::handle_accept(const boost::system::error_code &error, Session* session) {
    if (!error) {
        session->Start();
    } else {
        std::cerr << "Error: " << error.message() << std::endl;
        delete session;
    }
    start_accept();
}

void Server::start_accept() {
    auto* session = new Session(io_service_, directory_);
    acceptor_.async_accept(session->Socket(), [this, &session](const boost::system::error_code &error) {
        handle_accept(error, session);

    });
}
