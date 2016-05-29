#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string>
#include <sstream>

#define MAX_FILE_NAME_SIZE 128
#define MAX_BUFFER_SIZE 512
#define INIT 0
#define INITACK 1
#define OPEN 2
#define OPENACK 3
#define WRITEBLOCK 4
#define CHECK 5
#define VOTE 6
#define RESENDREQ 7
#define COMMIT 8
#define COMMITACK 9
#define ABORT 10
#define ABORTACK 11
#define TRANSACTREQ 12
#define TRANSACTRES 13
#define EVENT_TIMEOUT 0
#define EVENT_INCOMING 1
#define THEGROUP 0xe0010101


#define USEC_PER_SEC 1000000
#define USEC_PER_MSEC 1000
#define HEARTBEAT_MSEC 125
#define HEARTBEAT_USEC (HEARTBEAT_MSEC * (USEC_PER_MSEC))

typedef struct sockaddr_in Sockaddr;

typedef struct {
  unsigned char type;
  uint32_t body[1024];
} ReplFsPacket;

typedef struct {
  short eventType;
  ReplFsPacket *eventDetail; /* for incoming data */
  Sockaddr eventSource;
} ReplFsEvent;


typedef struct{
	uint32_t 	clientID;
	uint32_t 	serverID;
	uint32_t 	fd;
	uint32_t 	transactionID;
	uint32_t 	transactionStatue;
	uint32_t 	writeNumber;
	uint32_t 	nameLength;
	uint8_t 	fileName[MAX_FILE_NAME_SIZE];	
	uint32_t 	byteOffset;
	uint32_t 	blockSize;
	uint8_t 	buffer[MAX_BUFFER_SIZE];
	uint32_t 	success;
	uint32_t 	vote;
	uint32_t 	close;
	uint32_t	writeVector[4];
} packetInfo; 

void sendPacket(unsigned char, packetInfo);
packetInfo receviePacket(ReplFsPacket);
void netInit();
void getHostName(char *prompt, char **, Sockaddr*);
Sockaddr *resolveHost(register char *);
void nextEvent(ReplFsEvent*);
static void incrementTimeout(struct timeval*);
static void subtractTimevals(const struct timeval* , const struct timeval* , struct timeval* );