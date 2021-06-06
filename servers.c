# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<sys/socket.h>
# include<sys/types.h>
# include<netinet/in.h>
# include<arpa/inet.h>
# include<time.h>
# include<ctype.h>
# include<sys/wait.h>
# include<sys/ipc.h>
# include<sys/sem.h>
# define PORT 7003
# define MAX 5
// structure to store group information 
struct group{
    int grpid;
    int adminid;
    int grpmem[10]; 
    int count;
    struct group *next;
};

// this function takes a client id as input and returns its file descriptor  
int filedes(int *cltid,int id){
           int fd;
           for(int j=0;j<20;j++){
                      if(cltid[j]==id){                 
                               fd=j;
                               return fd;
                      }
          }
          return -1;
}
 
// this function search for a particular client in a group
int search(struct group *t,int id){ 
      for(int i=0;i<10;i++){
               if(t->grpmem[i]==id){
                         return i;
               }
      }  
      return -1;
}

// this function inserts number of clients which is in s array into a group for which we make a node of type group and insert it in a linked list
//linked list  contains  information of all groups  
void insert(struct group ** head,int groupid,int adminid,int count,char s[10][100]){
        struct group *new=(struct group *)malloc(sizeof(struct group));
        new->grpid=groupid;
        new->adminid=adminid;
        new->count=count;
        for(int i=0;i<count;i++){
                  new->grpmem[i]=atoi(s[i]);
        }
        new->next=NULL;
        if(*head==NULL){
                  *head=new;
                  return;
        }
        else{     
                  struct group * t=*head;
                  while(t->next!=NULL){
                              t=t->next; 
                  }
                  t->next=new;
                  return;
        }
                     
}

