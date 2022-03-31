#pragma once
#include <cstddef>

namespace Protocol {
    using ID = size_t;
    static const size_t MAX_FILENAME_LENGTH = 256;

    enum ProtocolStep {
        RequestID,
        SendID,
        AddListener,
        OpenGate,
        RequestFileName,
        SendFileSize,
        RequestFilePos,
        SendFilePos
    };

    struct ProtocolPackage {
        ProtocolStep step;
        ID id;
        size_t file_pos;
        size_t file_size;
        template<typename archive> void serialize(archive& ar, const unsigned version) {
            ar & step;
            ar & id;
        }
    };

} // namespace Protocol
