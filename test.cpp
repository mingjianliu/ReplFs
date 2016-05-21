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

client *client::client_singleton = 0;

int main(){

  static int serverID[16];
  client::instance()->set_fd(10);
  std::cout << client::instance()->get_fd() << std::endl;
  std::cout << client::instance()->servers.size() <<std::endl;
  client::instance()->servers.insert(1);
  client::instance()->servers.insert(2);
  client::instance()->servers.insert(3);
  client::instance()->servers.insert(1);
  std::cout << client::instance()->servers.size() <<std::endl;

  return 0;
}