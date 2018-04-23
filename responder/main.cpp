#include <cstdlib>
#include <winsock.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;
#define SERVER_PORT 6666          //自定义的服务端口
#define HOSTLEN 256               //主机名长度
#define BACKLOG 10                //同时等待的连接个数
 
int sendall(int s, char *buf, int *len) {
 int total = 0;           // 已经发送字节数
 int bytesleft = *len;                                   //剩余字节数 
 int n;
 while(total < *len) {
  n = send(s, buf+total, bytesleft, 0);
  if (n == -1) { break; }
  total += n;
  bytesleft -= n;
 }
 *len = total;           // 返回实际发送出去的字节数
 return n==-1?-1:0;          // 成功发送返回0 失败-1
}
void bad_req(int sock) {
 char* error = "HTTP/1.0 501 Not Implemented\r\nContent-type: text/plain\r\n\r\n"; 
 int len = strlen(error);
 if (sendall(sock, error, &len) == -1) {   //向客户发送
  printf("Sending failed!");
  return;
 }      
 char* prompt_info = "Bad request!\r\n";
 len = strlen(prompt_info);
 if (sendall(sock, prompt_info, &len) == -1) {
  printf("Sending failed!");
  return;
 }
}
bool not_exit(char* arguments) {
 struct stat dir_info;
 return (stat(arguments, &dir_info) == -1);
}
void file_not_found(char* arguments, int sock) {
 char* error_head = "HTTP/1.0 404 Not Found\r\nContent-type: text/plain\r\n\r\n";  
 int len = strlen(error_head);
 if (sendall(sock, error_head, &len) == -1) {    //向客户端发送
  printf("Sending error!");
  return;
 }   
 char prompt_info[50] = "File not existed:  ";
 strcat(prompt_info, arguments);
 len = strlen(prompt_info);
 if (sendall(sock, prompt_info, &len) == -1) {    //输出未找到的文件
  printf("Sending error!");
  return;
 }    
}
void send_header(int send_to, char* content_type) {
 
 char* head = "HTTP/1.0 200 OK\r\n";     //正确的头部信息
 int len = strlen(head);
 if (sendall(send_to, head, &len) == -1) {   //向连接的客户端发送数据
  printf("Sending error");
  return;
 }
 if (content_type) {         //content_type不为空
  char temp_1[30] = "Content-type: ";    //准备好要连接的字串
  strcat(temp_1, content_type);     //构造content_type
  strcat(temp_1, "\r\n");
  len = strlen(temp_1);
  if (sendall(send_to, temp_1, &len) == -1) {
   printf("Sending error!");
   return;
  }
 }
}
char* file_type(char* arg) {
 char * temp;          //临时字符串指针
 if ((temp=strrchr(arg,'.')) != NULL) {    //取得后缀
  return temp+1;
 }
 return "";           //如果请求的文件名中没有. 则返回空串
}
void send_file(char* arguments, int sock) {
 char* extension = file_type(arguments);    //获得文件后缀名
 char* content_type = "text/plain";     //初始化type='text/plain'
 FILE* read_from;         //本地文件指针从该文件中读取.html .jpg等
 int readed = -1;         //每次读得的字节数
 
 if (strcmp(extension, "html") == 0) {    //发送内容为html
  content_type = "text/html";
 }
 if (strcmp(extension, "gif") == 0) {    //发送内容为gif
  content_type = "image/gif";
 }
 if (strcmp(extension, "jpg") == 0) {    //发送内容为jpg
  content_type = "image/jpg";
 }
  if (strcmp(extension, "png") == 0) {    //发送内容为png
  content_type = "image/png";
 }
 read_from = fopen(arguments, "r");     //打开用户指定的文件准备读取
 if(read_from != NULL) {        //指针不为空
  char read_buf[128];        //读文件时的字节缓存数组
  send_header(sock, content_type);    //发送协议头
  send(sock, "\r\n", 2, 0);      //再加一个"\r\n" 不能缺少 格式要求
  while(!feof(read_from)) {      //判断文件是否已经结束
   fgets(read_buf, 128, read_from);   //读取
   int len = strlen(read_buf);
   if (sendall(sock, read_buf, &len) == -1) { //发送数据
    printf("Sending error!");    //出现发送错误显示到控制台 继续发送
    continue;
   }
  }
 }
}
void handle_req(char* request, int client_sock) {
 char command[BUFSIZ];        //保存解析到的命令字段 GET PUT
 char arguments[BUFSIZ];        //保存解析到的请求的文件
 strcpy(arguments, "./");       //注意该符号在不同操作系统的区别
 if (sscanf(request, "%s%s", command, arguments+2) != 2) {
  return;           //解析出错在返回
 }
 
 printf("handle_cmd:    %s\n",command);    //向控制台输出此时的命令
 printf("handle_path:   %s\n",arguments);   //向控制台输出此时的请求路径
 
 if (strcmp(command, "GET") != 0) {     //请求命令格式是否正确
  bad_req(client_sock);
  return;
 }
 if (not_exit(arguments)) {       //请求的文件是否存在  
  file_not_found(arguments, client_sock);
  return;
 }
 send_file(arguments, client_sock);     //命令格式及请求路径正确则发送数据
 
 return;
}
int make_server_socket() {
 struct sockaddr_in server_addr;       //服务器地址结构体
 int tempSockId;           //临时存储socket描述符
 tempSockId = socket(PF_INET, SOCK_STREAM, 0);
 
 if (tempSockId == -1) {         //如果返回值为－1 则出错
  return -1;
 }
 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons(SERVER_PORT);
 server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //本地地址
 memset(&(server_addr.sin_zero), '\0', 8);
 if (bind(tempSockId, (struct sockaddr *)&server_addr,
  sizeof(server_addr)) == -1) {       //绑定服务如果出错 则返回－1
  printf("bind error!\n");
  return -1;
 }
 if (listen(tempSockId, BACKLOG) == -1 ) {     //开始监听
  printf("listen error!\n");
  return -1;
 }
 return tempSockId;           //返回取得的SOCKET
}
int main(int argc, char * argv[]) {
 WSADATA wsaData;
 if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
  fprintf(stderr, "WSAStartup failed.\n");
  exit(1);
 }
 printf("My web server started...\n");
 int server_socket;        //服务器的socket
 int acc_socket;         //接收到的用户连接的socket
 int sock_size = sizeof(struct sockaddr_in);  
 struct sockaddr_in user_socket;     //客户连接信息
 server_socket = make_server_socket();   //创建服务器端的socket
 if (server_socket == -1) {      //创建socket出错
  printf("Server exception!\n");
  exit(2);
 }
 while(true) {
  acc_socket = accept(server_socket, (struct sockaddr *)&user_socket, &sock_size); //接收连接
  int numbytes;
  char buf[100];
  if ((numbytes=recv(acc_socket, buf, 99, 0)) == -1) {
   perror("recv");
   exit(1);
  }
  handle_req(buf, acc_socket);
 }
 return 0;
}
