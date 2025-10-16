#ifndef httpResponse_HPP
# define httpResponse_HPP

#include "includes.hpp"

class httpResponse
{
    private:
        std::string _httpVersion;
        int         _statusCode;
        std::string _statusMessage;
        std::map<std::string, std::string> _headers;
        std::string _body;
        std::string _filename;

        static std::map<std::string, std::string> _mimeTypes; //



    public:
        httpResponse();
        virtual ~httpResponse();
        void setStatus(int code, const std::string &messsage);
        void setFilename(const std::string &path, const std::string &root);
        void setHeader(const std::string &key, const std::string &value);
        void setBody(const std::string &body);
        int getStatusCode() const;
        std::string getExtension() const;
        std::string getMimeType(const std::string &extension) const;
        std::string getHeader(const std::string &key) const;
        std::string getBody() const;
        std::string intToString(int value) const;
        void generateFileBody();
        std::string buildCompleteResponse() const;

        void debugPrint() const {
            std::cout << "========== HTTP RESPONSE START ==========\n";
            std::cout << _httpVersion << " " << _statusCode << " " << _statusMessage << "\n";

            std::cout << "Headers:\n";
            for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
                std::cout << it->first << ": " << it->second << "\n";
            }

            if (!_body.empty())
                std::cout << "Body Size: " << _body.size() << " bytes\n";  // 길이만 찍어도 충분

            if (!_filename.empty())
                std::cout << "Filename: " << _filename << "\n";

            std::cout << "========== HTTP RESPONSE END ==========\n\n";
        }

};



#endif