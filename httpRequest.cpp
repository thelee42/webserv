#include    "httpRequest.hpp"

httpRequest::httpRequest()
{
    /* constructor implementation */
}

httpRequest::~httpRequest()
{
    /* destructor implementation */
}
std::string httpRequest::parseExtension(const std::string &filename) {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) // no '.'
        return "";
    return filename.substr(pos + 1); // from '.' to end
}


void    httpRequest::parseRequestLine(const char *buffer) {
        std::string request(buffer, strlen(buffer)); // or bytesRead = return of recv
        size_t pos = request.find("\r\n");       // getline
        std::string requestLine = request.substr(0, pos);

        std::istringstream request_line(requestLine);
        request_line >> _method >> _path >> _version;

        _extension = parseExtension(_path);
        if (_method != "GET")
            _filename = "405.html";
        else if (_path == "/")
            _filename = "./home.html";
        else
            _filename = "." + _path;
}

std::string httpRequest::getMethod() const { return _method; }
std::string httpRequest::getPath() const { return _path; }
std::string httpRequest::getVersion() const { return _version; }
std::string httpRequest::getExtension() const { return _extension; }
std::string httpRequest::getFilename() const { return _filename; }
void httpRequest::setFilename(const std::string &filename) { _filename = filename; }