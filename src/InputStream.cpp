#include <vpktool/InputStream.h>

#include <tuple>

using namespace vpktool;

InputStream::InputStream(const std::string& filepath, bool binary) {
    this->stream = reinterpret_cast<byte*>(fopen(filepath.c_str(), binary ? "rb" : "r"));
    this->streamLen = -1;
    this->streamPos = 0;
}

InputStream::InputStream(byte* buffer, std::uint64_t bufferLen) {
    this->stream = buffer;
    this->streamLen = bufferLen;
    this->streamPos = 0;
}

InputStream::~InputStream() {
    if (this->streamLen > 0)
        fclose((FILE*) this->stream);
}

InputStream::InputStream(InputStream&& other) noexcept {
    this->stream = other.stream;
    this->streamLen = other.streamLen;
    this->streamPos = other.streamPos;
}

InputStream& InputStream::operator=(InputStream&& other) noexcept {
    this->stream = other.stream;
    this->streamLen = other.streamLen;
    this->streamPos = other.streamPos;
    return *this;
}

InputStream::operator bool() const {
    return static_cast<bool>(this->stream);
}

bool InputStream::operator!() const {
    return !this->operator bool();
}

void InputStream::seek(std::uint64_t pos) {
    this->seek(pos, SEEK_SET);
}

void InputStream::seek(std::uint64_t offset, int offsetFrom) {
    if (this->streamLen > 0)
        fseek((FILE*) this->stream, static_cast<long>(offset), offsetFrom);
    else {
        if (offsetFrom == SEEK_SET)
            this->streamPos = offset;
        else if (offsetFrom == SEEK_CUR)
            this->streamPos += offsetFrom;
        else if (offsetFrom == SEEK_END)
            this->streamPos = this->streamLen + offsetFrom;
    }
}

long InputStream::tell() const {
    if (this->streamLen > 0)
        return ftell((FILE*) this->stream);
    return static_cast<long>(this->streamPos);
}

std::vector<byte> InputStream::readBytes(unsigned int length) {
    std::vector<byte> out;
    out.reserve(length);
    if (this->streamLen > 0)
        std::ignore = fread(out.data(), sizeof(byte), length, (FILE*) this->stream);
    else {
        for (int i = 0; i < length; i++, this->streamPos++) {
            out.push_back(this->stream[this->streamPos]);
        }
    }
    return out;
}

std::string InputStream::readString() {
    std::string out;
    char temp;
    temp = this->read<char>();
    while (temp != '\0') {
        out += temp;
        temp = this->read<char>();
    }
    return out;
}

byte InputStream::peek(long offset) {
    byte out = '\0';
    if (this->streamLen > 0) {
        this->seek(offset, SEEK_CUR);
        std::ignore = fread(&out, sizeof(byte), 1, (FILE *) this->stream);
        this->seek(-offset - 1, SEEK_CUR);
    } else {
        out = this->stream[this->streamPos + offset];
    }
    return out;
}
