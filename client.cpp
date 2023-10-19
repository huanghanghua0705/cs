#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#define PORT 8080
#define BUFFER_SIZE 1024

void sendFile(int clientSocket, const std::string& filename) {
    char* buffer = new char[BUFFER_SIZE];

    send(clientSocket, "upload", 6, 0);
    // 发送文件上传请求
    std::string request = filename;
    
    if(send(clientSocket, ("upload" + request).c_str(), ("upload" + request).length(), 0)<0){
        std::cout<<"upload failed!"<<std::endl;
    };
    
    
    // 等待服务端响应
    char response[BUFFER_SIZE] = {0};
    int valread = read(clientSocket, response, BUFFER_SIZE);
    response[valread] = '\0';



    if (std::string(response) == "OK") {
        // 打开文件进行读取
        std::ifstream file(request, std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file" << std::endl;
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
        
        std::cout << "File sent successfully" << std::endl;
    } else {
        std::cerr << "Server rejected the upload request" << std::endl;
    }
}

void receiveFile(int clientSocket, const std::string& filename) {
    char buffer[BUFFER_SIZE] = {0};

    send(clientSocket, "download", 8, 0);
    // 发送文件下载请求
    std::string request =filename;
    send(clientSocket, ( "download" + request).c_str(), ( "download" + request).length(), 0);
    
    // 等待服务端响应
    char response[BUFFER_SIZE] = {0};
    // std::cout << "1"<<std::endl;
    int valread = read(clientSocket, response, BUFFER_SIZE);
    // std::cout << "2"<<std::endl;
    response[valread] = '\0';
    
    if (std::string(response).find("download")) {
        //打开目录
        int chdirResult = chdir("download_dir");
    if (chdirResult == -1) {
        std::cerr << "Failed to change directory4" << std::endl;
    }


        // 打开文件进行写入
        std::ofstream file("download"+request, std::ios::out | std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }
        
        // 从服务端接收文件内容并写入文件
        while ((valread = read(clientSocket, buffer, BUFFER_SIZE)) > 0) {
            file.write(buffer, valread);
        }
        
        // 关闭文件
        file.close();
        chdir("..");
        std::cout << "File received successfully" << std::endl;
    } else {
        std::cerr << "Server rejected the download request" << std::endl;
    }
}

void dirlist(std::string dirname)
{
     // 打开目录
    DIR* dir = opendir(dirname.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory3" << std::endl;
    }

    // 读取目录中的条目
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 处理目录条目
        std::cout << entry->d_name << std::endl;
    }

    // 关闭目录
    closedir(dir);

}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
  
      while(true)
    {
    // 创建套接字
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    
    // 将IPv4地址从字符串转换为二进制形式
    if (inet_pton(AF_INET, "192.168.153.128", &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }
  
    
    // 连接到服务器
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }
  
    std::cout << "Enter 'upload' to upload a file or 'download' to download a file or 'list' to list all files of the dir or 'q' to exit: ";
    std::string choice;
    std::cin >> choice;
    
    if (choice == "upload") {
        std::string filename;
        std::cout << "Enter the filename to upload: ";
        std::cin >> filename;
        sendFile(clientSocket, filename);
    } else if (choice == "download") {
        std::string filename;
        std::cout << "Enter the filename to download: ";
        std::cin >> filename;
        receiveFile(clientSocket, filename);
    }
    else if(choice=="q")
    {
        std::cout<<"niyituichu"<<std::endl;
        break;
    }

    else if(choice == "list")
    {
        std::string dirname;
        std::cout << "Enter the dirname to list: ";
        std::cin >> dirname;
        dirlist(dirname);
    }    
     else {
        std::cerr << "Invalid choice" << std::endl;
    }
        // 关闭套接字
    close(clientSocket);
    }
        

    return 0;
}
