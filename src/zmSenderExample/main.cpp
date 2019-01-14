
#include "zmServer/zmServer.h"


int main(int argc, char *argv[]){

    ZM::zmStartServer("127.0.0.1", 2144);

    ZM::zmStream strm = ZM::zmStream;

    ZM::zmSetStream();

}
