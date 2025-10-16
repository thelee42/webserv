#include "configParser.hpp"
#include "utils.hpp"
#include <sys/stat.h>

//error pages, shouldBeNB bool?

std::string getConfContentMultiValue(std::string buffer, std::size_t i) {
    std::string tmp;
    i = skipSpaces(buffer, i);
    tmp = buffer.substr(i);
    if (!tmp.empty() && ft_back(tmp, ';'))
        ft_popback(tmp);
    while (!tmp.empty() && (ft_back(tmp, ' ') || ft_back(tmp, '\t') || 
                           ft_back(tmp, '\n') || ft_back(tmp, '\r')))
        ft_popback(tmp);
    if (tmp.empty()) {
        throw std::runtime_error("Error: config value can't be empty");
    }
    return tmp;
}

std::string getConfContentSingleValue(std::string buffer, std::size_t i, bool ShouldBeNB) {
    std::string tmp;

    i = skipSpaces(buffer, i);
    tmp = buffer.substr(i);
    if (!tmp.empty() && ft_back(tmp, ';'))
        ft_popback(tmp);
    while (!tmp.empty() && (ft_back(tmp, ' ') || ft_back(tmp, '\t') || 
                           ft_back(tmp, '\n') || ft_back(tmp, '\r')))
        ft_popback(tmp);
    if (tmp.empty()) {
        throw std::runtime_error("Error: config value can't be empty");
    }
    std::stringstream ss(tmp);
    std::string value, extra;
    ss >> value;
    if (ss >> extra)
        throw std::runtime_error("Error: invalid input " + extra + "");
    if (ShouldBeNB) {
        for (std::size_t pos = 0; pos < value.length(); pos++) {
            if (!std::isdigit(static_cast<unsigned char>(value[pos])))
                throw std::runtime_error("Error: non-numeric value in config");
        }
    }
    return value;
}

void parseLocation(std::ifstream &config, std::vector<Server*>& server_list, std::string buffer, std::size_t i, std::size_t serverIdx) {
    
    LocationConfig  location;
    int             lineNB = 0;
    bool            insideLocation = true;
    
    std::size_t pathStart = buffer.find("location", i) + 8;
    pathStart = skipSpaces(buffer, pathStart);
    std::size_t bracketPos = buffer.find("{", pathStart);
    if (bracketPos == std::string::npos)
        throw std::runtime_error("Error: location brackets not matching");
    location.path = buffer.substr(pathStart, bracketPos - pathStart);
    while (!location.path.empty() && (ft_back(location.path, ' ') || ft_back(location.path, '\t')))
        ft_popback(location.path);
    if (!isValidPath(location.path)) {
        throw std::runtime_error("Error: invalid location path: " + location.path);
    }
    while (std::getline(config, buffer)) {
        lineNB++;
        if (empty_line(buffer))
            continue;
        if (buffer.find("}") != std::string::npos) {
            insideLocation = false;
            break;
        } 
        std::size_t pos;
        if ((pos = buffer.find("methods")) != std::string::npos) {
            std::string methodsList = getConfContentMultiValue(buffer, pos + 7);
            std::stringstream stream(methodsList);
            std::string method; 
            while (stream >> method)
                location.methods.push_back(method);
        }
        else if ((pos = buffer.find("upload_directory")) != std::string::npos) {
            location.uploadDirectory = getConfContentSingleValue(buffer, pos + 16, false);
            if (!isValidPath(location.uploadDirectory)) {
                throw std::runtime_error("Error: invalid upload directory path: " + location.uploadDirectory);
            }
        }
        else if ((pos = buffer.find("client_max_body_size")) != std::string::npos)
            location.clientMax_BS = toInt(getConfContentSingleValue(buffer, pos + 20, true));
        else if ((pos = buffer.find("cgi_extension")) != std::string::npos)
            location.cgiExtensions.push_back(getConfContentSingleValue(buffer, pos + 13, false));
        else if ((pos = buffer.find("cgi_path")) != std::string::npos) {
            std::string cgiPath = getConfContentSingleValue(buffer, pos + 8, false);
            if (!isValidPath(cgiPath)) {
                throw std::runtime_error("Error: invalid CGI path: " + cgiPath);
            }
            location.cgiPaths.push_back(cgiPath);
        }
        else if ((pos = buffer.find("index")) != std::string::npos)
            location.index = getConfContentSingleValue(buffer, pos + 5, false);
        else if ((pos = buffer.find("allowed_extensions")) != std::string::npos) {
            std::string extensionsList = getConfContentMultiValue(buffer, pos + 18);
            std::stringstream stream(extensionsList);
            std::string ext;
            while (stream >> ext)
                location.allowedExtensions.push_back(ext);
        }
        else if ((pos = buffer.find("root")) != std::string::npos) {
            location.root = getConfContentSingleValue(buffer, pos + 4, false);
            if (!isValidPath(location.root)) {
                throw std::runtime_error("Error: invalid root path: " + location.root);
            }
        }
        else if ((pos = buffer.find("redirect")) != std::string::npos)
            location.redirect = getConfContentSingleValue(buffer, pos + 8, false);
        else if ((pos = buffer.find("error_pages")) != std::string::npos) {
            std::string errorPagesList = getConfContentMultiValue(buffer, pos + 11);
            std::stringstream stream(errorPagesList);
            std::string errorPage;
            while (stream >> errorPage)
                location.errorPages.push_back(errorPage);
        }
        else
            throw std::runtime_error("Error: unknown data");
    }
    if (insideLocation)
        throw std::runtime_error("Error: location block not properly closed");
    if (serverIdx < server_list.size())
        server_list[serverIdx]->addLocation(location);
}

