/****************/
/* Your Name	*/
/* Date		*/
/* CS 244B	*/
/* Spring 2014	*/
/****************/

#define DEBUG
#define MAX_RESEND 10
#define MaxBlockLength 512
#define EVENT_TIMEOUT 0
#define EVENT_INCOMING 1

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include "network.cpp"

static int initialized = 0;
static int ThePacketLoss = 0;

/* ------------------------------------------------------------------ */

struct data{
  int offset;
  char strData[MaxBlockLength];
  int blockSize;
};


//The singleton client class
class client{
    static client *client_singleton;
    uint32_t ID;
    uint32_t fd;
    uint32_t transaction;
    uint32_t currentWrite;  
    std::vector<data> writeInfo;

    client(){
      ID = random(); 
      fd = 0;
      writeInfo.clear();
      transaction = 0;
      currentWrite = 0;
      servers.clear();
    }
  public:
    std::set<uint32_t> servers;

    uint32_t get_ID(){
      return ID;
    }

    void set_ID(uint32_t id){
      ID = id;
    }

    uint32_t get_fd(){
      return fd;
    }

    void set_fd(uint32_t number){
      fd = number;
    }

    std::vector<data> readData(){
      return writeInfo;
    }

    void writeData(data write){
      writeInfo.push_back(write);
      currentWrite++;
    }

    uint32_t getSequenceNO(){
      return currentWrite;
    }

    uint32_t getTranscation(){
      return transaction;
    }

    void finish_transcation(){
      transaction++;
      currentWrite = 0;
      writeInfo.clear();
    }

    static client *instance(){
      if(!client_singleton){
        client_singleton = new client;
      }
      return client_singleton;
    }
};




int
InitReplFs( unsigned short portNum, int packetLoss, int numServers ) {
  if(initialized != 0) 
    return 0; 
#ifdef DEBUG
  printf( "InitReplFs: Port number %d, packet loss %d percent, %d servers\n", 
	  portNum, packetLoss, numServers );
#endif

  /****************************************************/
  /* Initialize network access, local state, etc.     */
  /****************************************************/
  client *client::client_singleton = 0;
  client::instance()->set_ID(uint32_t random());
  ThePacketLoss = packetLoss;
  //Send out the init packet
  packetInfo packet;
  packet.clientID = client::instance()->get_ID();
  
  netInit();
  int resend = 0;
  sendpacket(INIT, packet);

  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;

  while(resend < MAX_RESEND && client::instance()->servers.size() != numServers){
    NextEvent(&event);
    //if recevied timeout interval, resend one
    if(event.eventType==EVENT_TIMEOUT){
      ++resend;
      sendpacket(INIT, packet);
    }
    //if received initACK, save the serverID
    if(event.eventType==EVENT_INCOMING && incoming.type == INITACK){
      packetInfo info = receviePacket(incoming);
      if(info.clientID == client::instance()->get_ID){
        client::instance()->servers.insert(info.serverID);
      }
    }
  }  

  if(resend == MAX_RESEND)
    return (ErrorReturn);

  return( NormalReturn );  
}

/* ------------------------------------------------------------------ */

int
OpenFile( char * fileName ) {
  ASSERT( client::instance()->servers.size() != 0);
  ASSERT( fileName );
#ifdef DEBUG
  printf( "OpenFile: Opening File '%s'\n", fileName );
#endif

  if(client::instance()->get_fd == 0){return ErrorReturn;}
  uint32_t fd = 0;
  while(fd!=0)  fd = random();
  client::instance()->set_fd();

  packetInfo packet;
  packet.clientID = client::instance()->get_ID();
  packet.fd = client::instance()->get_fd();
  packet.nameLength = strlen(fileName);
  strncpy(packet.fileName, fileName, strlen(fileName));
  sendpacket(OPEN, packet);
  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;
  int resend = 0;
  std::set<uint32_t> server  = client::instance()->servers;
  while(resend < MAX_RESEND && server.size() != 0){
    NextEvent(&event);
    //if recevied timeout interval, resend one
    if(event.eventType==EVENT_TIMEOUT){
      ++resend;
      sendpacket(OPEN, packet);
    }
    //if received initACK, save the serverID
    if(event.eventType==EVENT_INCOMING && incoming.type == OPENACK){
      packetInfo info = receviePacket(incoming);
      if(info.clientID == client::instance()->get_ID && info.fd == fd && server.find(serverID)!=server.end() ){
        server.erase(serverID);
        if(info.success == 0) return -1;
        else 
      }
    }
  }  

  cleanServer(server); 

  return( fd );
}

