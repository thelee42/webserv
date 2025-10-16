#pragma once

#include "includes.hpp"

struct LocationConfig {

	std::string							index;
	std::string							path;
	std::string							redirect;
	std::string							root;
	std::string							uploadDirectory;

	std::size_t							clientMax_BS;

	std::vector<std::string>			allowedExtensions;
	std::vector<std::string>			cgiExtensions;
	std::vector<std::string>			cgiPaths;
	std::vector<std::string>			errorPages;
	std::vector<std::string>			methods;

	LocationConfig() : index("index.html"), path("/"), clientMax_BS(10) {}

	std::string getPath() const { return path; }
	std::string getIndex() const { return index; }
	std::vector<std::string> getMethods() const { return methods; }
	std::string getUploadDirectory() const { return uploadDirectory; }
	//get server root if location root is empty
	std::string getRoot(const std::string& serverRoot) const { return root.empty() ? serverRoot : root; }
	std::string getRedirect() const { return redirect; }
	std::size_t getClientMaxBS() const { return clientMax_BS; }
	std::vector<std::string> getAllowedExtensions() const { return allowedExtensions; }
	std::vector<std::string> getCgiExtensions() const { return cgiExtensions; }
	std::vector<std::string> getCgiPaths() const { return cgiPaths; }
	std::vector<std::string> getErrorPages() const { return errorPages; }

	bool isMethodAllowed(const std::string& method) const {
        for (size_t i = 0; i < methods.size(); ++i) {
            if (methods[i] == method)
                return true;
        }
        return false;
    }
	bool isExtensionAllowed(const std::string& filename) const {
		size_t dotPos = filename.rfind('.');
		if (dotPos == std::string::npos)
			return false; // 확장자가 없음

		std::string ext = filename.substr(dotPos); // 예: ".jpg"

		const std::vector<std::string>& allowed = getAllowedExtensions();
		for (size_t i = 0; i < allowed.size(); ++i) {
			if (ext == allowed[i])
				return true;
		}
		return false; // 허용되지 않은 확장자
	}
};

class Server
{
	private:
		int								_port;
		
		std::string						_index;
		std::string						_root;
		std::string						_serverName;

		std::size_t						_clientMax_BS;

		std::vector<std::string>		_errorPages;
		std::vector<LocationConfig>		_locations;

	public:
		Server();
		~Server();

			//SETTERS
		void							setPort(int port);
		void							setServerName(std::string serverName);
		void							setRoot(std::string root);
		void							setIndex(std::string index);
		void							setClientMaxBS(std::size_t clientMaxBS);
		void							setErrorPages(std::string errorPages);
			//GETTERS
		int								getPort() const;
		std::string 					getServerName() const;
		std::string						getRoot() const;
		std::string						getIndex() const;
		std::size_t						getClientMaxBS() const;


void addLocation(const LocationConfig& location);
std::vector<LocationConfig>& getLocationConfig();
void addErrorPage(const std::string& errorPage);
std::vector<std::string> getErrorPages() const;
//match functions
bool matches(const std::string& host, const std::string& port) const;
const LocationConfig* matchLocation(const std::string& request_path) const;

};