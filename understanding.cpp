#include "includes.hpp"
#include "httpRequest.hpp"
#include "httpResponse.hpp"
#include "configParser.hpp"
#include "Server.hpp"


int main(int ac, char **av) {
    configParser                config;
    std::vector<Server *>       server_list;
    std::string                 filename(av[1]);

    //READING CONFIG//
    try {
        config.readConfig(filename, server_list);
        std::cout << "Success" << std::endl;
    } 
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
    if (server_list.empty()) {
        std::cerr << "Error: empty servers" << std::endl;
        return 1;
    }


    std::vector<int>    server_fds;
    server_fds.resize(server_list.size());
    std::map<std::string, std::string> mimeTypes = initMimeTypes();

    std::vector<struct pollfd> poll_fds;

    // server sockets in poll fd
    for (size_t i = 0; i < server_list.size(); i++){
        /* 서버를 포트에 연결하고 소켓을 만들고 poll 에 담아두고 대기 */
    }

    char buffer[BUFFER_SIZE];
    std::map<int, std::string> client_buffers;
    std::map<int, std::string> client_send_buffers;
    std::map<int, size_t> client_send_size;
    //연결 루프
    while(true) {
        //poll init        
        for (size_t i = 0; i < poll_fds.size(); i++) {
            //에러, 연결종료 처리
            if (poll_fds[i].revents & POLLIN) {
                poll_fds[i].revents = 0;
                if (i < server_list.size()) /* server socket */{
                    /* 서버 소켓은 새로운 클라이언트와 accept 처리
                    accept 되면 poll 에 담아둠 */
                }
                else /* client socket */{
                    int client_fd = poll_fds[i].fd;
                    bool should_close = false;
                    
                    while (!should_close) {
                        /* recv로 요청 읽고 저장 */
                    }
                    
                    // 연결을 닫아야 하거나 완전하지 않은 요청인 경우
                    if (should_close) {
                        /* 닫아야 하는 경우 닫기 처리 */
                    }
                    
                    // 완전한 HTTP 요청이 수신되었는지 확인
                    std::string& request = client_buffers[client_fd];
                    size_t header_end = request.find("\r\n\r\n");
                    if (header_end != std::string::npos) {
                        bool request_complete = true;
                        size_t content_length = 0;
                        
                        // Content-Length 확인
                        std::string headers = request.substr(0, header_end + 4);
                        size_t pos = headers.find("Content-Length:");
                        if (pos != std::string::npos) {
                            /* 헤더는 다 받았는데 바디를 안 맞은 경우 non complet 표시 */
                        }
                        
                        if (request_complete) {
                            // 완전한 요청 수신됨 - 처리 시작
                            // 응답 작성
                            //응답 준비 되면 POLLOUT 으로 변경
                            //poll_fds[i].events = POLLOUT; 
                        }
                    }
                }
            }

            // send event
            if (poll_fds[i].revents & POLLOUT && i >= server_list.size()) {
                //poll fd 가 POLLOUT 이고 client 소켓인 경우
                int client_fd = poll_fds[i].fd;
                std::string& send_buffer = client_send_buffers[client_fd];
                size_t& send_size = client_send_size[client_fd];

                if (send_size < send_buffer.size()) {
                    //보내고 send 사이즈 업데이트.
                }
                if (send_size >= send_buffer.size()) {
                    //응답 길이만큼 다 보냈으면 fd 닫기.
                }
            }
        }
    }
    for (size_t i = 0; i < poll_fds.size(); i++) {
        close(poll_fds[i].fd);
    }
    return 0;
}
