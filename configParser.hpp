#pragma once

#include "includes.hpp"
#include "Server.hpp"

struct dataFlag {
    bool hasPort;
    bool hasRoot; 
    bool hasIndex;
    
    dataFlag() : hasPort(false), hasRoot(false), hasIndex(false) {}
};

class configParser {
	private:
		std::vector<Server *>	servers;
		
	public:
		configParser();
		~configParser();
		
			//FUNCTIONS
		void 							readConfig(std::string &filename, 
										std::vector<Server*>& server_list);
										
		void 							parseConfig(std::ifstream &config, 
										std::vector<Server*>& server_list);
		
			//GETTERS
		const std::vector<Server *>&	getServers() const { return servers; }
};