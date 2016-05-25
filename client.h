/****************/
/* Your Name	*/
/* Date		*/
/* CS 244B	*/
/* Spring 2013	*/
/****************/

enum {
  NormalReturn = 0,
  ErrorReturn = -1,
};

#include <stdbool.h>

/* ------------------------------------------------------------------ */

#ifdef ASSERT_DEBUG
#define ASSERT(ASSERTION) \
 { assert(ASSERTION); }
#else
#define ASSERT(ASSERTION) \
{ }
#endif

/* ------------------------------------------------------------------ */

	/********************/
	/* Client Functions */
	/********************/
#ifdef __cplusplus
extern "C" {
#endif

//extern int InitReplFs(unsigned short portNum, int packetLoss, int numServers);
//extern int OpenFile(char * strFileName);
//extern int WriteBlock(int fd, char * strData, int byteOffset, int blockSize);
//extern int Commit(int fd);
//extern int Abort(int fd);
//extern int CloseFile(int fd);

/* ------------------------------------------------------------------ */

extern int InitReplFs( unsigned short, int, int);
extern int OpenFile( char*);
extern int WriteBlock( int, char*, int, int);
extern int Commit(int);
extern int Commit_helper( int, bool);
extern void resendPacket(std::vector<data>, uint32_t[]);
extern int Abort( int);
extern void cleanServer(std::set<uint32_t>);
extern int CloseFile( int);

#ifdef __cplusplus
}
#endif



