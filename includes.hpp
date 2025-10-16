#ifndef includes_HPP
# define includes_HPP

#include <dirent.h>      // opendir(), readdir(), closedir()
#include <fcntl.h>       // fcntl()
#include <netinet/in.h>   // sockaddr_in 구조체, htons(), INADDR_ANY
#include <poll.h>       // poll()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>   // mkdir
#include <sys/socket.h>   // socket(), bind(), listen(), accept(), send(), read()
#include <sys/types.h>    // socket 관련 데이터 타입
#include <unistd.h>       // close()

#include <algorithm>
#include <cerrno>         // errno
#include <climits>
#include <cstring>        // memset, strlen, strerror
#include <fstream>       // std::ifstream
#include <iostream>       // std::cout, std::cerr
#include <map>           // std::map
#include <sstream>       // std::ostringstream
#include <string>        // std::string
#include <vector> 

#include <cstdlib>      // exit()



//UTILS
void setNonBlocking(int sockfd);
std::map<std::string, std::string> initMimeTypes();


//UPLOAD
bool    createUploadDir(const std::string &dir);
bool    saveFile(const std::string &dir, const std::string &filename, const std::string &content);
std::string generateUploadPage(const std::string &uploadDir, int message);

#define BACKLOG 10
#define BUFFER_SIZE 1024

#endif