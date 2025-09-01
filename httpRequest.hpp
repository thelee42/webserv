#ifndef httpRequest_HPP
# define httpRequest_HPP

#include <string>
#include <map>
#include <sstream>

class httpRequest
{
    private:
        //request line
        std::string _method;
        std::string _path;
        std::string _version;
        std::string _extension;
        std::string _mimeType;
        std::string _filename;
        // headers
        std::map<std::string, std::string> _headers;
        // body
        std::string _body;
        // else
        std::string _query;


    public:
        httpRequest();
        ~httpRequest();
        std::string parseExtension(const std::string &filename);
        void parseRequestLine(const char *buffer);
        std::string getMethod() const;
        std::string getPath() const;
        std::string getVersion() const;
        std::string getExtension() const;
        std::string getFilename() const;
        void setFilename(const std::string &filename);
};

#endif