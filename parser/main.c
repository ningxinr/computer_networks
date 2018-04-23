//TCP/IP for the console application.
//
//Dev-C add libws2_32.a & libwsock32.a in Project.
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>                   //by user

WORD LPort = 6666;                     //���ض˿�LocalPort
char LIP[16]="127.0.0.1";              //����IP��ַLocalIPAddr
 
SOCKET S;                          //�׽ӿ�SOCKET
struct sockaddr_in rAddr;              //Զ�̲�����remoteAddr
struct sockaddr_in lAddr;              //���ز�����localAddr

WSADATA WD;                            //WinSock DLL��Ϣ
int r;                                 //result;
//----------------------------------------------------------------
int recvall(int s, char *buf, int *len) {
 int total = 0;           // �Ѿ������ֽ���
 int bytesleft = *len;                                   //��ʣ������ֽ�
 int n;
 while(total < *len) {
  n = recv(s, buf+total, bytesleft, 0);
  if (n == -1||*(buf+total)==13)  
    break; 
  total += n;
  bytesleft -= n;
 }
 if(n!=-1)
   *(buf+total)=0;
 *len = total;           // ����ʵ�ʽ��յ��ֽ���
 return n==-1?-1:0;          // �ɹ����ͷ���0 ʧ��-1
}
//----------------------------------------------------------------
void ShowInfo(char *info)
{    puts(info);
     exit(1);
}
//----------------------------------------------------------------
void SetSockAddr(struct sockaddr_in *A, WORD Port, char *IP)
{
     A->sin_family = AF_INET;              //TCP/IPЭ�� 
     A->sin_port = htons(Port);            //�˿�
     A->sin_addr.s_addr = inet_addr(IP);   //IP��ַ 
}
//----------------------------------------------------------------- 
int main(int argc, char *argv[])
{
    int len;
   WORD v;                                 //wVersonRequested;
//-----------------------Startup Win Socket------------------------
   v=0x0101;                               //0x0101 for v1.1, 0x0002 for v2.0
   r = WSAStartup(v, (LPWSADATA)&WD);
   if (r !=0) ShowInfo("Start_Error");
//-----------------------Create Win Socket-------------------------
   S = socket(PF_INET, SOCK_STREAM, 0);
   if (S == -1)  ShowInfo("Socket_Create_Error");
  
int l=sizeof(rAddr);
char Msg[256], pch1[80], pch2[80];
   
   SetSockAddr(&lAddr, LPort, LIP);
   r = bind(S, (struct sockaddr far *)&lAddr, sizeof(lAddr));
   if(r == -1)   ShowInfo("bind_Error");
   listen(S,5);
   S = accept(S, (struct sockaddr far *)&rAddr, &l);
   
   do{
//------------------------Receive Mess-----------------------------
          len = sizeof(Msg);
          r = recvall(S, Msg, &len);
          //r = recvfrom(SD, Msg, 80, 0, (struct sockaddr far *)&rAddr, &l);
                              //�����׽ӿڵ�������ַ��Ϣ�����rAddr��
          if(r == -1)    ShowInfo("Recieve_Error");
          if(!strcmp(Msg, "exit"))
             break;
          puts(Msg);          
          puts("Recieve ok!");
//------------------------Send Mess--------------------------------
          pch1 = strtok (msg, " "); 
          // gets the first word, words separated by space
          pch2 = strtok (NULL, " "); 
          // gets the subsequent word from msg
          if(strcmp(pch1,"GET"))
             Msg = "Bad request";
          else     
                strcmp(pch2,"/"); 
          // ��/�� for filename meant to return index.html

          r = sendto(S, Msg, strlen(Msg), 0, (struct sockaddr far *)&rAddr, l);
          if(r == -1)
             ShowInfo("Send Error");
          puts("Send ok!");          
   }while(1);
   
   closesocket(S);
   WSACleanup();
  
  system("PAUSE");	
  return 0;
}
