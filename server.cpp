#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#define PORT 8080
#define BUFFER_SIZE 1024

void receiveFile(int clientSocket) {
    char* buffer = new char[BUFFER_SIZE];
    
     

    // 接收文件名
    char filename[BUFFER_SIZE];
    int valread = read(clientSocket, filename, BUFFER_SIZE);
    filename[valread] = '\0';

    // 响应客户端
    std::string response;
    //std::ofstream outputFile(filename, std::ios::out | std::ios::binary);
    if (true) {
        response = "OK";
    } else {
        response = "Error";
    }
    send(clientSocket, response.c_str(), response.length(), 0);
    
    // 如果响应为 "OK"，则接收并保存文件内容
    if (response == "OK") {
                
       // 进入子目录
    int chdirResult = chdir("upload_dir");
    if (chdirResult == -1) {
        std::cerr << "Failed to change directory1" << std::endl;
    }

                std::fstream outputFile1(filename, std::ios::out | std::ios::binary); // 打开文件并使用追加模式
                // 写入文件
                while ((valread = read(clientSocket, buffer, BUFFER_SIZE)) > 0) {
            outputFile1.write(buffer, valread);
        }
                outputFile1.close();
                
    // 回退到父目录
        // chdir("..");
        std::cout << "File received and saved successfully" << std::endl;
    } else {
        std::cerr << "Failed to receive file" << std::endl;
    }
}

void sendFile(int clientSocket, const std::string& filename) {
    char buffer[BUFFER_SIZE] = {0};
    
    // std::cout << filename << std::endl;

    // 发送文件名
   if (send(clientSocket, filename.c_str(), filename.length(), 0)<0){
    std::cout<<"failed to send"<<std::endl;
   };
    


    //打开目录
    int chdirResult = chdir("upload_dir");
    if (chdirResult == -1) {
        std::cerr << "Failed to change directory2" << std::endl;
    }
    else{
        std::cout << "access to dir"<< std::endl;
    }

    // 打开文件进行读取
    std::ifstream file(filename.substr(8), std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file1" << std::endl;
        return;
    }
    
    file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // 从文件读取内容并发送给服务端
        while (file.read(buffer, fileSize)) {
            send(clientSocket, buffer, file.gcount(), 0);
            std::fill(buffer, buffer+BUFFER_SIZE, 0); // 清空缓冲区
        }

        // 关闭文件
        file.close();
        //chdir("..");
    std::cout << "File sent successfully" << std::endl;
}


void handleClientRequest(int clientSocket) {
    char buffer[BUFFER_SIZE] = {0};

    // 接收客户端请求
    int ll = recv(clientSocket,buffer,BUFFER_SIZE,0);
    int valread = recv(clientSocket, buffer,BUFFER_SIZE,0);
    std::string request(buffer);
    std::cout << valread<<std::endl;
    std::cout << request <<std::endl;
    if (request == ("downloaduploadfile.txt")) {
        std::string filename = request;
        sendFile(clientSocket, filename);
    } 
    // else if(request.find("upload"))
    // {
    //     receiveFile(clientSocket);
    // }
    else {
        std::cerr << "Invalid request from client" << std::endl;
        std::string response = "ERROR";
        send(clientSocket, response.c_str(), response.length(), 0);
    }

    close(clientSocket);
}



int main() {
    int serverSocket, newSocket, valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pid_t childpid;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // 创建套接字
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // 设置套接字选项
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 绑定套接字
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // 监听连接请求
    if (listen(serverSocket, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    while(1){
    // 接受连接请求
    if ((newSocket = accept(serverSocket, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
   if((childpid=fork())==0) {
    char rec[10]={0};
    int h = recv(newSocket,rec,6,0);
    // 文件上传
    std::string flag = std::string(rec);
    std::cout << rec <<std::endl;
    if(flag == "upload")
    {
    receiveFile(newSocket);
    exit(0);
    }
    else{
    // 文件下载
    handleClientRequest(newSocket);
    exit(0);
    // 关闭套接字
    }
    
    }
    close(newSocket);
}
close(serverSocket);
return 0;
}
