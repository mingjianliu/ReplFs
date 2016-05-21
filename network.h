#define MAX_FILE_NAME_SIZE 128
#define MAX_BUFFER_SIZE 512

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


typedef struct {
  unsigned char type;
  uint32_t body[256];
} ReplFsPacket;

typedef struct {
  short eventType;
  ReplFsPacket *eventDetail; /* for incoming data */
  Sockaddr eventSource;
} ReplFsEvent;


typedef union{
				init Init;
				initACK InitACK;
				open Open;
				openACK OpenACK;
				writeblock Writeblock;
				check Check;
				vote Vote;
				resendRequest ResendRequest;
				commit Commit;
				commitACK CommitACK;
				abort Abort;
				abortACK AbortACK;
				transcationRequery TranscationRequery;
				transcationResponse TranscationResponse;	
} packetInfo; 

typedef struct{
	uint32_t clientID;
} Init;

typedef struct{
	uint32_t clientID;
	uint32_t serverID;
} InitACK;

typedef struct{
	uint32_t clientID;
	uint32_t fd;
	uint32_t nameLength;
	uint8_t fileName[MAX_FILE_NAME_SIZE];
} Open;

typedef struct{
	uint32_t clientID;
	uint32_t serverID;
	uint32_t fd;
	uint32_t success;
} OpenACK;

typedef struct{
	uint32_t clientID;
	uint32_t fd;
	uint32_t transactionID;
	uint32_t writeNumber;
	uint32_t byteOffset;
	uint32_t blockSize;
	uint8_t buffer[MAX_BUFFER_SIZE];
} Writeblock;

typedef struct{
	uint32_t clientID;
	uint32_t fd;
	uint32_t transactionID;
	uint32_t writeNumber;
} Check;

typedef struct{
	uint32_t clientID;
	uint32_t serverID;
	uint32_t fd;
	uint32_t vote;
} Vote;

typedef struct{
	uint32_t clientID;
	uint32_t serverID;
	uint32_t fd;
	uint32_t writeVector;
} ResendRequest;

typedef struct{
	uint32_t clientID;
	uint32_t fd;
	uint32_t transactionID;
	uint32_t close;
} Commit;

typedef struct{
	uint32_t clientID;
	uint32_t serverID;
	uint32_t fd;
	uint32_t transactionID;
} CommitACK;

typedef struct{
	uint32_t clientID;
	uint32_t fd;
	uint32_t transactionID;
} Abort;

typedef struct{
	uint32_t clientID;
	uint32_t serverID;
	uint32_t fd;
	uint32_t transactionID;
} AbortACK;

typedef struct{
	uint32_t serverID;
	uint32_t fd;
	uint32_t transactionID;
} TranscationRequery;

typedef struct{
	uint32_t serverID;
	uint32_t fd;
	uint32_t transactionID;
	uint32_t transactionStatue;
} TranscationResponse;

void sendPacket(unsigned char type, ){

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






