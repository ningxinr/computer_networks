#include <cstdlib>
#include <winsock.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;
#define SERVER_PORT 6666          //�Զ���ķ���˿�
#define HOSTLEN 256               //����������
#define BACKLOG 10                //ͬʱ�ȴ������Ӹ���
 
int sendall(int s, char *buf, int *len) {
 int total = 0;           // �Ѿ������ֽ���
 int bytesleft = *len;                                   //ʣ���ֽ��� 
 int n;
 while(total < *len) {
  n = send(s, buf+total, bytesleft, 0);
  if (n == -1) { break; }
  total += n;
  bytesleft -= n;
 }
 *len = total;           // ����ʵ�ʷ��ͳ�ȥ���ֽ���
 return n==-1?-1:0;          // �ɹ����ͷ���0 ʧ��-1
}
void bad_req(int sock) {
 char* error = "HTTP/1.0 501 Not Implemented\r\nContent-type: text/plain\r\n\r\n"; 
 int len = strlen(error);
 if (sendall(sock, error, &len) == -1) {   //��ͻ�����
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
 if (sendall(sock, error_head, &len) == -1) {    //��ͻ��˷���
  printf("Sending error!");
  return;
 }   
 char prompt_info[50] = "File not existed:  ";
 strcat(prompt_info, arguments);
 len = strlen(prompt_info);
 if (sendall(sock, prompt_info, &len) == -1) {    //���δ�ҵ����ļ�
  printf("Sending error!");
  return;
 }    
}
void send_header(int send_to, char* content_type) {
 
 char* head = "HTTP/1.0 200 OK\r\n";     //��ȷ��ͷ����Ϣ
 int len = strlen(head);
 if (sendall(send_to, head, &len) == -1) {   //�����ӵĿͻ��˷�������
  printf("Sending error");
  return;
 }
 if (content_type) {         //content_type��Ϊ��
  char temp_1[30] = "Content-type: ";    //׼����Ҫ���ӵ��ִ�
  strcat(temp_1, content_type);     //����content_type
  strcat(temp_1, "\r\n");
  len = strlen(temp_1);
  if (sendall(send_to, temp_1, &len) == -1) {
   printf("Sending error!");
   return;
  }
 }
}
char* file_type(char* arg) {
 char * temp;          //��ʱ�ַ���ָ��
 if ((temp=strrchr(arg,'.')) != NULL) {    //ȡ�ú�׺
  return temp+1;
 }
 return "";           //���������ļ�����û��. �򷵻ؿմ�
}
void send_file(char* arguments, int sock) {
 char* extension = file_type(arguments);    //����ļ���׺��
 char* content_type = "text/plain";     //��ʼ��type='text/plain'
 FILE* read_from;         //�����ļ�ָ��Ӹ��ļ��ж�ȡ.html .jpg��
 int readed = -1;         //ÿ�ζ��õ��ֽ���
 
 if (strcmp(extension, "html") == 0) {    //��������Ϊhtml
  content_type = "text/html";
 }
 if (strcmp(extension, "gif") == 0) {    //��������Ϊgif
  content_type = "image/gif";
 }
 if (strcmp(extension, "jpg") == 0) {    //��������Ϊjpg
  content_type = "image/jpg";
 }
  if (strcmp(extension, "png") == 0) {    //��������Ϊpng
  content_type = "image/png";
 }
 read_from = fopen(arguments, "r");     //���û�ָ�����ļ�׼����ȡ
 if(read_from != NULL) {        //ָ�벻Ϊ��
  char read_buf[128];        //���ļ�ʱ���ֽڻ�������
  send_header(sock, content_type);    //����Э��ͷ
  send(sock, "\r\n", 2, 0);      //�ټ�һ��"\r\n" ����ȱ�� ��ʽҪ��
  while(!feof(read_from)) {      //�ж��ļ��Ƿ��Ѿ�����
   fgets(read_buf, 128, read_from);   //��ȡ
   int len = strlen(read_buf);
   if (sendall(sock, read_buf, &len) == -1) { //��������
    printf("Sending error!");    //���ַ��ʹ�����ʾ������̨ ��������
    continue;
   }
  }
 }
}
void handle_req(char* request, int client_sock) {
 char command[BUFSIZ];        //����������������ֶ� GET PUT
 char arguments[BUFSIZ];        //�����������������ļ�
 strcpy(arguments, "./");       //ע��÷����ڲ�ͬ����ϵͳ������
 if (sscanf(request, "%s%s", command, arguments+2) != 2) {
  return;           //���������ڷ���
 }
 
 printf("handle_cmd:    %s\n",command);    //�����̨�����ʱ������
 printf("handle_path:   %s\n",arguments);   //�����̨�����ʱ������·��
 
 if (strcmp(command, "GET") != 0) {     //���������ʽ�Ƿ���ȷ
  bad_req(client_sock);
  return;
 }
 if (not_exit(arguments)) {       //������ļ��Ƿ����  
  file_not_found(arguments, client_sock);
  return;
 }
 send_file(arguments, client_sock);     //�����ʽ������·����ȷ��������
 
 return;
}
int make_server_socket() {
 struct sockaddr_in server_addr;       //��������ַ�ṹ��
 int tempSockId;           //��ʱ�洢socket������
 tempSockId = socket(PF_INET, SOCK_STREAM, 0);
 
 if (tempSockId == -1) {         //�������ֵΪ��1 �����
  return -1;
 }
 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons(SERVER_PORT);
 server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //���ص�ַ
 memset(&(server_addr.sin_zero), '\0', 8);
 if (bind(tempSockId, (struct sockaddr *)&server_addr,
  sizeof(server_addr)) == -1) {       //�󶨷���������� �򷵻أ�1
  printf("bind error!\n");
  return -1;
 }
 if (listen(tempSockId, BACKLOG) == -1 ) {     //��ʼ����
  printf("listen error!\n");
  return -1;
 }
 return tempSockId;           //����ȡ�õ�SOCKET
}
int main(int argc, char * argv[]) {
 WSADATA wsaData;
 if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
  fprintf(stderr, "WSAStartup failed.\n");
  exit(1);
 }
 printf("My web server started...\n");
 int server_socket;        //��������socket
 int acc_socket;         //���յ����û����ӵ�socket
 int sock_size = sizeof(struct sockaddr_in);  
 struct sockaddr_in user_socket;     //�ͻ�������Ϣ
 server_socket = make_server_socket();   //�����������˵�socket
 if (server_socket == -1) {      //����socket����
  printf("Server exception!\n");
  exit(2);
 }
 while(true) {
  acc_socket = accept(server_socket, (struct sockaddr *)&user_socket, &sock_size); //��������
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
