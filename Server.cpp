#include "Server.hpp"


///SETTERS///

void		Server::setPort(int port) {
	this->_port = port;
}

void		Server::setServerName(std::string serverName) {
	this->_serverName = serverName;
}
void		Server::setRoot(std::string root) {
	this->_root = root;
}

void		Server::setIndex(std::string index){
	this->_index = index;
}
void		Server::setClientMaxBS(std::size_t clientMaxBS) {
	this->_clientMax_BS = clientMaxBS;
}


///GETTERS///

int			Server::getPort() const {
	return (_port);
}

std::string Server::getServerName() const {
	return (_serverName);
}

std::string Server::getRoot() const {
	return (_root);
}

std::string Server::getIndex() const {
	return (_index);
}

std::size_t Server::getClientMaxBS() const {
	return (_clientMax_BS);
}


///CONSTRUCTORS DESTRUCTORS///

Server::Server() : _port(8080), _root("./www/html"), _serverName("localhost"), _clientMax_BS(100) {
}

Server::~Server() {
}

/////////////////////////////////

void Server::addLocation(const LocationConfig& location) {
    _locations.push_back(location);
}

std::vector<LocationConfig>& Server::getLocationConfig() {
    return _locations;
}

void Server::setErrorPages(std::string errorPages) {
    std::stringstream ss(errorPages);
    std::string errorPage;
    _errorPages.clear();
    while (ss >> errorPage) {
        _errorPages.push_back(errorPage);
    }
}

void Server::addErrorPage(const std::string& errorPage) {
    _errorPages.push_back(errorPage);
}

std::vector<std::string> Server::getErrorPages() const {
    return _errorPages;
}

bool Server::matches(const std::string& host, const std::string& port) const {
    // 포트 문자열 -> int 변환
    int port_num = 0;
    try {
        port_num = atoi(port.c_str());
    } catch (...) {
        // 변환 실패시 기본값 0 or _port 비교 실패 처리
        return false;
    }

    // 서버 포트와 일치 여부 확인
    if (_port != port_num) {
        return false;
    }

    // host와 서버 이름 비교 (대소문자 구분 필요하면 추가 처리)
    if (_serverName == host) {
        return true;
    }

    // 혹시 여러 서버 이름이 있을 경우 처리 (지금은 단일 이름이므로 pass)
    
    return false;
}

//  // request_path에 매칭되는 첫 번째 LocationConfig 반환 (없으면 NULL)
// const LocationConfig* Server::matchLocation(const std::string& request_path) const {
//     const LocationConfig* bestMatch = NULL;
//     size_t bestMatchLength = 0;

//     for (size_t i = 0; i < _locations.size(); ++i) {
//         const std::string& locPath = _locations[i].getPath();

//         // 요청 경로가 location path로 시작하는 경우
//         if (request_path.compare(0, locPath.length(), locPath) == 0) {
//             if (locPath.length() > bestMatchLength) {
//                 bestMatch = &_locations[i];
//                 bestMatchLength = locPath.length();
//             }
//         }
//     }

//     // fallback: "/" 매칭 location 찾기
//     if (!bestMatch) {
//         for (size_t i = 0; i < _locations.size(); ++i) {
//             if (_locations[i].getPath() == "/") {
//                 return &_locations[i];
//             }
//         }
//     }

//     return bestMatch; // NULL 일 수도 있음
// }

const LocationConfig* Server::matchLocation(const std::string &reqPath) const {
    const LocationConfig* best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < _locations.size(); ++i) {
        std::string prefix = _locations[i].getPath();

        // normalize trailing slash ("/"은 그대로)
        if (prefix.size() > 1 && prefix[prefix.size() - 1] == '/')
            prefix.resize(prefix.size() - 1);

        // quick reject: prefix longer than reqPath -> cannot match
        if (prefix.size() > reqPath.size())
            continue;

        // must match at start
        if (reqPath.compare(0, prefix.size(), prefix) != 0)
            continue;

        // boundary check:
        // - if prefix == "/" -> match everything
        // - otherwise match only if exact equal OR next char is '/'
        if (prefix == "/"
            || reqPath.size() == prefix.size()
            || (reqPath.size() > prefix.size() && reqPath[prefix.size()] == '/'))
        {
            if (prefix.size() > bestLen) {
                best = &_locations[i];
                bestLen = prefix.size();
            }
        }
    }
    return best;
}