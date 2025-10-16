#include "includes.hpp"

std::string ft_tostring(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

int ft_back(std::string tmp, char occ) {
    if (tmp.empty()) {
		std::cout << "empty" << std::endl;
        return 0;
	}
    std::size_t length = tmp.length() - 1;
    if (tmp[length] == occ) {
        return 1;
	}
	return 0;
}

void ft_popback(std::string &tmp) {
    if (tmp.empty()) {
		std::cout << "empty" << std::endl;
        return ;
	}
    std::size_t length = tmp.length() - 1;
	tmp.erase(length);
}

int empty_line(std::string line) {
    for (std::size_t i = 0; i < line.size(); i++) {
        if (!std::isspace(line[i]) && line[i] != '\n')
            return (0);
    }
    return (1);
}

std::size_t skipSpaces(const std::string& line, std::size_t pos) {
    std::size_t newPos = line.find_first_not_of(" \t\r\f\v=", pos);
    if (newPos != std::string::npos && line[newPos] != '\n') {
        return (newPos);
    }
    return (pos);
}

int toInt(const std::string &s) {
    int valueToInt;
    std::stringstream string(s);

    string >> valueToInt;
    if (string.fail() || !string.eof())
        throw std::runtime_error("Error: bad value in config");
    return (valueToInt);
}

bool isValidPath(const std::string& path) {
    if (path.empty())
        return false;
    if (path[0] != '/' && path.substr(0, 2) != "./")
        return false;
    std::string invalidChars = "<>|\"*?";
    for (std::size_t i = 0; i < invalidChars.length(); i++) {
        if (path.find(invalidChars[i]) != std::string::npos)
            return false;
    }
    if (path.find("../") != std::string::npos)
        return false;
    return true;
}