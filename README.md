# Multi-Client-Chat-Application

In this Project we implemented a multi client-server chat application using TCP sockets. There will be a single server and multiple clients communicating with the server.
Each  client  process  will  open  a  new connection with the server and add the client socket id to it’s fdset(). I have used select function to handle multiple client requests.


## 1 : Simple Chat with Group Functionality



### 1. Connection Establishment :
The client send the connection request to server. The server reply  client  with  appropriate  messages  if  connection  can  be  established  or not.

#### Successful :
If  the  connection  can  be  established  successfully,  then  generate  appropriate  identifiers for the  client  and  store  them  at  server.  Identifiers  includes -Unique  Id  (5  Digit  Random Number).  After  connection  is  established, client sends the  above  details  with  a  welcome message.

#### Unsuccessful :
If the number of clients connected are already reached limit(5) then no further client is allowed to connect and server should inform client that “Connection Limit Exceeded !!”.

#### Client details table:
Client details table are kept in memory at server side.


### 2. Data Transfer Phase :
Client sends a query message to server asking the details of online clients. Server send the details of all the online clients[each with unique key]. After receiving details, client transfer messages to any other client of choice by using its unique id. (Note that this is a one to one communication).

#### Message info table:
This table stores message details. NOTE:There  can  be  a  situation  when  a  client  A  gets  the  list  of  online  clients  and  before  it can  send  any  message  to  client  B,  client  B  goes  offline.  The  sender  in  this  case is notified that the client is now disconnected and that message is discarded.


### 3. Connection Termination :
In order to disconnect, the client send an EXIT message to  the  server.  The  server notify  all  other  clients with  the  details  of  client  which  is going to disconnect. Then the client process is terminated.


### 4. Broadcast : 
A client is able to send a broadcast message by typing “/broadcast ”. The message can be delivered to all clients connected to the server.

### 5. Group Formation : 
In this part, a client can able to make a group having unique id with  available  active  members  (max  10  in  a  group).  

Two  basic  paradigms  that  have  to  be implemented are:

#### a) Without  permission  of  other  members: 

The client can be able to make a simple group by typing ”/makegroup” followed by the client ids of all the members that the admin  want  to  include  and  the  members  will  be  automatically  joined  (making  a group in WhatsApp).

#### b.With permission of other members:
The client can be able to make a request of forming a simple group by typing ”/makegroupreq” followed by the client ids of all the  members  that  the  admin  want  to  include.  A  notification is  sent  to  that specific clients asking for permission to join the group. If a particular client wants to join, then it may send “/joingroup” followed by group id as the response. 
#### NOTE:
Group will be created only after receiving replies from all the clients to whom join request has been sent before by the admin(or creator) of the group. 




## Functionalities Included:

#### 1. /active :
To display all the available active clients that are connected to theserver.


#### 2. /send dest client_id  Message : 
To send message to the client corresponding to its unique id.


#### 3. /broadcast Message : 
Message should be broadcasted to all the activeclients.


#### 4. /makegroup client_id1 client_id2 ...client_idn :
A group with unique id will be made including all the mentioned clients along with the adminclient.


#### 5. /sendgroup  group_id  Message3:  
The  sender  should  be  in  the  group  to  transfer the  message  to  all  his  peers  of  that  group.  The  message  should  be  send  to all  the peers along with group info.


#### 6. /activegroups : 
To display all the groups that are currently active on server and the sender is a part of.


#### 7. /makegroupreq  client_id1  client_id2 ...client_idn :
A group having unique id should  be  made  with  currently only  the  admin  client.  The  request  message  for joining the group should  be notified to all the specified clients. Clients can respond to join that group.


#### 8. /joingroup  group_id: 
If  this  message  is  sent  by  a  client  having  the  request  for joining the group,then he will be added to group immediately.


#### 9. /declinegroup  group_id:
If this message is sent by a client having the request for joining the group, then client will not be added to the group.


#### 10. /quit :
The client will be removed from the server. This client will be removed from all the active groups.


#### 11. /activeallgroups :
To display all the groups which are active on theserver.


#### 12. /joingroup  group_id: 
If  this  message  is  sent  by  a  client  having  the  request  for joining the group, then he will be added to group immediately. Otherwise a request should  be  passed  to  the  admin  of  that  group  and  if  admin  responds  to  the  request positively then he should be joined to that group.



![rsz_screenshot_70](https://user-images.githubusercontent.com/28837542/120936627-6dd08980-c726-11eb-860b-17c074892c1b.png)



### Points to note:

#### Server Side1.
1) select() is used for the implementation.
2) Used send() and recv() system call.
3) If client B wants to send message to client A, B won’t be able to send to A’s socket directly. Instead it uses message details table at server side. Client B will send message to server and then server will pass on the message to client A.
4) Server will check in message details table. If it founds an entry of a particular client with dest_id as it’s sock_id, then send the message to the client and remove that entry.
5) Log messages on server (server’s terminal).
6) It handle cases when a client is no longer a part of a group, and similar cases.
7) If a client who initiate the group quits from the server then every group which he owns should be automatically deleted.

#### Client Side1.
Reading  is done from the standard input and writing to the server (send() system call) and reading from the server (recv() system call) will be handled by different processes (i have used fork here).
