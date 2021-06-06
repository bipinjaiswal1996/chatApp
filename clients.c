# include<stdio.h>
# include<stdlib.h>
# include<sys/types.h>
# include<netinet/in.h>
# include<sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
int main(){
  int sockfd,port=7003;
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  struct sockaddr_in ser;
  ser.sin_family=AF_INET;
  ser.sin_port=htons(port);
  ser.sin_addr.s_addr=INADDR_ANY;
  // initiating connection request to the server
  if(connect(sockfd,(struct sockaddr *)& ser,sizeof(ser))<0){
            perror("error connecting");
  }
  int n;
   printf("Connected to server\n");
   char s[39];
   recv(sockfd,s,sizeof(s),0);
   printf("%s\n",s);
   if(strncmp("Connection Limit Exceeded",s,21)==0){
                exit(1);
   }
  // communicating with server
  if(fork()==0){ 
           // child process will handle all the write requests from the client
           while(1){
                   char buffer[1024];
                   bzero(buffer, sizeof(buffer));
                   fgets(buffer,sizeof(buffer), stdin);
                   n=send(sockfd,buffer,sizeof(buffer),0);
                   if(strncmp("/quit",buffer,4)==0){
                            exit(1);
                   }
           }
  }
  else{
         // parent process will handle all the read requests of the client 
          while(1){
                   char buffer[256];
                   bzero(buffer, sizeof(buffer));
                   recv(sockfd,buffer,sizeof(buffer),0);
                   if(strncmp("/quit",buffer,4)==0){
                            exit(1);
                   }
                   printf("%s\n",buffer);                   
          }
        
  }
  return 0;
}
