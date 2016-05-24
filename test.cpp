#define DEBUG
#define MAX_RESEND 10
#define MaxBlockLength 512

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
#include <string.h>
#include <stdint.h>

using namespace std;

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

    int get_fd(){
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


typedef struct{
  uint32_t serverID;
  uint32_t fd;
  uint32_t transactionID;
  uint32_t transactionStatue;
} TranscationResponse;


client *client::client_singleton = 0;

void testlength(char * test){
  char result[10];
  strncpy(result, test, strlen(test));
  cout<< result <<endl;

}

void test1(int test[3]){
  cout<< test[2] << endl;
  test[2] = 1;
}

int main(){
  std::vector<int> test;
  test.resize(3);
  test[0] = 1;
  cout << test[2] << endl;
  return 0;
}