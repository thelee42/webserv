
#include "httpRequest.hpp"
#include "httpResponse.hpp"
#include "configParser.hpp"
#include "Server.hpp"
#include "includes.hpp"


Server* matchServer(const httpRequest& req, const std::vector<Server*>& servers) {
    std::string host_header = req.getHeaderValue("Host");
    std::string host_name;
    std::string port;

    // "localhost:8080" → "localhost", "8080"
    size_t colon = host_header.find(':');
    if (colon != std::string::npos) {
        host_name = host_header.substr(0, colon);
        port = host_header.substr(colon + 1);
    } else {
        host_name = host_header;
        // 포트는 client_fd 가 연결된 포트로 추정
    }

    // 서버 리스트에서 매칭
    for (size_t i = 0; i < servers.size(); ++i) {
        if (servers[i]->matches(host_name, port)) {
            return servers[i];
        }
    }

    // 못 찾으면 첫 번째 서버 fallback
    return servers[0];
}


struct ClientConnexion {
    std::string recv_buffer;
    std::string send_buffer;
    size_t send_offset;
    bool request_complete;
    bool header_complete;
    size_t header_end;
    size_t content_length;
    httpRequest request;
    httpResponse response;
    const Server* serv;
    const LocationConfig* loc;

    // 생성자에서 초기화
    ClientConnexion()
        : recv_buffer(),
          send_buffer(),
          send_offset(0),
          request_complete(false),
          header_complete(false),
          header_end(0),
          content_length(0),
          request(),       // 기본 생성자 호출
          response(),      // 기본 생성자 호출
          serv(NULL),      // nullptr 대신 NULL (C++98)
          loc(NULL)
    {
        // 빈 바디
    }
};


int setServerSocket(int port) {

    
    // 1. 소켓 생성
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "socket failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    // 소켓 옵션: 재사용 가능
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 

    // 2. 서버 주소 정보 설정
    struct sockaddr_in address;
    address.sin_family = AF_INET; // IPv4 주소 패밀리
    address.sin_addr.s_addr = INADDR_ANY;  // 모든 IP에서 접속 허용
    address.sin_port = htons(port); // 포트 번호 바이트순서로 변환 (htons)

    // 3. 소켓 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // 4. 연결 요청 대기
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    setNonBlocking(server_fd);
    
    return server_fd;
}