// this function delete a particular group node from linked list of groups
void delete(struct group ** head,int admid){
      struct group* temp = *head, *prev; 
  
    // If head node itself holds the key to be deleted 
    if (temp != NULL && temp->adminid == admid) 
    { 
        *head = temp->next;   // Changed head 
        free(temp);               // free old head 
        return; 
    } 
  
    // Search for the key to be deleted, keep track of the 
    // previous node as we need to change 'prev->next' 
    while (temp != NULL && temp->adminid != admid) 
    { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    // If key was not present in linked list 
    if (temp == NULL) return; 
  
    // Unlink the node from linked list 
    prev->next = temp->next; 
  
    free(temp);
} 

// this function will print the information of all groups like its groupid,adminid, and all it members
void print(struct group * head){
         struct group * t=head;
         while(t!=NULL){
                printf("grpid-%d  adminid-%d  members - ",t->grpid,t->adminid);
                for(int i=0;i<t->count;i++){
                           printf("%d ",t->grpmem[i]);
                           
                }
                t=t->next;
         }
         printf("\n");
}

int main()
{       

        struct group *head=NULL;
        int cltid[20],nclients=0;
        for(int i=0;i<20;i++){
                cltid[i]=-1;
        }
        int semid1;
        semid1=semget(IPC_PRIVATE,1,0666|IPC_CREAT);
        struct sembuf p ={0,-1,0};
        struct sembuf v={0,+1,0};
        semctl(semid1,0,SETVAL,1);
        fd_set master;    // master file descriptor list
        fd_set read_fds; // temp file descriptor list for select()
        int fdmax;      // maximum file descriptor number
	int listener;  // listening socket descriptor
        struct sockaddr_in serverAddr;
 
	int newfd; // newly accepted socket descriptor
	struct sockaddr_in remoteAddr;

	socklen_t addr_size;
        FD_ZERO(&master);  // clear the master and temp sets
        FD_ZERO(&read_fds);
	pid_t childpid;

	listener = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	bind(listener, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	listen(listener, 5);
        FILE *fp;
        fp=fopen("messages.txt","w");
        // add the listener to the master set
        FD_SET(listener, &master);
        fdmax = listener;  // keep track of the biggest file descriptor
        socklen_t addrlen;
        printf("listening...\n");
        // main loop
        for(;;) {
                 read_fds = master;
                 if(select(fdmax+1, &read_fds,NULL,NULL,NULL) == -1) {
                               perror("select");
                               exit(4);
                 }
                 // run through the existing connections looking for data to read
                 for(int i =0; i <= fdmax; i++){
                               if(FD_ISSET(i, &read_fds)) {
                                           if(i == listener) {
                                                   // handle new connections
                                                   nclients++;
                                                   addrlen =sizeof(remoteAddr);
                                                   newfd = accept(listener,(struct sockaddr *)&remoteAddr,&addrlen);
                                                   if(newfd == -1){
                                                           perror("accept");
                                                    }
                                                    else{
                                                          if(nclients>MAX){
                                                               send(newfd,"Connection Limit Exceeded",39,0);
                                                               nclients--;
                                                               close(newfd);       
                                                          }
                                                          else{
                                                               int num = (rand() % (20000 + 1)) + 30000;
                                                               cltid[newfd]=num;
                                                               FD_SET(newfd, &master); // add to master set 
                                                               if(newfd > fdmax) {     // keep track of the max
                                                                         fdmax = newfd;
                                                               }
                                                               printf("Connected with client  %d\n",num);
                                                               char s[39];
                                                               sprintf(s,"Welcome Client %d",num);
                                                               send(newfd,s,sizeof(s),0);
                                                                 
                                                          }
                                                    }
                                           }
                                           else{
                                                
                                                // handle data from a client
                                                char buffer[1024];;
                                                int n=recv(i, buffer, 1024,0);
                                                printf("%d : %s\n",cltid[i],buffer);
                                                // in case client enters exit or type ctrl+c from keyboard
                                                if((strncmp(buffer,"/quit",5)== 0) || (n==0) ){
                                                     printf("Disconnected from Client  %d\n",cltid[i]);
                                                     send(i,"/quit\n",5,0);
                                                     int x=cltid[i];
                                                     cltid[i]=-1;
                                                     close(i);
                                                     FD_CLR(i, &master);
                                                     for(int j=0;j<20;j++){
                                                                if(cltid[j]!=-1){
                                                                         char s[30];
                                                                         sprintf(s,"Client %d gone offline\n",x);
                                                                         send(j,s,sizeof(s),0);  
                                                                } 
                                                     }
                                                     delete(&head,x);
                                                     struct group *t=head;
                                                     while(t!=NULL){
                                                           int k=search(t,cltid[i]); 
                                                           if(k!=-1){
                                                               t->grpmem[k]=-1;      
                                                            }
                                                           t=t->next; 
                                                     }
                                                }
                                                // To display all the groups that are currently active on server and the sender is a part of
                                                else     
                                                if((strncmp(buffer,"/activegroups",13) == 0)){
                                                                    int active[10],k=0;
                                                                    struct group * t=head;
                                                                    while(t!=NULL){
                                                                             if(search(t,cltid[i])!=-1 || t->adminid==cltid[i]){
                                                                                     active[k]=t->grpid;
                                                                                     k++;
                                                                             }
                                                                             t=t->next;
                                                                    } 
                                                                    char s[100]="";
                                                                    sprintf(s,"Active Groups which Client %d is a part of are  ",cltid[i]);
                                                                    for(int i=0;i<k;i++){
                                                                             char c[10];
                                                                             sprintf(c,"%d ",active[i]);
                                                                             strcat(s,c);
                                                                    }
                                                                    int fd=filedes(cltid,cltid[i]);
                                                                    send(fd,s,sizeof(s),0);
                                                }
                                                //To display all the groups which are active on the server.
                                                else
                                                if((strncmp(buffer,"/activeallgroups",16) == 0)){
                                                                    struct group * t=head;
                                                                    char s[100]="Active Groups on the server are ";
                                                                    while(t!=NULL){
                                                                             char c[100];
                                                                             sprintf(c,"\ngroupid - %d  adminid- %d   members-",t->grpid,t->adminid);
                                                                             strcat(s,c);
                                                                             for(int i=0;i<t->count;i++){
                                                                                         if(t->grpmem[i]!=-1){
                                                                                                  char f[10];
                                                                                                  sprintf(f,"%d ",t->grpmem[i]);
                                                                                                  strcat(s,f);
                                                                                         }          
                                                                             }
                                                                             
                                                                             t=t->next;
                                                                    }
                                                                    int fd=filedes(cltid,cltid[i]);
                                                                    send(fd,s,sizeof(s),0);
                                                }
                                                // To display all the available active clients that are connected to the server.
                                                else
                                                if((strncmp(buffer,"/active",7) == 0)){
                                                     char s[100]="Active Clients connected to the server are :";
                                                     for(int i=0;i<20;i++){
                                                         if(cltid[i]!=-1){
                                                                char c[10];
                                                                sprintf(c,"%d\n",cltid[i]);
                                                                strcat(s,c);    
                                                         }
                                                     }
                                                     send(i,s,sizeof(s),0); 
                                                }
                                               //The  sender  should  be  in  the  group  to  transfer the  message  to  all  his  peers  of  that
                                              //group.  The  message  should  be  send  to all  the peers along with group info.     
                                                else
                                                if((strncmp(buffer,"/sendgroup",10) == 0)){
                                                             char s[2][100]={"",""};
                                                             int k=0,j=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s[j],&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                           j++;
                                                                    }
                                                             }
                                                             
                                                             int grpid=atoi(s[0]),r=0;
                                                             struct group * t=head;
                                                             while(t!=NULL){
                                                                        if(t->grpid==grpid){
                                                                                r=1;
                                                                                break;
                                                                        }
                                                                       t=t->next;
                                                             }
                                                             if(r==1){
                                                                     if(search(t,cltid[i]) || t->adminid==cltid[i]){
                                                                           for(int j=0;j<t->count;j++){
                                                                                 int fd=filedes(cltid,t->grpmem[j]); 
                                                                                 if(fd!=i){
                                                                                      send(fd,s[1],sizeof(s[1]),0);
                                                                                 }
                                                                           }
                                                                           int adminfd=filedes(cltid,t->adminid);
                                                                           if(adminfd!=i){
                                                                                    send(adminfd,s[1],sizeof(s[1]),0);
                                                                            }
                                                                            char e[10],d[100];
                                                                            while(fgets(e,sizeof(e),fp)!=NULL){
                          
                                                                            }
                                                                         sprintf(d,"source-%d  group groupid : %d  message : %s" ,cltid[i],grpid,s[1]);
                                                                         fprintf(fp,"%s\n",d);
                                                                         fflush(fp);                    
                                                                    }
                                                                    else{
                                                                
                                                                       send(i,"You cannot send message to this group as you are not a member.\n",50,0);
                                                                     }   
                                                             }
                                                             else{
                                                                  send(i,"No group is present with this groupid.\n",40,0);
                                                              } 
                                                }
                                                //To send message to the client corresponding to its unique id.
                                                else
                                                if((strncmp(buffer,"/send",5) == 0)){
                                                             char s[2][100]={"",""};
                                                             int k=0,j=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s[j],&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                           j++;
                                                                    }
                                                             }
                                                             char d[100],e[10];
                                                             while(fgets(e,sizeof(e),fp)!=NULL){
                        
                                                             }
                                                             sprintf(d,"source-%d  destination-%s  message : %s" ,cltid[i],s[0],s[1]);
                                                             fprintf(fp,"%s\n",d);
                                                             fflush(fp);
                                                             int id=atoi(s[0]);
                                                             int fd=filedes(cltid,id);
                                                             if(fd==-1){
                                                                      send(i,"There is no client present with this id.",40,0);
                                                             }
                                                             else{
                                                                      char f[100];
                                                                      sprintf(f,"%d : %s",cltid[i],s[1]);
                                                                      send(fd,f,sizeof(f),0);
                                                             }
                                                }
                                                //A group having unique id should  be  made  with  currently only  the  admin  client.  The  request 
                                               //message  for joining the group should  be notified to all the specified clients. Clients can respond  
                                               //to join that group.
                                                else
                                                if(strncmp("/makegroupreq",buffer,13)==0){
                                                             char s[10][100]={"","","","","","","","","",""};
                                                             int k=0,j=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s[j],&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                           j++;
                                                                    }
                                                             }
                                                             int r=0;
                                                             for(int l=0;l<j;l++){
                                                                     int id=atoi(s[l]);
                                                                     int fd=filedes(cltid,id);
                                                                     if(fd==-1){
                                                                           r=1;
                                                                           char c[35];
                                                                           sprintf(c,"No client is present with id  %d",id);
                                                                           send(i,c,sizeof(c),0);
                                                                     } 
                                                             }
                                                             if(r==0){
                                                                     int num = (rand() % (20000 + 1)) + 30000;
                                                                     for(int i=0;i<j;i++){
                                                                             int id=atoi(s[i]);
                                                                             int fd=filedes(cltid,id);
                                                                             char c[35];
                                                                             sprintf(c,"Do you want to join group  %d",num);
                                                                             send(fd,c,sizeof(c),0);
                                                                      
                                                                     }
                                                                     insert(&head,num,cltid[i],0,s);
                                                             }
                                                }
                                              // A group with unique id will be made including all the mentioned clients along with the admin client.
                                                else
                                                if(strncmp("/makegroup",buffer,10)==0){
                                                             char s[10][100]={"","","","","","","","","",""};
                                                             int k=0,j=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s[j],&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                           j++;
                                                                    }
                                                             }
                                                             int r=0;
                                                             for(int l=0;l<j;l++){
                                                                     int id=atoi(s[l]);
                                                                     int fd=filedes(cltid,id);
                                                                     if(fd==-1){
                                                                           r=1;
                                                                           char c[35];
                                                                           sprintf(c,"No client is present with id  %d",id);
                                                                           send(i,c,sizeof(c),0);
                                                                     } 
                                                             }
                                                             if(r==0){
                                                                   int num = (rand() % (20000 + 1)) + 30000;
                                                                   insert(&head,num,cltid[i],j,s);
                                                             }
                                                }
                                                //Message should be broadcasted to all the active clients.
                                                else
                                                if((strncmp(buffer,"/broadcast",10) == 0)){
                                                      char s[100]={""};
                                                             int k=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s,&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                    }
                                                             }
                                                     char f[40];
                                                     sprintf(f,"%d : %s" ,cltid[i],s);
                                                     for(int j=0;j<20;j++){
                                                         if(cltid[j]!=-1 && j!=i){
                                                                  send(j,f,sizeof(s),0);  
                                                         }
                                                     }
                                                     char e[10],d[100];
                                                     while(fgets(e,sizeof(e),fp)!=NULL){
                        
                                                      }
                                                     sprintf(d,"source-%d  broadcast  message : %s" ,cltid[i],s);
                                                     fprintf(fp,"%s\n",d);
                                                     fflush(fp);
             
                                                }
                                               //  If  this  message  is  sent  by  a  client  having  the  request  for joining the group,then he 
                                              //will be added to group immediately.
                                                else
                                                if((strncmp(buffer,"/joingroup",10) == 0)){
                                                             char s[10]={""};
                                                             int k=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s,&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                    }
                                                             } 
                                                             int grpid=atoi(s);                                                            
                                                             struct group * t=head;
                                                             int adminid;
                                                             while(t!=NULL){
                                                                           if(t->grpid==grpid){
                                                                                       adminid=t->adminid;
                                                                                       t->grpmem[t->count]=cltid[i];
                                                                                       t->count=t->count+1;
                                                                                       break;
                                                                           }
                                                                           t=t->next;
                                                             }
                                                             int fd=filedes(cltid,adminid); 
                                                             char c[50];
                                                             sprintf(c,"Client %d accept your request to make group\n",cltid[i]);
                                                             send(fd,c,sizeof(c),0);
                                                                          
                                                }
                                            // if this message is sent by a client having the request for joining the group, then client will not be 
                                           //added to the group.
                                                else
                                                if((strncmp(buffer,"/declinegroup",10) == 0)){
                                                             char s[10]={""};
                                                             int k=0;
                                                             for(int i=0;i<strlen(buffer);i++){
                                                                    if(buffer[i]=='<'){
                                                                            k=1;
                                                                            continue;
                                                                    }else

                                                                    if(k==1 && buffer[i]!='>'){
                                                                          strncat(s,&buffer[i],1); 
                                                                    }else
                                                                    
                                                                    if(buffer[i]=='>'){
                                                                           k=0;
                                                                    }
                                                             } 
                                                             int grpid=atoi(s);
                                                             int adminid;
                                                             struct group * t=head;
                                                             while(t!=NULL){
                                                                           if(t->grpid==grpid){
                                                                                  adminid=t->adminid;
                                                                                  break;
                                                                                       
                                                                           }
                                                                           t=t->next;
                                                             }
                                                             int fd=filedes(cltid,adminid); 
                                                             char c[50];
                                                             sprintf(c,"Client %d decline tour request to make group\n",cltid[i]);
                                                             send(fd,c,sizeof(c),0);
                                                }
                                                else{
                                                     send(i,"INVALID REQUEST\n",16,0);
                                                }
                                           } // end of else
                                                       
                                       }
                                  }
                            }
	close(newfd);
	return 0;
}

