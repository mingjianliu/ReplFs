#include <stdbool.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <limits.h>
#include <algorithm>
#include "network.cpp"

//Define data structure of clients
static std::string path;
static uint32_t serverId;

#define EVENT_TIMEOUT 0
#define EVENT_INCOMING 1
#define MaxBlockLength 512
#define DEFAULT_PORT 5018
#define MAX_FILE_NAME_SIZE 128
#define MAX_WAIT_TIME 30

struct data{
  int offset;
  char strData[MaxBlockLength];
  int blockSize;
};

class client{
    uint32_t fd;
    uint32_t transaction;  
    std::vector<data> writeInfo;
    uint32_t writeVector[4];
    uint32_t seqno;

  public:
  	uint8_t fileName[MAX_FILE_NAME_SIZE];
  	int nameLength;

    client(){
      fd = 0;
      writeInfo.clear();
      writeInfo.resize(128);
      transaction = 0;
      for(int i=0; i<4; i++){
      	writeVector[i] = 0;
      }
      nameLength = 0;
      for(int i=0; i<MAX_FILE_NAME_SIZE; i++){
      	fileName[i] = 0;
      }
    }

    uint32_t get_fd(){
      return fd;
    }

    void set_fd(uint32_t number, uint8_t name[MAX_FILE_NAME_SIZE]){
      fd = number;
      strncpy((char*)fileName, (char*)name, strlen((char*)name));
      nameLength = strlen((char*)name);
    }

    void setSeqNO(uint32_t number){
    	seqno = number;
    }

    uint32_t getSeqNO(){
    	return seqno;
    }

    uint32_t* readData(){
      return writeVector;
    }

    void writeData(uint32_t sequenceNO, data write){
      writeInfo[sequenceNO] = write;
      writeVector[sequenceNO/32] |= 1<<(sequenceNO%32); 
      seqno = std::max(sequenceNO, seqno);
    }

    std::vector<data> getWriteInfo(){
    	return writeInfo;
    }

    uint32_t getTranscation(){
      return transaction;
    }

    void finish_transcation(){
      transaction++;
      seqno = 0;
      writeInfo.clear();
      writeInfo.resize(128);
      for(int i=0; i<4; i++){
      	writeVector[i] = 0;
      }
    }

    void close(){
      fd = 0;
      transaction = 0;
      seqno = 0;
      writeInfo.clear();
      writeInfo.resize(128);
      for(int i=0; i<4; i++){
      	writeVector[i] = 0;
      }
      for(int i=0; i<MAX_FILE_NAME_SIZE; i++){
      	fileName[i] = 0;
      }
      nameLength = 0;
    }
};

static std::map<uint32_t,client> clients;

void handleInit(packetInfo packet);
void handleOpen(packetInfo packet);
void handleWriteBlock(packetInfo packet);
void handleCheck(packetInfo packet);
bool checkAllReceived(uint32_t* writeVector, uint32_t writeNumber);
void handleCommit(packetInfo packet);
void handleAbort(packetInfo packet);

int main(const int argc, char* argv[]){
  //Generate serverID
  serverId = random();
  clients.clear();

  //Get input arguments
  unsigned short portNum;
  int dropPercent;
  if(argc == 1){
    portNum = DEFAULT_PORT;
    dropPercent = 10;
    path = "./";
  }else if(argc >= 7){
    portNum = atoi(argv[2]);
    dropPercent = atoi(argv[6]);
    path = argv[4];
    if(path[path.length()-1] != '/'){
      path +='/';
    }
    int ret = mkdir(path.c_str(),0777);
    if(ret == -1){
      printf("machine already in use\n");
      return -1;
    }
    //For test
    if(argc >=8){
    	serverId = atoi(argv[7]);
    }
  }
  ThePacketLoss = dropPercent;
  Port = portNum;

  //Start network and receive packets
  netInit();
  printf("Server started!\n");
  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;
  while(true){
    NextEvent(&event);
    if(event.eventType == EVENT_INCOMING){
      packetInfo packet = receviePacket(incoming);
      switch(incoming.type){
        case INIT:
          handleInit(packet);
          break;
        case OPEN:
          handleOpen(packet);
          break;
        case WRITEBLOCK:
          handleWriteBlock(packet);
          break;
        case CHECK:
          handleCheck(packet);
          break;
        case COMMIT:
          handleCommit(packet);
          break;
        case ABORT:
          handleAbort(packet);
          break;
  	  }  
    }
  }
  return 0;
}

//All helpers of different packets
void handleInit(packetInfo packet){

	if(clients.find(packet.clientID) == clients.end()){
		//Add client
		clients.insert(std::pair<uint32_t,client>(packet.clientID, *new client()));
		//Respond with INITACK	
		packetInfo outPacket;
		outPacket.clientID = packet.clientID;
		outPacket.serverID = serverId;
		for(int i=0; i<30; i++){
			sendPacket(INITACK, outPacket);
		}
	}
}