/* ------------------------------------------------------------------ */

int
WriteBlock( int fd, char * buffer, int byteOffset, int blockSize ) {
  //char strError[64];
  int bytesWritten;

  ASSERT( fd >= 0 );
  ASSERT( byteOffset >= 0 );
  ASSERT( buffer );
  ASSERT( blockSize >= 0 && blockSize < MaxBlockLength );
  ASSERT(byteOffset + blockSize < MaxFileSize);
  ASSERT( client::instance()->servers.size() != 0);

#ifdef DEBUG
  printf( "WriteBlock: Writing FD=%d, Offset=%d, Length=%d\n",
	fd, byteOffset, blockSize );
#endif

  packetInfo packet;
  packet.clientID = client::instance()->get_ID();
  packet.fd = client::instance()->get_fd();
  packet.transactionID = client::instance()->getTranscation();
  packet.writeNumber = client::instance()->getSequenceNO();
  packet.byteOffset = blockSize;
  packet.blockSize = byteOffset;
  strncpy(packet.buffer, buffer, strlen(buffer));
  
  int resend = 0;
  data write;
  write.offset = byteOffset;
  write.blockSize = blockSize;
  for(int i=0; i<MaxBlockLength; i++){
    strncpy(write.strData, buffer, strlen(buffer));
  }
  client::instance()->writeData(write);
  
  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;

  while(resend < MAX_RESEND) {
    ++resend;
    sendpacket(WRITEBLOCK, packet);
  }  

  return( bytesWritten );

  //send the writeblock packet 10 times and don't wait for response



}

/* ------------------------------------------------------------------ */

int Commit(int fd){
  return Commit_helper(fd, false);
}

