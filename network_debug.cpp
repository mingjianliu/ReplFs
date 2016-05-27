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


	//uint8_t 	buffer[MAX_BUFFER_SIZE];

int main(){
	char fileName[32] = "writeTest.txt";
	Port = 5018;
	netInit();
	ThePacketLoss = 0;
	packetInfo packet;
	packet.clientID = 11;
	packet.serverID = 12;
	packet.fd = 13;

	packet.transactionID = 14;
	packet.transactionStatue = 15;
	packet.writeNumber = 16;
	packet.byteOffset = 17;
	packet.blockSize = 5;
	packet.success = 18;
	packet.vote = 19;
	packet.close = 20;
	packet.writeVector[0] = 1;
	packet.writeVector[1] = 2;
	packet.writeVector[2] = 3;
	packet.writeVector[3] = 4;
	
	for(int i=0; i<MAX_FILE_NAME_SIZE; i++){
      	packet.fileName[i] = 0;
    }
	strncpy((char*)packet.fileName, fileName, strlen(fileName));
	packet.nameLength = strlen(fileName);
	cout<<"The length is "<<packet.nameLength<<endl;
	cout<<"The input file is "<<fileName<<endl;
	cout<<"The file name is "<<(char*)packet.fileName<<endl;
	sendPacket(OPEN,packet);
	int resend = 0;

	ReplFsEvent event;
    ReplFsPacket incoming;
  	event.eventDetail = &incoming;

	while(resend < 10){
    NextEvent(&event);
    //if recevied timeout interval, resend one
    if(event.eventType==EVENT_TIMEOUT){
      ++resend;
      sendPacket(OPEN, packet);
    }
    //if received initACK, save the serverID
    if(event.eventType==EVENT_INCOMING && incoming.type == OPEN){
      cout<<"Aha! Got the packet!"<<endl;
      packetInfo info = receviePacket(incoming);
      cout<<"The clientID is "<<info.clientID<<endl;
      cout<<"The serverID is "<<info.serverID<<endl;
      cout<<"The FD is "<<info.fd<<endl;
      cout<<"The filename is "<<(char*)info.fileName<<endl;
      cout<<"The file length is "<<info.nameLength<<endl;
	  cout<<"The transactionID is "<<info.transactionID<<endl;
	  cout<<"The transactionStatue is "<<info.transactionStatue<<endl;
	  cout<<"The writeNumber is "<<info.writeNumber<<endl;
	  cout<<"The byteOffset is "<<info.byteOffset<<endl;
	  cout<<"The blockSize is "<<info.blockSize<<endl;
	  cout<<"The success is "<<info.success<<endl;
	  cout<<"The vote is "<<info.vote<<endl;
	  cout<<"The close is "<<info.close<<endl;
	  cout<<"The writeVector 0 is "<<info.writeVector[0]<<endl;
	  cout<<"The writeVector 1 is "<<info.writeVector[1]<<endl;
	  cout<<"The writeVector 2 is "<<info.writeVector[2]<<endl;
	  cout<<"The writeVector 3 is "<<info.writeVector[3]<<endl;







      return 0;
    }
  }  
}