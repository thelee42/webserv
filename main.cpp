#include <iostream>       // std::cout, std::cerr
#include <cstring>        // memset, strlen, strerror
#include <sys/types.h>    // socket 관련 데이터 타입
#include <sys/socket.h>   // socket(), bind(), listen(), accept(), send(), read()
#include <netinet/in.h>   // sockaddr_in 구조체, htons(), INADDR_ANY
#include <unistd.h>       // close()
#include <cerrno>         // errno

#define PORT 8080      // 서버 포트
#define BUFFER_SIZE 1024

#include <fcntl.h>       // fcntl()

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

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    //const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from Thea LEE server!\n";
    const char* response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"
                "<html>"
                "<body>"
                "<h1>Hello from Thea LEE server!</h1>"
                "<p>42 Webserv implementation</p>"
                "</body>"
                "</html>";
    // 1. 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "socket failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    // // 소켓 옵션: 재사용 가능
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 

    // set non-blocking
    //setNonBlocking(server_fd);

    // 2. 서버 주소 정보 설정
    address.sin_family = AF_INET; // IPv4 주소 패밀리
    address.sin_addr.s_addr = INADDR_ANY;  // 모든 IP에서 접속 허용
    address.sin_port = htons(PORT); // 포트 번호 바이트순서로 변환 (htons)

    // 3. 소켓 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // 4. 연결 요청 대기
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server is listening on port " << PORT << "...\n";

    // select - epoll - kqueue

    //연결 루프
    while(true) {
        // 5. 클라이언트 연결 수락
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            // if (errno == EAGAIN || errno == EWOULDBLOCK) //nonblocking
            // //아직 연결 요청이 없음 → 다시 루프 계속
            //    continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //setNonBlocking(new_socket);

        // 6. 클라이언트 메시지 읽기
        if (read(new_socket, buffer, BUFFER_SIZE) <= 0) {
            std::cout << "Client disconnected.\n";
            close(new_socket);
            continue;
        }
        // if (read <= 0)
        //if (errno == EAGAIN || errno == EWOULDBLOCK) //nonblocking
        // 아직 연결 요청이 없음 → 다시 루프 계속
        //    continue;
        if (strncmp(buffer, "GET /favicon.ico", 16) == 0) {
            // 그냥 소켓 닫고 루프 계속
            close(new_socket);
            continue;
        }
        std::cout << "Received request:\n" << buffer << "\n";

        // 7. 응답 전송
        send(new_socket, response, strlen(response), 0);
        std::cout << "Response sent.\n";

        // 8. 소켓 종료
        close(new_socket);
    }
    close(server_fd);
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

