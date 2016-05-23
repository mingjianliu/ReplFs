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
#include "network.h"

static int theSocket;
/* Use this socket address to send packets to the multi-cast group. */
static Sockaddr groupAddr;
static int ThePacketLoss = 0;
static int Port;

void sendPacket(unsigned char packType, packetInfo packet){
	if(random()%100 < ThePacketLoss)	return;
	ReplFsPacket outPacket;
	outPacket.type = packType;
	outPacket.body[0] = htonl(packet.clientID);
	outPacket.body[1] = htonl(packet.serverID);
	outPacket.body[2] = htonl(packet.fd);
	outPacket.body[3] = htonl(packet.transactionID);
	outPacket.body[4] = htonl(packet.transactionStatue);
	outPacket.body[5] = htonl(packet.writeNumber);
	outPacket.body[6] = htonl(packet.nameLength);
	outPacket.body[7] = htonl(packet.byteOffset);
	outPacket.body[8] = htonl(packet.blockSize);
	outPacket.body[9] = htonl(packet.success);
	outPacket.body[10] = htonl(packet.vote);
	outPacket.body[11] = htonl(packet.close);
	for(int i=0; i<4; i++){
		outPacket.body[12+i] = htonl(packet.writeVector[i]);
	}
	if(packType == OPEN){
		for(int i=0; i<MAX_FILE_NAME_SIZE; i++){
			outPacket.body[16+i] = htonl((uint32_t) packet.fileName[i]);
		}	
	}
	if(packType == WRITEBLOCK){
		for(int i=0; i<MAX_BUFFER_SIZE; i++){
			outPacket.body[16+MAX_FILE_NAME_SIZE+i] = htonl((uint32_t) packet.buffer[i]);			
		}	
	}
	
	if (sendto(theSocket, &outPacket, sizeof(outPacket), 0, (struct sockaddr *)&groupAddr, sizeof(Sockaddr)) < 0)
		printf((char *)"Send packet error");	
}

packetInfo receviePacket(ReplFsPacket inPacket){
	packetInfo packet;
	packet.clientID = ntohl(inPacket.body[0]);
	packet.serverID = ntohl(inPacket.body[1]);
	packet.fd = ntohl(inPacket.body[2]);
	packet.transactionID = ntohl(inPacket.body[3]);
	packet.transactionStatue = ntohl(inPacket.body[4]);
	packet.writeNumber = ntohl(inPacket.body[5]);
	packet.nameLength = ntohl(inPacket.body[6]);
	packet.byteOffset = ntohl(inPacket.body[7]);
	packet.blockSize = ntohl(inPacket.body[8]);
	packet.success = ntohl(inPacket.body[9]);
	packet.vote = ntohl(inPacket.body[10]);
	packet.close = ntohl(inPacket.body[11]);
	for(int i=0; i<4; i++){
		packet.writeVector[i] = ntohl(inPacket.body[12+i]);
	}
	if(inPacket.type == OPEN){
		for(int i=0; i<MAX_FILE_NAME_SIZE; i++){
			packet.fileName[i] = ntohl(inPacket.body[16+i]);
		}	
	}
	if(inPacket.type == WRITEBLOCK){
		for(int i=0; i<MAX_BUFFER_SIZE; i++){
			packet.buffer[i] = ntohl(inPacket.body[16+MAX_FILE_NAME_SIZE+i]);			
		}	
	}
	return packet;
}

void netInit(){
	Sockaddr nullAddr;
	Sockaddr *thisHost;
  	char buf[128];
  	int reuse;
  	u_char ttl;
  	struct ip_mreq mreq;
	
  	gethostname(buf, sizeof(buf));
  	if ((thisHost = resolveHost(buf)) == (Sockaddr *)NULL)
  	  printf((char *)"who am I?");
  	
  	theSocket = socket(AF_INET, SOCK_DGRAM, 0);
  	if (theSocket < 0)
  	  printf((char *)"can't get socket");
	
  	/* SO_REUSEADDR allows more than one binding to the same
  	   socket - you cannot have more than one player on one
  	   machine without this */
  	reuse = 1;
  	if (setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, &reuse,
  	               sizeof(reuse)) < 0) {
  	  printf((char *)"setsockopt failed (SO_REUSEADDR)");
  	}
	
  	nullAddr.sin_family = AF_INET;
  	nullAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  	nullAddr.sin_port = Port;
  	if (bind(theSocket, (struct sockaddr *)&nullAddr, sizeof(nullAddr)) < 0)
  	  printf((char *)"netInit binding");
	
  	/* Multicast TTL:
  	   0 restricted to the same host
  	   1 restricted to the same subnet
  	   32 restricted to the same site
  	   64 restricted to the same region
  	   128 restricted to the same continent
  	   255 unrestricted
	
  	   DO NOT use a value > 32. If possible, use a value of 1 when
  	   testing.
  	*/
	
  	ttl = 1;
  	if (setsockopt(theSocket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
  	               sizeof(ttl)) < 0) {
  	  printf((char *)"setsockopt failed (IP_MULTICAST_TTL)");
  	}
	
  	/* join the multicast group */
  	mreq.imr_multiaddr.s_addr = htonl(THEGROUP);
  	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  	if (setsockopt(theSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq,
  	               sizeof(mreq)) < 0) {
  	  printf((char *)"setsockopt failed (IP_ADD_MEMBERSHIP)");
  	}
	
  	printf("netinit finished!\n");
	
  	/* Get the multi-cast address ready to use in SendData()
  	   calls. */
  	memcpy(&groupAddr, &nullAddr, sizeof(Sockaddr));
  	groupAddr.sin_addr.s_addr = htonl(THEGROUP);
}