void handleOpen(packetInfo packet){

	packetInfo outPacket;
	outPacket.clientID = packet.clientID;
	outPacket.serverID = serverId;
	outPacket.fd = packet.fd;
	outPacket.success = 1;
	std::map<uint32_t,client>::iterator iter = clients.find(packet.clientID);

	//Check whether the client exists & one file is already opened for that client
	if(iter == clients.end()){
		outPacket.success = 0;
	}else{

	  if(iter->second.get_fd() != 0){
		  outPacket.success = 0;
	  }
	  else{
	  	for(std::map<uint32_t,client>::iterator it=clients.begin(); it!=clients.end(); it++){
		  if(it->second.get_fd()!=0 && strcmp((char*)it->second.fileName, (char*)packet.fileName) == 0){
			outPacket.success = 0;
			break;
		  }
		}
	  }
	}
	//if no one match, create one and return success = true
	if(outPacket.success == 1){
		//Update in clients
		iter->second.set_fd(packet.fd, packet.fileName);
		iter->second.nameLength = packet.nameLength;
	}

	for(int i=0; i<30; i++){
		sendPacket(OPENACK, outPacket);
	}
}

void handleWriteBlock(packetInfo packet){

  //printf( "Handle writeblock\n");

	std::map<uint32_t,client>::iterator iter = clients.find(packet.clientID);
	//Save write information
	if(iter == clients.end()){
		return;
	}else if(packet.transactionID == iter->second.getTranscation() && packet.fd == iter->second.get_fd()){
		data writeInfo;
		writeInfo.offset = packet.byteOffset;
		strncpy(writeInfo.strData, (char*)packet.buffer, MaxBlockLength);
		writeInfo.blockSize = packet.blockSize;
		iter->second.writeData(packet.writeNumber, writeInfo);
	}
	return;
}

void handleCheck(packetInfo packet){

	//Check if all writes are there
	bool ready = true;
	std::map<uint32_t,client>::iterator iter = clients.find(packet.clientID);
	if(iter == clients.end()){
    return;
  }	
	if(iter->second.get_fd() != packet.fd){
    return;
  }
	if(iter->second.getTranscation() != packet.transactionID)
  	return;

	uint32_t* writeVector = iter->second.readData();
	packetInfo outPacket;
	outPacket.clientID = packet.clientID;
	outPacket.serverID = serverId;
	outPacket.fd = packet.fd;
	//If some writes absent, send ResendRequest, and wait for writeblock
	if(!checkAllReceived(writeVector, packet.writeNumber)){
		//Send resendrequest
		for(int i=0; i<4; i++){
			outPacket.writeVector[i] = *(writeVector+i);	
		}
		ReplFsEvent event;
    ReplFsPacket incoming;
    event.eventDetail = &incoming;
    int resend = 0;
		while(resend < MAX_WAIT_TIME && !checkAllReceived(writeVector, packet.writeNumber)){
  			NextEvent(&event);
  			  //if recevied timeout interval, resend one
  			if(event.eventType==EVENT_TIMEOUT){
  			  	sendPacket(RESENDREQ, outPacket);
            resend++;
  			}
  			//if received initACK, save the serverID
  			if(event.eventType==EVENT_INCOMING && incoming.type == WRITEBLOCK){
  			    packetInfo info = receviePacket(incoming);
  			    if(packet.clientID == info.clientID && iter->second.getTranscation() == info.transactionID && packet.fd == info.fd){
  			    	handleWriteBlock(info);
  			    }
  			}
		}
    if(resend == MAX_WAIT_TIME){
      //Do something to verify if any one received commit, if not, just do iter->second.finish_transcation();
      //Send to request for statue, if no one response, iter->second.finish_transcation();
    }
	}

	//All writeblocks are arrived. Send Vote
	iter->second.setSeqNO(packet.writeNumber);
	outPacket.vote = true;
	sendPacket(VOTE, outPacket);
}

bool checkAllReceived(uint32_t* writeVector, uint32_t writeNumber){
	for(uint32_t i=0; i<writeNumber; i++){
		if(*(writeVector+i/32) & 1<<(i/32) == 0)
			return false;
	}
	return true;
}

void handleCommit(packetInfo packet){

	//get file name
	std::map<uint32_t,client>::iterator iter = clients.find(packet.clientID);
	if(iter == clients.end())	return;
	if(iter->second.get_fd() != packet.fd)	return;
	if(iter->second.getTranscation() != packet.transactionID)	return;
	std::string filePath;
	filePath.assign((char*)iter->second.fileName, (size_t)iter->second.nameLength);
	filePath = path + filePath;
 	int fd = open(filePath.c_str(),O_WRONLY | O_CREAT, 0777);
 	std::vector<data> writeInfo = iter->second.getWriteInfo();
 	for(uint32_t i=0; i<iter->second.getSeqNO(); i++){
 		lseek(fd, writeInfo[i].offset, SEEK_SET);
 		write(fd, writeInfo[i].strData, writeInfo[i].blockSize);
 	}
 	close(fd);
	//If close == true, close this file locally and do some clean up stuff
 	iter->second.finish_transcation();
 	if(packet.close == 1){
 		iter->second.close();
 	}

  packetInfo outPacket;
  outPacket.clientID = packet.clientID;
  outPacket.serverID = serverId;
  outPacket.fd = packet.fd;
  outPacket.transactionID = packet.transactionID;
  sendPacket(COMMITACK, outPacket);
}

void handleAbort(packetInfo packet){

	//Advance transcationID and abort all writes
	std::map<uint32_t,client>::iterator iter = clients.find(packet.clientID);
	if(iter == clients.end())	return;
	if(iter->second.get_fd() != packet.fd)	return;
	if(iter->second.getTranscation() != packet.transactionID)	return;
	iter->second.finish_transcation();

  packetInfo outPacket;
  outPacket.clientID = packet.clientID;
  outPacket.serverID = serverId;
  outPacket.fd = packet.fd;
  outPacket.fd = packet.transactionID;
  sendPacket(ABORTACK, outPacket);
}