int
Commit_helper( int fd, boolean close) {
  ASSERT( fd >= 0 );
  ASSERT( client::instance()->servers.size() != 0);
  if( fd != client::instance()->get_fd()) return ErrorReturn;
#ifdef DEBUG
  printf( "Commit: FD=%d\n", fd );
#endif

	/****************************************************/
	/* Prepare to Commit Phase			    */
	/* - Check that all writes made it to the server(s) */
	/****************************************************/

  //TODO: If has no write, no need to check, go to abort

  //send check packet
  std::vector<data> writeinfo = client::instance()->readData();
  //get response in 3sec, if received vote=no, then call abort and return error; if received resendrequest, send all lost packet
  packetInfo packet;
  packet.clientID = client::instance()->get_ID();
  packet.fd = client::instance()->get_fd();
  packet.transactionID = client::instance()->getTranscation();
  packet.writeNumber = client::instance()->getSequenceNO();
  sendpacket(CHECK, packet);

  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;
  int resend = 0;
  std::set<uint32_t> server  = client::instance()->servers;
  bool abort = false;

  while(resend < MAX_RESEND && server.size() != 0){
    NextEvent(&event);
    //if recevied timeout interval, resend one
    if(event.eventType==EVENT_TIMEOUT){
      ++resend;
      sendpacket(CHECK, packet);
    }
    if(event.eventType==EVENT_INCOMING){
      packetInfo info = receviePacket(incoming);
      if(incoming.type == VOTE && server.find(info.serverID)!=server.end()){
        //if vote = yes, delete it in servers; if vote = no, call abort later, and delete it in server
        if(info.vote == 0)
          abort = true;
        server.erase(info.serverID);
      }

      if(incoming.type == RESENDREQ && server.find(info.serverID)!=server.end() && info.clientID == packet.clientID && info.fd == packet.fd){
        //resend all writes needed
        resendPacket(data, info.writeVector);
        resend = 0;
      }
    }

  //If any server left, regard them as dead, delete in client::instance.
  cleanServer(server); 

  if(client::instance()->servers.size()==0)
    //should close itself
    return NormalReturn;

  //If abort == True, call abort and exit with ErrorReturn
  if(abort == true){
    return (ErrorReturn);
  }
  //If abort == False, go to commit phase

	/****************/
	/* Commit Phase */
	/****************/
  resend = 0;
  packet.close = (close)? 1:0;
  server = client::instance()->servers;
  sendpacket(COMMIT, packet);
  while(resend < MAX_RESEND && server.size() != 0){
    NextEvent(&event);
    //if recevied timeout interval, resend one
    if(event.eventType==EVENT_TIMEOUT){
      ++resend;
      sendpacket(COMMIT, packet);
    }
    if(event.eventType==EVENT_INCOMING && incoming.type == COMMITACK && server.find(incoming.serverID)!=server.end()){
      packetInfo info = receviePacket(incoming);
      //If response with commitACK, client and transcation matches
      if(info.clientID == packet.clientID && info.transactionID == packet.transactionID && info.fd == packet.fd){
        server.erase(info.serverID);
      }
    }
  }

  cleanServer(server); 

  return( NormalReturn );
}

void resendPacket(std::vector<data> data, std::vector<int> writeVector){
  for(int i=0; i<4; i++){
    int compare = 1;
    for(int shift=0; shift<32; shift++){
      if(!(compare & writeVector)){
        //Send this packet by i*32 + j
        WriteBlock(client::instance()->get_fd(), data[i*32+j].strData, data[i*32+j].offset, data[i*32+j].blockSize)
      }

      compare << 1;
    }
  }
}

/* ------------------------------------------------------------------ */

int
Abort( int fd )
{
  ASSERT( fd >= 0 );
  ASSERT( client::instance()->servers.size() != 0);
#ifdef DEBUG
  printf( "Abort: FD=%d\n", fd );
#endif

  /*************************/
  /* Abort the transaction */
  /*************************/
  packetInfo packet;
  packet.clientID = client::instance()->get_ID();
  packet.fd = client::instance()->get_fd();
  packet.transactionID = client::instance()->getTranscation();
  sendpacket(ABORT, packet);

  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;
  int resend = 0;
  std::set<uint32_t> server  = client::instance()->servers;

  while(resend < MAX_RESEND && server.size() != 0){
    NextEvent(&event);
    //if recevied timeout interval, resend one
    if(event.eventType==EVENT_TIMEOUT){
      ++resend;
      sendpacket(ABORT, packet);
    }
    if(event.eventType==EVENT_INCOMING){
      packetInfo info = receviePacket(incoming);
      if(incoming.type == ABORTACK && server.find(info.serverID)!=server.end() && info.clientID==packet.clientID
         && info.fd==packet.fd && info.transactionID==packet.transactionID){
        server.erase(info.serverID);
      }

    }
  }
  
  cleanServer(server);  

  return(NormalReturn);
}

void cleanServer(std::set<uint32_t> server){
  if(server.size()!=0){
    for(uint32_t each: server){
      client::instance()->servers.earse(each);
    }
  }
}


/* ------------------------------------------------------------------ */

int
CloseFile( int fd ) {

  ASSERT( fd >= 0 );
  ASSERT( client::instance()->servers.size() != 0);
#ifdef DEBUG
  printf( "Close: FD=%d\n", fd );
#endif

	/*****************************/
	/* Check for Commit or Abort */
	/*****************************/

  if ( Commit( fd, true ) < 0 ) {
    perror("Close");
    return(ErrorReturn);
  }

  return(NormalReturn);
}

/* ------------------------------------------------------------------ */