/* get hostname and host socket */
void getHostName(char *prompt, char **hostName, Sockaddr *hostAddr) {
  char buf[128];
  Sockaddr *AddrTemp;

  buf[0] = '\0';
  for (AddrTemp = (Sockaddr *)NULL; AddrTemp == (Sockaddr *)NULL;) {
    printf("%s %s: ", prompt, "(CR for any host)");
    fgets(buf, sizeof(buf) - 1, stdin);
    if (strlen(buf) == 0)
      break;
    *hostName = (char *)malloc((unsigned)(strlen(buf) + 1));
    if (*hostName == NULL)
      printf("no mem for hostName");
    strcpy(*hostName, buf);

    /* check for valid maze name */
    AddrTemp = resolveHost(*hostName);
    if (AddrTemp == (Sockaddr *)NULL) {
      printf("Don't know host %s\n", *hostName);
      free(*hostName);
      *hostName = NULL;
    }
  }
  if ((*hostName != NULL) && (strlen(*hostName) != 0))
    bcopy((char *)AddrTemp, (char *)hostAddr, sizeof(Sockaddr));
}

Sockaddr *resolveHost(register char *name) {
  register struct hostent *fhost;
  struct in_addr fadd;
  static Sockaddr sa;

  if ((fhost = gethostbyname(name)) != NULL) {
    sa.sin_family = fhost->h_addrtype;
    sa.sin_port = 0;
    bcopy(fhost->h_addr, &sa.sin_addr, fhost->h_length);
  } else {
    fadd.s_addr = inet_addr(name);
    if (fadd.s_addr != -1) {
      sa.sin_family = AF_INET; /* grot */
      sa.sin_port = 0;
      sa.sin_addr.s_addr = fadd.s_addr;
    } else
      return (NULL);
  }
  return (&sa);
}

void NextEvent(ReplFsEvent* event){
  static bool nextTimeoutInitialized = false;
  static struct timeval nextTimeout;
  if(!nextTimeoutInitialized){
    gettimeofday(&nextTimeout,NULL);
    incrementTimeout(&nextTimeout);
    nextTimeoutInitialized = true;
  }
  struct timeval currTime;
  gettimeofday(&currTime,NULL);
  struct timeval timeTillTimeout;
  subtractTimevals(&nextTimeout,&currTime,&timeTillTimeout);
  fd_set fdmask;
  FD_ZERO(&fdmask);
  FD_SET(theSocket,&fdmask);
  if(select(theSocket+1,&fdmask,NULL,NULL,&timeTillTimeout) > 0){
  	socklen_t fromLen = sizeof(struct sockaddr);
  	recvfrom(theSocket,event->eventDetail,sizeof(ReplFsPacket),0,(struct sockaddr*)&(event->eventSource),&fromLen);
    event->eventType = EVENT_INCOMING;
  }else{
    incrementTimeout(&nextTimeout);
    event->eventType = EVENT_TIMEOUT;
    memset(&(event->eventSource),0, sizeof(event->eventSource));
    memset(event->eventDetail,0, sizeof(event->eventDetail));
  }
  return;
}

static void incrementTimeout(struct timeval* timeout){
  timeout->tv_usec += HEARTBEAT_USEC;
  if(timeout->tv_usec >= USEC_PER_SEC){
    timeout->tv_sec += timeout->tv_usec / USEC_PER_SEC;
    timeout->tv_usec %= USEC_PER_SEC;
  }
}

static void subtractTimevals(const struct timeval* one, const struct timeval* two, struct timeval* result){
  long usecs = (one->tv_sec - two->tv_sec) * USEC_PER_SEC;
  usecs += (one->tv_usec - two->tv_usec);
  result->tv_usec = usecs % USEC_PER_SEC;
  result->tv_sec = usecs / USEC_PER_SEC;
}



int main(){
	return 0;
}






