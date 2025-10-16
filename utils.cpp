#include    "includes.hpp"



void setNonBlocking(int sockfd) 
{
    int fl = fcntl(sockfd, F_GETFL, 0);
    if (fl < 0) {
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, fl | O_NONBLOCK) < 0) {
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::map<std::string, std::string> initMimeTypes() // webserver class 
{
    std::map<std::string, std::string> mimeTypes;

    // text
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"]  = "text/html";
    mimeTypes[".txt"]  = "text/plain";
    mimeTypes[".css"]  = "text/css";
    mimeTypes[".js"]   = "application/javascript";
    mimeTypes[".json"] = "application/json";

    // image
    mimeTypes[".jpg"]  = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".png"]  = "image/png";
    mimeTypes[".gif"]  = "image/gif";
    mimeTypes[".bmp"]  = "image/bmp";
    mimeTypes[".ico"]  = "image/x-icon";
    mimeTypes[".svg"]  = "image/svg+xml";
    mimeTypes[".webp"] = "image/webp";

    // CGI 
    // mimeTypes["py"]   = "text/html";
    // mimeTypes["pl"]   = "text/html";  
    // mimeTypes["cgi"]  = "text/html";

    return mimeTypes;
}