void checkServerFields(Server* server, int serverIndex, const dataFlag& flags) {
    if (!flags.hasPort) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " missing required field: port");
    }
    if (!flags.hasRoot) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " missing required field: root");
    }
    if (!flags.hasIndex) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " missing required field: index");
    }
    if (server->getPort() < 1 || server->getPort() > 65535) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " has invalid port: " + ft_tostring(server->getPort()));
    }
    if (server->getRoot().empty()) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " has empty root path");
    }
    if (server->getIndex().empty()) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " has empty index");
    }
    if (!isValidPath(server->getRoot())) {
        throw std::runtime_error("Error: server " + ft_tostring(serverIndex + 1) + " has invalid root path format: " + server->getRoot());
    }
}

void configParser::parseConfig(std::ifstream &config, std::vector<Server*>& server_list) {

    std::vector<dataFlag>	serverFlags;
    std::string     		buffer;
    std::size_t				i = 0;
    int             		j = -1, lineNB = 0, bracketCount = 0;
    bool					insideServer = false, insideLocation = false, foundServer = false;

    while (std::getline(config, buffer)) {
        lineNB++;
        if (empty_line(buffer))
            continue;
        std::size_t pos = 0;
        if ((pos = buffer.find("server {", 0)) != std::string::npos) {
            if (insideServer)
                throw std::runtime_error("Error: nested server block (line " + ft_tostring(lineNB) + ")");
            if (server_list.size() >= 10) {
                throw std::runtime_error("Error: maximum number of servers exceeded");
            }
            std::string beforeServer = buffer.substr(0, pos);
            if (!empty_line(beforeServer))
                throw std::runtime_error("Error: out of bounds content (line " + ft_tostring(lineNB) + ")");
            std::string afterServer = buffer.substr(pos + 8);
            if (!empty_line(afterServer))
                throw std::runtime_error("Error: out of bounds content (line " + ft_tostring(lineNB) + ")");
            foundServer = true;
            insideServer = true;
            bracketCount++;
            server_list.push_back(new Server);
            serverFlags.push_back(dataFlag());
            j++;
            continue;
        }
        if ((pos = buffer.find("}", 0)) != std::string::npos) {
            if (!insideServer) {
                throw std::runtime_error("Error: opening & closing brackets not matching (line " + ft_tostring(lineNB) + ")");
            }
            std::string beforeBracket = buffer.substr(0, pos);
            std::string afterBracket = buffer.substr(pos + 1);
            if (!empty_line(beforeBracket) || !empty_line(afterBracket)) {
                throw std::runtime_error("Error: out of bounds content (line " + ft_tostring(lineNB) + ")");
            }
            if (j >= 0 && static_cast<std::size_t>(j) < server_list.size()) {
                checkServerFields(server_list[j], j, serverFlags[j]);
            }
            insideServer = false;
            bracketCount--;
            continue;
        }
        if (!insideServer)
            throw std::runtime_error("Error: out of bounds content (line " + ft_tostring(lineNB) + ")");
        if ((j >= 0 && (i = buffer.find("listen")) != std::string::npos) && !insideLocation) {
            std::string portValue = getConfContentSingleValue(buffer, i + 6, true);
            int port = toInt(portValue);
            if (port < 1 || port > 65535) {
                throw std::runtime_error("Error: port must be >= 1 && <= 65535 (line " + ft_tostring(lineNB) + ")");
            }
            server_list[j]->setPort(port);
            serverFlags[j].hasPort = true;
        }
        else if ((j >= 0 && (i = buffer.find("server_name")) != std::string::npos) && !insideLocation)
            server_list[j]->setServerName(getConfContentSingleValue(buffer, i + 11, false));
        else if ((j >= 0 && (i = buffer.find("root")) != std::string::npos) && !insideLocation) {
            std::string rootPath = getConfContentSingleValue(buffer, i + 4, false);
            if (!isValidPath(rootPath)) {
                throw std::runtime_error("Error: invalid root path: " + rootPath + " (line " + ft_tostring(lineNB) + ")");
            }
            server_list[j]->setRoot(rootPath);
            serverFlags[j].hasRoot = true;
        }
        else if ((j >= 0 && (i = buffer.find("index")) != std::string::npos)) {
            server_list[j]->setIndex(getConfContentSingleValue(buffer, i + 5, false));
            serverFlags[j].hasIndex = true;
        }
        else if ((j >= 0 && (i = buffer.find("client_max_body_size")) != std::string::npos) && !insideLocation)
            server_list[j]->setClientMaxBS(toInt(getConfContentSingleValue(buffer, i + 20, true)));
        else if ((j >= 0 && (i = buffer.find("error_pages")) != std::string::npos) && !insideLocation) {
            std::string errorPagesStr = getConfContentMultiValue(buffer, i + 11);
            std::stringstream ss(errorPagesStr);
            std::string errorPage;
            while (ss >> errorPage) {
                server_list[j]->addErrorPage(errorPage);
            }
        }
        else if ((j >= 0 && (i = buffer.find("location ")) != std::string::npos) && !insideLocation) {
            insideLocation = true;
            parseLocation(config, server_list, buffer, i, j);
            insideLocation = false;
        }
        else
            throw std::runtime_error("Error: unknown data (line " + ft_tostring(lineNB) + ")");
    }
    if (!foundServer)
        throw std::runtime_error("Error: no server in config file");
    if (bracketCount != 0)
        throw std::runtime_error("Error: opening & closing brackets not matching");
    if (insideServer)
        throw std::runtime_error("Error: opening & closing brackets not matching");
    for (std::size_t i = 0; i < server_list.size(); i++) {
        checkServerFields(server_list[i], static_cast<int>(i), serverFlags[i]);
    }
}

void configParser::readConfig(std::string &filename, std::vector<Server*>& server_list) {
    
	std::ifstream config;

    config.open(filename.c_str(), std::fstream::in);
    if (!config.is_open())
        throw std::runtime_error("Error: couldn't open config file: " + filename);
    try {
        parseConfig(config, server_list);
    } catch (const std::exception& e) {
        config.close();
        throw;
    }
    config.close();
}

configParser::configParser() {
}

configParser::~configParser() {
    for (std::size_t i = 0; i < servers.size(); i++) {
        delete servers[i];
    }
}