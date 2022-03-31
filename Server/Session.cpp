#include <Session.hpp>


SessionOperator* SessionOperator::instance_ = nullptr;


Session::Session(boost::asio::io_service &io_service, std::string directory):
    sock_(io_service), directory_(std::move(directory)),
    io_service_(io_service)
{ }

boost::asio::ip::tcp::socket &Session::Socket() {
    return sock_;
}

void Session::Start() {
    auto pack = ReadPackage();
    io_service_.post(
        [this, pack]() {
            ProcessPackage(pack);
        });
}

void Session::ProcessPackage(const Protocol::ProtocolPackage &package) {
    using namespace Protocol;
    switch(package.step) {

        case ProtocolStep::RequestID:
            SendID(SessionOperator()->GetUniqueID());
            break;
        case ProtocolStep::AddListener:
            SessionOperator()->AddSession(this, package.id);
            break;
        case ProtocolStep::OpenGate:
            io_service_.post(
                [package]() {
                    SessionOperator()->OpenGate(package.id);
                });
            break;
        default:
            write_error("protocol violation");
            EndSession();
            break;
    }
}

void Session::EndSession() {
    sock_.close();
    delete this;
}


void Session::SendID(Protocol::ID id) {
    using namespace Protocol;
    Protocol::ProtocolPackage pack{ProtocolStep::SendID, id};
    SendPackage(pack);
}


SessionOperator *Session::SessionOperator() {
    return SessionOperator::GetInstance();
}

void Session::StartSending(size_t file_pos, size_t chunk_size) {
    using namespace Protocol;
    // expect query to be RequestFilePos
    auto pack = ReadPackage();
    if (pack.step != ProtocolStep::RequestFilePos) {
        write_error("protocol violation");
        EndSession();
    }
    SendFilePos(file_pos);
    file_.seekg((long long) file_pos);
    chunk_size_ = chunk_size;
    io_service_.post(
        [this]() {
            SendChunk();
        });

}

void Session::SendFilePos(size_t file_pos) {
    using namespace Protocol;
    Protocol::ProtocolPackage pack{ProtocolStep::SendFilePos, 0, file_pos};
    SendPackage(pack);
}

void Session::SendPackage(Protocol::ProtocolPackage &package) {
    // serialize object
    std::ostream os(&buffer_);
    boost::archive::binary_oarchive oa(os);
    oa << package;
    // send object
    boost::asio::write(sock_, buffer_,
                       boost::asio::transfer_at_least(sizeof(Protocol::ProtocolPackage)));

}

Protocol::ProtocolPackage Session::ReadPackage() {
    // read serialized object
    boost::asio::read(sock_, buffer_,
                      boost::asio::transfer_at_least(sizeof(Protocol::ProtocolPackage)));

    // deserialize object
    Protocol::ProtocolPackage package{};
    std::istream is(&buffer_);
    boost::archive::binary_iarchive ia(is);
    ia >> package;

    return package;

}

std::string Session::RequestFileName() {
    using namespace Protocol;
    Protocol::ProtocolPackage pack{ProtocolStep::RequestFileName};
    SendPackage(pack);
    auto answer = ReadPackage();
    if (answer.step != ProtocolStep::SendFileSize) {
        write_error("protocol violation");
        EndSession();
    }

    boost::asio::read(sock_, buffer_, boost::asio::transfer_at_least(answer.file_size));

    std::string res = boost::asio::buffer_cast<const char *>(buffer_.data());
    return res;
}

std::string Session::GetDirectory() {
    return directory_;
}

void Session::SetDirectory(std::string directory) {
    directory_ = std::move(directory);
}

void Session::SendChunk() {
    std::streamsize sub_chunk_size = 1024*1024; // 1 MB
    std::streamsize need_to_read = (std::streamsize)chunk_size_ - (std::streamsize)sent_size_;
    if (need_to_read == 0) {
        EndSession();
        return;
    }
    sub_chunk_size = std::min(sub_chunk_size, need_to_read);
    std::string chunk;
    chunk.resize(sub_chunk_size);
    file_.read(&chunk[0], sub_chunk_size);
    sent_size_ += sub_chunk_size;
    file_ >> chunk;
    boost::asio::write(sock_, boost::asio::buffer(chunk),
                       boost::asio::transfer_exactly(sub_chunk_size));

    io_service_.post(
        [this]() {
            SendChunk();
        });

}


template<typename Writable>
void write_error(const Writable &ex) {
    std::cout << ex << std::endl;
}


Protocol::ID SessionOperator::GetUniqueID() {
    return unique_id.fetch_add(1);
}

void SessionOperator::OpenGate(Protocol::ID id) {

    auto& sessions = session_map[id];
    size_t count = sessions.size();

    if (count == 0) {
        write_error("no listeners");
    } else {
        std::string filename = sessions[0]->RequestFileName();
        std::filesystem::path path(sessions[0]->GetDirectory());
        path += filename;
        sessions[0]->SetDirectory(path.string());
        size_t file_size = std::filesystem::file_size(path);
        size_t chunk_size = file_size / count;
        size_t last_part = file_size % count;

        for (size_t session_ind = 0; session_ind < count; ++session_ind) {
            sessions[session_ind]->StartSending(session_ind * chunk_size, chunk_size);
            if (session_ind + 1 == count) {
                sessions[session_ind]->StartSending(session_ind*chunk_size, chunk_size + last_part);
            } else {
                sessions[session_ind]->StartSending(session_ind*chunk_size, chunk_size);
            }
        }
    }
}

SessionOperator *SessionOperator::GetInstance() {
    if (instance_ == nullptr) {
        instance_ = new SessionOperator();
    }
    return instance_;
}

void SessionOperator::AddSession(Session *session, Protocol::ID id) {
    session_map[id].push_back(session);
}