int main(int ac, char **av) {

    if (ac != 2) {
		std::cerr << "Error: need config file" << std::endl;
		return (1);
    }

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
        int fd = setServerSocket(server_list[i]->getPort());
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        poll_fds.push_back(pfd);
        std::cout << "Server is listening on port " << server_list[i]->getPort() << "...\n";
    }

    char buffer[BUFFER_SIZE];
    std::map<int, ClientConnexion> clients;
    //연결 루프
    while(true) {
        //poll init
        int activity = poll(poll_fds.data(), poll_fds.size(), -1);
        if (activity < 0) {
            std::cerr << "poll failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE); 
        }
        
        for (size_t i = 0; i < poll_fds.size(); i++) {
            if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(poll_fds[i].fd);
                clients.erase(poll_fds[i].fd);
                poll_fds.erase(poll_fds.begin() + i);
                --i;
                continue;
            }
            if (poll_fds[i].revents & POLLIN) {
                //poll_fds[i].revents = 0;
                if (i < server_list.size()) /* server socket */{
                    struct sockaddr_in clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);
                    int new_socket = accept(poll_fds[i].fd, (struct sockaddr *)&clientAddr, &clientLen);
                    if (new_socket < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
                        std::cerr << "accept failed: " << strerror(errno) << std::endl;
                        continue;
                    }
                    setNonBlocking(new_socket);
                    struct pollfd client_pfd;
                    client_pfd.fd = new_socket;
                    client_pfd.events = POLLIN;
                    client_pfd.revents = 0;
                    poll_fds.push_back(client_pfd);
                    clients[new_socket] = ClientConnexion(); // 버퍼 초기화
                    std::cout << "New client fd=" << new_socket << "\n";
                }
                else /* client socket */{
                    int client_fd = poll_fds[i].fd;
                    
                    memset(buffer, 0, BUFFER_SIZE); // 버퍼 초기화
                    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
                    
                    if (bytes_received > 0)
                        clients[client_fd].recv_buffer.append(buffer, bytes_received);
                    else if (bytes_received == 0 || (bytes_received < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                        std::cout << "Client fd=" << client_fd << " disconnected\n";
                        close(client_fd);
                        clients.erase(client_fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        --i;
                        continue;
                    }
                    // 완전한 HTTP 요청이 수신되었는지 확인
                    if (!clients[client_fd].header_complete) {
                        clients[client_fd].header_end = clients[client_fd].recv_buffer.find("\r\n\r\n");
                        if (clients[client_fd].header_end != std::string::npos) { // 헤더가 완전하게 수신됨
                            clients[client_fd].header_complete = true;
                            clients[client_fd].request.parseRequest(clients[client_fd].recv_buffer); // parse request, header, body..
                            clients[client_fd].serv = matchServer(clients[client_fd].request, server_list);
                            clients[client_fd].loc = clients[client_fd].serv->matchLocation(clients[client_fd].request.getPath());
                            
                            if (clients[client_fd].loc == NULL) {
                                clients[client_fd].response.setStatus(404, "Not Found");
                                //응답 작성 함수
                                clients[client_fd].response.setFilename(clients[client_fd].request.getPath(), clients[client_fd].loc->getRoot(clients[client_fd].serv->getRoot()));
                                clients[client_fd].response.generateFileBody();
                                clients[client_fd].send_buffer = clients[client_fd].response.buildCompleteResponse();
                                clients[client_fd].send_offset = 0;
                                clients[client_fd].recv_buffer.clear();
                                poll_fds[i].events = POLLOUT;
                                continue;
                            }

                            // 허용 메소드 확인
                            if (!clients[client_fd].loc->isMethodAllowed(clients[client_fd].request.getMethod())) {
                                //errorpages
                                clients[client_fd].response.setStatus(405, "Method Not Allowed");
                                //응답 작성 함수
                                poll_fds[i].events = POLLOUT; 
                                continue;
                            }

                            // Content-Length 확인
                            std::string content = clients[client_fd].request.getHeaderValue("Content-Length");
                            clients[client_fd].content_length = 0;
                            //GET, DELETE 일 때
                            if ((clients[client_fd].request.getMethod() == "GET" || clients[client_fd].request.getMethod() == "DELETE") && !content.empty()) {
                                clients[client_fd].response.setStatus(400, "Bad Request");
                                //응답 작성 함수
                                poll_fds[i].events = POLLOUT; 
                                continue;
                            }
                            // POST일 때
                            if (clients[client_fd].request.getMethod() == "POST") {
                                if (content.empty() && clients[client_fd].request.getHeaderValue("Transfer-Encoding") != "chunked") {
                                    clients[client_fd].response.setStatus(411, "Length Required");
                                    //응답 작성 함수
                                    poll_fds[i].events = POLLOUT; 
                                    continue;
                                }
                                if (clients[client_fd].request.getHeaderValue("Transfer-Encoding") == "chunked") {
                                    // 청크 인코딩 처리
                                }
                                else if (!content.empty()) {
                                    clients[client_fd].content_length = std::atoi(content.c_str());
                                    // max body size 확인
                                    if (clients[client_fd].content_length > clients[client_fd].loc->getClientMaxBS() * 1024 * 1024) /* BS * mb */ {
                                        clients[client_fd].response.setStatus(413, "Payload Too Large");
                                        //응답 작성 함수
                                        poll_fds[i].events = POLLOUT;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                    if (clients[client_fd].header_complete && !clients[client_fd].request_complete) {
                        size_t total_needed = clients[client_fd].header_end + 4 + clients[client_fd].content_length;
                        if (clients[client_fd].recv_buffer.size() >= total_needed) {
                            clients[client_fd].request_complete = true;
                            clients[client_fd].request.setBody(clients[client_fd].recv_buffer.substr(clients[client_fd].header_end + 4, clients[client_fd].content_length));
                        }
                    }

                    if (clients[client_fd].request_complete) {
                        clients[client_fd].request.debugPrint();
                        // 완전한 요청 수신됨 - 처리 시작
                        std::cout << "Received request from fd=" << client_fd << ":\n" << clients[client_fd].recv_buffer << "\n";
                        clients[client_fd].response.setStatus(200, "OK");
                        
                        // 리다이렉트 처리
                        if (clients[client_fd].request.getPath() == "/helloworld") {
                            clients[client_fd].response.setStatus(302, "Found");
                            //if (clients[client_fd].loc->getRedirect().empty()) {
                            //    clients[client_fd].response.setStatus(500, "Internal Server Error");
                            //}
                            clients[client_fd].response.setHeader("Location", clients[client_fd].loc->getRedirect());
                            clients[client_fd].response.setHeader("Content-Length", "0");
                        }
                        // 파일 업로드 처리
                        else if (clients[client_fd].loc->getPath() == "/upload") /* loc->getPath() */{
                                // server root
                            std::string baseRoot = clients[client_fd].serv->getRoot();
                            if (!baseRoot.empty() && baseRoot[baseRoot.size() - 1] != '/')
                                baseRoot += "/";

                            // location upload_directory
                            std::string upDir = clients[client_fd].loc->getUploadDirectory();
                            if (!upDir.empty() && upDir[0] == '/')
                                upDir = upDir.substr(1); // "/uploads" → "uploads"

                            // 최종 업로드 디렉토리
                            std::string uploadDir = baseRoot + upDir;

                            // 요청된 경로에서 /upload 이후 부분 추출
                            std::string relPath = clients[client_fd].request.getPath().substr(
                                clients[client_fd].loc->getPath().size()
                            );
                            if (!relPath.empty() && relPath[0] == '/')
                                relPath = relPath.substr(1);

                            if (clients[client_fd].request.getMethod() == "GET") {
                                if (relPath.empty()) {
                                    int message = 1; // 상태 메시지 (1: 없음, 2: 성공, 3: 실패, 4: 확장자 오류)
                                    std::string query = clients[client_fd].request.getQueryString();
                                        if (query.find("msg=2") != std::string::npos)
                                            message = 2;
                                        else if (query.find("msg=3") != std::string::npos)
                                            message = 3;
                                        else if (query.find("msg=4") != std::string::npos)
                                            message = 4;
                                    
                                    // /upload → 업로드 페이지
                                    std::string body = generateUploadPage(uploadDir, message);
                                    clients[client_fd].response.setHeader("Content-Type", "text/html");
                                    clients[client_fd].response.setHeader("Content-Length", clients[client_fd].response.intToString(body.size()));
                                    clients[client_fd].response.setBody(body);
                                }
                                else {
                                    // /upload/filename.txt → 파일 다운로드
                                    std::string filepath = upDir + "/" + relPath;
                                    clients[client_fd].response.setFilename(filepath, baseRoot);
                                    clients[client_fd].response.generateFileBody();
                                }
                            }
                            else if (clients[client_fd].request.getMethod() == "POST" && clients[client_fd].request.getHeaderValue("Content-Type").find("multipart/form-data") != std::string::npos) {
                                clients[client_fd].request.parseMultipartBody(clients[client_fd].loc); // check extension
                                    if (clients[client_fd].request.getUploadFilename().empty()) {
                                        std::cerr << "Upload failed: No valid extension or file\n";
                                        clients[client_fd].response.setStatus(303, "See Other");
                                        clients[client_fd].response.setHeader("Location", "/upload?msg=4");
                                        clients[client_fd].response.setHeader("Content-Length", "0");
                                        clients[client_fd].response.setBody("");
                                        // poll_fds[i].events = POLLOUT;
                                        // continue;
                                    }
                                if (!clients[client_fd].request.getUploadFilename().empty()) {
                                    if (saveFile(uploadDir, clients[client_fd].request.getUploadFilename(), clients[client_fd].request.getUploadContent())) {
                                        std::cout << "File uploaded: " << clients[client_fd].request.getUploadFilename() << "\n";
                                        clients[client_fd].response.setStatus(303, "See Other");
                                        clients[client_fd].response.setHeader("Location", "/upload?msg=2");
                                        clients[client_fd].response.setHeader("Content-Length", "0");
                                        clients[client_fd].response.setBody("");
                                    } 
                                    else {
                                        std::cerr << "File upload failed: " << clients[client_fd].request.getUploadFilename() << "\n";
                                        clients[client_fd].response.setStatus(303, "See Other");
                                        clients[client_fd].response.setHeader("Location", "/upload?msg=3");
                                        clients[client_fd].response.setHeader("Content-Length", "0");
                                        clients[client_fd].response.setBody("");
                                    }
                                }
                            }
                                // DELETE
                            else if (clients[client_fd].request.getMethod() == "DELETE") {
                                if (relPath.empty()) {
                                    clients[client_fd].response.setStatus(400, "Bad Request");
                                    std::string body = "{\"success\": false, \"message\": \"No file specified\"}";
                                    clients[client_fd].response.setHeader("Content-Type", "application/json");
                                    clients[client_fd].response.setHeader(
                                        "Content-Length", clients[client_fd].response.intToString(body.size())
                                    );
                                    clients[client_fd].response.setBody(body);
                                } else {
                                    std::string filepath = uploadDir + "/" + relPath;
                                    if (remove(filepath.c_str()) == 0) {
                                        clients[client_fd].response.setStatus(200, "OK");
                                        clients[client_fd].response.setHeader("Content-Type", "application/json");
                                        std::string body = "{\"success\": true, \"message\": \"File deleted\"}";
                                        clients[client_fd].response.setHeader(
                                            "Content-Length", clients[client_fd].response.intToString(body.size())
                                        );
                                        clients[client_fd].response.setBody(body);
                                    } else {
                                        clients[client_fd].response.setStatus(404, "Not Found");
                                        clients[client_fd].response.setHeader("Content-Type", "application/json");
                                        std::string body = "{\"success\": false, \"message\": \"File not found\"}";
                                        clients[client_fd].response.setHeader(
                                            "Content-Length", clients[client_fd].response.intToString(body.size())
                                        );
                                        clients[client_fd].response.setBody(body);
                                    }
                                }
                            }
                            
                        }
                        else {
                            clients[client_fd].response.setFilename(clients[client_fd].request.getPath(), clients[client_fd].loc->getRoot(clients[client_fd].serv->getRoot()));
                            clients[client_fd].response.generateFileBody();
                        }
                        clients[client_fd].send_buffer = clients[client_fd].response.buildCompleteResponse();
                        clients[client_fd].send_offset = 0;
                        clients[client_fd].recv_buffer.clear();
                        poll_fds[i].events = POLLOUT; 
                        std::cout << "Response prepared for fd=" << client_fd << ", switching to POLLOUT\n";
                        clients[client_fd].response.debugPrint();
                    }
                }
            }
            // send event
            if (poll_fds[i].revents & POLLOUT && i >= server_list.size()) {
                int client_fd = poll_fds[i].fd;
                std::string& send_buffer = clients[client_fd].send_buffer;
                size_t& send_offset = clients[client_fd].send_offset;

                if (send_offset < send_buffer.size()) {
                    ssize_t bytes_sent = send(client_fd, 
                                            send_buffer.data() + send_offset, 
                                            send_buffer.size() - send_offset, 
                                            0);
                    
                    if (bytes_sent > 0) {
                        send_offset += bytes_sent;
                        std::cout << "Sent " << bytes_sent << " bytes to fd=" << client_fd 
                                 << " (" << send_offset << "/" << send_buffer.size() << ")\n";
                    }
                    else if (bytes_sent < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            std::cerr << "send error: " << strerror(errno) << std::endl;
                            // 오류 발생 시 연결 종료
                            close(client_fd);
                            clients.erase(client_fd);
                            poll_fds.erase(poll_fds.begin() + i);
                            --i;
                            continue;
                        }
                    }
                }
                if (send_offset >= send_buffer.size()) {
                    std::cout << "Response completely sent to fd=" << client_fd << "\n";
                    close(client_fd);
                    clients.erase(client_fd);
                    poll_fds.erase(poll_fds.begin() + i);
                    --i;
                }
            }
        }
    }
    for (size_t i = 0; i < poll_fds.size(); i++) {
        close(poll_fds[i].fd);
    }
    return 0;
}



/*
1. socket 생성 : server_fd = socket(AF_INET, SOCK_STREAM, 0)
2. address 정보 설정 : struct sockaddr_in address
    address.sin_family = AF_INET;           IPv4 주소 패밀리 (AF_INET)
    address.sin_addr.s_addr = INADDR_ANY;   IP 주소 (INADDR_ANY, 모든 IP에서 접속 허용)
    address.sin_port = htons(PORT);          포트 번호 (htons(PORT), 바이트 순서 변환)
3. bind() 소켓과 주소 정보 연결
4. listen() 클라이언트 연결 요청 대기
5. epoll 생성
-----------
6. 클라이언트 요청 처리 loop (요청은 브라우저가 자동으로 보냄)
    - accept() 클라이언트 연결 수락 
    - read() 클라이언트 요청 읽기
    - 응답 작성 const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from Thea LEE server!\n";
    - send() 클라이언트 응답 전송
    - 소켓 종료 close()
7. 서버 소켓 닫기 close() *조건 필요





1. socket() - 소켓 생성
int socket (int domain, int type, int protocol)
    domain : AF_INET(IPv4)
    type : SOCK_STREAM(TCP)
    protocol : 0 (자동 선택)
return : socket fd or -1

2. bind() - 소켓에 IP+port 할당
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    sockfd : socket() 호출로 생성된 소켓의 파일 디스크립터
    addr : 바인딩할 주소(IP/포트) 정보 (sockaddr_in 구조체)
    addrlen : 주소 정보의 길이  (ex. sizeof(struct sockaddr_in))
return : 0 성공, -1 실패

3. listen() - 소켓 연결대기
int listen(int sockfd, int backlog)
    sockfd : socket() 호출로 생성된 소켓의 파일 디스크립터
    backlog : 대기열에 들어갈 수 있는 최대 클라이언트 수(큐 크기)
return : 0 성공, -1 실패

+
epoll_create1()
    int epfd = epoll_create1(0);
epoll_ctl()
    struct epoll_event ev;
    ev.events = EPOLLIN;   // 읽기 이벤트
    ev.data.fd = server_fd; // 서버 소켓
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);
epoll_wait()
    int n = epoll_wait(epfd, events, MAX_EVENTS, -1);


4. accept() - 클라이언트 연결 수락
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    sockfd : socket() 호출로 생성된 소켓의 파일 디스크립터
    addr : 클라이언트 주소 정보를 저장할 구조체 (sockaddr_in 구조체)
    addrlen : 주소 정보의 길이 (ex. sizeof(struct sockaddr_in))
return : 클라이언트 소켓의 파일 디스크립터 또는 -1 실패

5. read() - 클라이언트 요청 읽기 : 브라우저가 자동으로 해줌
ssize_t read(int fd, void *buf, size_t count)
    fd : 읽을 파일 디스크립터 (클라이언트 소켓)
    buf : 읽은 데이터를 저장할 버퍼
    count : 읽을 최대 바이트 수
return : 실제로 읽은 바이트 수 또는 -1 실패

6. send() - 클라이언트에게 응답 전송
ssize_t send(int sockfd, const void *buf, size_t len, int flags)
    sockfd : 클라이언트 소켓의 파일 디스크립터
    buf : 전송할 데이터 (응답 메시지)
    len : 전송할 데이터의 길이
    flags : 0 (기본 플래그)
return : 실제로 전송된 바이트 수 또는 -1 실패

7. close() - 소켓 종료
int close(int fd)
    fd : 종료할 파일 디스크립터 (소켓)
return : 0 성공, -1 실패

*/

