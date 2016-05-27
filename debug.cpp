#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <sstream>
#include <stdbool.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <cmath>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <functional>
#include <sstream>
#include <cstring>
#include "network.cpp"
using namespace std;

//Define data structure of clients
static std::string path;
static uint32_t serverId;

#define EVENT_TIMEOUT 0
#define EVENT_INCOMING 1
#define MaxBlockLength 512
#define DEFAULT_PORT 44032
#define MAX_FILE_NAME_SIZE 128

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
      seqno = max(sequenceNO, seqno);
      cout<<sequenceNO<<endl;
      cout<<max(sequenceNO, seqno)<<endl;
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

int main(){
  //test singleton in client
  //test NextEvent
  //test netinit
  //test receive/send packet

  ReplFsEvent event;
  ReplFsPacket incoming;
  event.eventDetail = &incoming;
  for(int i=0; i<10; i++){
    NextEvent(&event);
    if(event.eventType ==EVENT_TIMEOUT){
      cout<<"TIMEOUT: "<<i<<endl;
    }
  }
}


//Passed 
void testFD(){
	client testclient;
	uint32_t fd = 20;
	char fileName[10] = "testfile";
	testclient.set_fd(fd, (uint8_t*) fileName);
	std::string filePath;
	filePath.assign((char*)testclient.fileName, (size_t)testclient.nameLength);
	cout<<"fd: ";
	cout<<testclient.get_fd()<<endl;
	cout<<"filename: ";
	cout<<filePath<<endl;
	cout<<"namelength: ";
	cout<<testclient.nameLength<<endl;
}

//Passed
void testIO(){
  client a = *new client();
  data b;
  b.offset = 11;
  b.blockSize = 6;
  strncpy(b.strData, "choose", MaxBlockLength);
  a.writeData(10, b);
  vector<data> d = a.getWriteInfo();
  data c = d[10];
  a.close();
  cout<<a.get_fd()<<endl;
  cout<<a.getTranscation()<<endl;
  cout<<a.getSeqNO()<<endl;
}



