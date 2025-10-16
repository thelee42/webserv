#include    "httpResponse.hpp"

httpResponse::httpResponse() : _httpVersion("HTTP/1.1"), _statusCode(200), _statusMessage("OK"), _body(""), _filename("") {
    // Default headers
    _headers["Content-Type"] = "text/html";
    //_headers["Connection"] = "close";
}

httpResponse::~httpResponse() {
}

void httpResponse::setStatus(int code, const std::string &message) {
    _statusCode = code;
    _statusMessage = message;
}

void httpResponse::setFilename(const std::string& path, const std::string& root)
{
    std::cout << "SETTING FILE root : " << root << ", path: " << path << "\n";
    std::string filepath = root;
    if (filepath.back() != '/')
        filepath += "/";
    if (path == "/")
        filepath += "index.html";
    // std::string cleanPath = path;
    // if (!cleanPath.empty() && cleanPath[0] == '/') {
    //     cleanPath = cleanPath.substr(1);
    // }
    // if (cleanPath.empty() || cleanPath == "/")
    //     filepath += "index.html";
    else
        filepath += path;

    // // 디렉토리 요청인 경우 index 파일로 보정
    // struct stat statbuf;
    // if (stat(filepath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
    //     if (filepath.back() != '/')
    //         filepath += "/";
    //     filepath += index;
    // }

    this->_filename = filepath;
}



void httpResponse::setHeader(const std::string &key, const std::string &value) {
    _headers[key] = value;
}

void httpResponse::setBody(const std::string &body) {
    _body = body;
}

int httpResponse::getStatusCode() const {
    return _statusCode;
}

std::string httpResponse::getHeader(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

std::string httpResponse::getBody() const {
    return _body;
}

std::string httpResponse::intToString(int value) const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string httpResponse::getExtension() const { 
    // 실제 파일명(_filename)에서 확장자 추출 (경로가 아닌)
    size_t pos = _filename.find_last_of('.');
    if (pos == std::string::npos || pos == 0)
        return "";
    
    std::string ext = _filename.substr(pos);
    return ext;
}

std::string httpResponse::getMimeType(const std::string &extension) const {
    static std::map<std::string, std::string> mimeTypes = initMimeTypes();
    if (mimeTypes.find(extension) != mimeTypes.end())
        return mimeTypes[extension];
    return "text/html"; // default
}

void httpResponse::generateFileBody() {
    std::ifstream file(_filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        setStatus(404, "Not Found");
        _filename = "./404.html";
        file.open(_filename.c_str(), std::ios::binary);
    }
    if (file.is_open()) {
        // 파일 크기 측정
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // 파일 내용 읽기
        std::vector<char> fileBuffer(size);
        file.read(fileBuffer.data(), size);
        file.close();
        
        // HTTP 응답 헤더
        setHeader("Content-Type", getMimeType(getExtension()));
        // CGI
        if (!getExtension().compare(".py"))
		{
			int	fds[2];
			int	return_value;
			pipe(fds);
			pid_t	pid = fork();
			if (pid == 0)
			{
				std::string	exec = "/usr/bin/python3";
				char	*tab[] = {&exec[0], &_filename[0], NULL};
				dup2(fds[1], 1);
				close(fds[1]);
				close(fds[0]);
				std::cerr << &exec[0] << ", " << &_filename[0] << std::endl;
				char	curr[50];
				getcwd(curr, 50);
				std::cerr << "curr: " << curr << std::endl;
				execve(&exec[0], tab, NULL);
				perror(&exec[0]);
				std::cerr << "test2\n";
				exit(1);
			}
			close(fds[1]);
			waitpid(pid, &return_value, 0);
			std::cout << return_value << std::endl;
			std::cout << "finished waiting\n";
			std::string	content;
			char	buffer[2];
			buffer[1] = '\0';
			while (read(fds[0], buffer, 1) > 0)
			{
				std::cerr << buffer;
				content += buffer;
			}
			std::cout << "finished reading\n";
			setHeader("Content-Length", intToString(content.length()));
			//setHeader("Connection", "keep-alive");
            setHeader("Connection", "close");
			_body = content;
			return ;
		}

        setHeader("Content-Length", intToString(size));
        //setHeader("Connection", "keep-alive");
        setHeader("Connection", "close");

        // 파일 내용 추가
        _body.assign(&fileBuffer[0], fileBuffer.size());
    }
}

std::string httpResponse::buildCompleteResponse() const {
    std::string response = _httpVersion + " " + intToString(_statusCode) + " " + _statusMessage + "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }

    response += "\r\n"; // 헤더 끝
    response += _body;   // 바디
    return response;
}

// void httpResponse::generateFileBody() {
//     std::ifstream file(_filename.c_str(), std::ios::binary);
//     // 파일 크기 측정
//     file.seekg(0, std::ios::end);
//     std::streamsize size = file.tellg();
//     file.seekg(0, std::ios::beg);
    
//     // 파일 내용 읽기
//     std::vector<char> fileBuffer(size);
//     file.read(fileBuffer.data(), size);
//     file.close();
    
//     // HTTP 응답 헤더
//     setHeader("Content-Type", getMimeType(getExtension()));
//     setHeader("Content-Length", intToString(size));
//     //setHeader("Connection", "keep-alive");

//     // 파일 내용 추가
//     _body = std::string(fileBuffer.begin(), fileBuffer.end());
// }