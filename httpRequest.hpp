#ifndef httpRequest_HPP
# define httpRequest_HPP

#include "includes.hpp"
#include "Server.hpp"

class httpRequest
{
    private:
        //request line
        std::string _method;
        std::string _path;
        std::string _version;
        std::string _queryString;
        // headers
        std::map<std::string, std::string> _headers;
        // body
        std::string _body;
        size_t _contentLength;
    
        // Multipart/form-data
        std::string _boundary;
        std::string _uploadFilename;
        std::string _uploadContent;
        // chunked
        bool isChunked; //
        std::vector<std::string> _chunks; //



    public:
        httpRequest();
        ~httpRequest();
        void parseRequest(const std::string &buffer);
        void parseMultipartBody(const LocationConfig* loc);
        void setBody(const std::string &body) { _body = body; }

        std::string getMethod() const;
        std::string getPath() const;
        std::string getVersion() const;
        std::string getHeaderValue(const std::string &key) const;
        std::string getBody() const;
        std::string getUploadFilename() const;
        std::string getUploadContent() const;

        std::string getQueryString() const;
        // filename
        std::string getFilename() const;
        void setFilename(const std::string &filename);

        // std::string getExtension() const { 
        //     // 실제 파일명(_uploadFilename)에서 확장자 추출 (경로가 아닌)
        //     size_t pos = _uploadFilename.find_last_of('.');
        //     if (pos == std::string::npos || pos == 0)
        //         return "";
            
        //     std::string ext = _uploadFilename.substr(pos);
        //     return ext;
        // }





        void debugPrint() const {
            std::cout << "========== HTTP REQUEST START ==========\n";
            std::cout << _method << " " << _path << " " << _version << "\n";

            std::cout << "Headers:\n";
            for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
                std::cout << it->first << ": " << it->second << "\n";
            }

            if (!_body.empty()) {
                std::cout << "Body:\n" << _body << "\n";
            }

            if (!_boundary.empty())
                std::cout << "Boundary: " << _boundary << "\n";

            if (!_uploadFilename.empty())
                std::cout << "Upload Filename: " << _uploadFilename << "\n";

            if (!_uploadContent.empty())
                std::cout << "Upload Content Size: " << _uploadContent.size() << " bytes\n";

            std::cout << "========== HTTP REQUEST END ==========\n\n";
        }
        
};

#endif