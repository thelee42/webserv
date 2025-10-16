#include    "httpRequest.hpp"
#include <sys/socket.h>

httpRequest::httpRequest()
{
    /* constructor implementation */
}

httpRequest::~httpRequest()
{
    /* destructor implementation */
}


void    httpRequest::parseRequest(const std::string &buffer) {

    // request line        
    size_t pos = buffer.find("\r\n");       // getline
    std::string requestLine;
    if (pos != std::string::npos)
        requestLine = buffer.substr(0, pos);
    else
        requestLine = buffer; // fallback: 전체 문자열을 요청라인으로
    std::istringstream request_line(requestLine);
    request_line >> _method >> _path >> _version;
    size_t queryPos = _path.find('?');
    if(queryPos != std::string::npos) {
        _queryString = _path.substr(queryPos + 1);
        _path = _path.substr(0, queryPos);
    }
    // headers
    size_t headerPos = pos + 2; // skip \r\n
    size_t bodyPos = buffer.find("\r\n\r\n");
    if (bodyPos == std::string::npos) {
        // headers not complete
        bodyPos = buffer.size();
    }
    // Extract header block
    std::string headerBlock = buffer.substr(headerPos, bodyPos - headerPos);
    std::istringstream headers(headerBlock);
    std::string headerLine;
    while(std::getline(headers, headerLine)) {
        if(!headerLine.empty() && headerLine.back() == '\r')
            headerLine.pop_back(); // remove trailing \r
        // Process headerLine
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 1);
            value.erase(0, value.find_first_not_of(' '));
            value.erase(value.find_last_not_of(' ') + 1); // ?
            _headers[key] = value;
        }
    }
}




// void    httpRequest::parseRequestLine(const std::string &buffer) {

//     // request line        
//     size_t pos = buffer.find("\r\n");       // getline
//     std::string requestLine;
//     if (pos != std::string::npos)
//         requestLine = buffer.substr(0, pos);
//     else
//         requestLine = buffer; // fallback: 전체 문자열을 요청라인으로
//     std::istringstream request_line(requestLine);
//     request_line >> _method >> _path >> _version;
//     size_t queryPos = _path.find('?');
//     if(queryPos != std::string::npos) {
//         _queryString = _path.substr(queryPos + 1);
//         _path = _path.substr(0, queryPos);
//     }
// }

// void    httpRequest::parseHeaders(const std::string &buffer) {
//     // headers
//     size_t pos = buffer.find("\r\n");
//     size_t headerPos = pos + 2; // skip \r\n
//     size_t bodyPos = buffer.find("\r\n\r\n");
//     if (bodyPos == std::string::npos) {
//         // headers not complete
//         bodyPos = buffer.size();
//     }
//     // Extract header block
//     std::string headerBlock = buffer.substr(headerPos, bodyPos - headerPos);
//     std::istringstream headers(headerBlock);
//     std::string headerLine;
//     while(std::getline(headers, headerLine)) {
//         if(!headerLine.empty() && headerLine.back() == '\r')
//             headerLine.pop_back(); // remove trailing \r
//         // Process headerLine
//         size_t colonPos = headerLine.find(':');
//         if (colonPos != std::string::npos) {
//             std::string key = headerLine.substr(0, colonPos);
//             std::string value = headerLine.substr(colonPos + 1);
//             value.erase(0, value.find_first_not_of(' '));
//             value.erase(value.find_last_not_of(' ') + 1); // ?
//             _headers[key] = value;
//         }
//     }
// }



std::string parseChunkedBody(const std::string& rawBody) {
    std::string result;
    size_t pos = 0;

    while (true) {
        // 1️⃣ 청크 크기 줄 찾기
        size_t endOfSize = rawBody.find("\r\n", pos);
        if (endOfSize == std::string::npos)
            throw std::runtime_error("Invalid chunked encoding: missing size line");

        // 2️⃣ 16진수 크기 파싱
        std::string sizeStr = rawBody.substr(pos, endOfSize - pos);
        size_t chunkSize = std::strtoul(sizeStr.c_str(), NULL, 16);

        pos = endOfSize + 2; // "\r\n" 스킵

        if (chunkSize == 0) {
            // 3️⃣ 마지막 청크 (0\r\n\r\n)
            break;
        }

        // 4️⃣ 데이터 읽기
        if (pos + chunkSize > rawBody.size())
            throw std::runtime_error("Invalid chunked encoding: chunk too short");

        result.append(rawBody, pos, chunkSize);

        pos += chunkSize + 2; // 데이터 + "\r\n" 스킵
    }

    return result;
}



void httpRequest::parseMultipartBody(const LocationConfig* loc) {
    if(_body.empty())
        return;
    isChunked = false;
    // 1. boundary 추출
    std::string contentType = getHeaderValue("Content-Type");
    if (contentType.find("multipart/form-data") == std::string::npos)
        return;
    if (/* hasHeader("Transfer-Encoding") &&  */getHeaderValue("Transfer-Encoding") == "chunked") {
        isChunked = true;
        _body = parseChunkedBody(_body);
    }
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) return; // boundary 없으면 종료
    _boundary = "--" + contentType.substr(boundaryPos + 9);

    // 2. 첫 part 찾기
    size_t partStart = _body.find(_boundary);
    if (partStart == std::string::npos) return;

    // 3. filename 추출
    size_t filenamePos = _body.find("filename=\"", partStart);
    if (filenamePos != std::string::npos) {
        filenamePos += 10; // skip "filename=\""
        size_t filenameEndPos = _body.find("\"", filenamePos);
        if (filenameEndPos != std::string::npos)
            _uploadFilename = _body.substr(filenamePos, filenameEndPos - filenamePos);
            if (!loc->isExtensionAllowed(_uploadFilename)) {
                std::cerr << "File extension not allowed: " << _uploadFilename << std::endl;
                _uploadFilename.clear(); // 허용되지 않은 확장자면 파일명 초기화
                return ;
            }
    }
    // 4. 파일 content 시작 위치
    size_t contentPos = _body.find("\r\n\r\n", filenamePos);
    if (contentPos != std::string::npos) {
        contentPos += 4; // skip \r\n\r\n

        // 5. 파일 content 끝 위치 (다음 boundary 전까지)
        size_t contentEnd = _body.find(_boundary, contentPos);
        if (contentEnd != std::string::npos) {
            // 마지막 CRLF 제거
            if (_body[contentEnd-2] == '\r' && _body[contentEnd-1] == '\n')
                contentEnd -= 2;

            _uploadContent = _body.substr(contentPos, contentEnd - contentPos);
        }
    }
}

std::string httpRequest::getMethod() const { return _method; }
std::string httpRequest::getPath() const { return _path; }
std::string httpRequest::getVersion() const { return _version; }
std::string httpRequest::getQueryString() const { return _queryString; }

// std::string httpRequest::getExtension() const { 
//     size_t pos = _path.find_last_of('.');
//     if (pos == std::string::npos) // no '.'
//         return "";
//     return _path.substr(pos + 1); // from '.' to end
// }



std::string httpRequest::getHeaderValue(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end())
        return it->second;
    return "";
}
//it->first : key
//it->second : value

std::string httpRequest::getBody() const { return _body; }

std::string httpRequest::getUploadFilename() const { return _uploadFilename; }
std::string httpRequest::getUploadContent() const { return _uploadContent; }


