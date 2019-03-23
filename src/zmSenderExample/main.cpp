
#include "zmServer/zmServer.h"


int main(int argc, char *argv[]){

    try {
            asio::io_context io_context;
            udp_server server(io_context);
            io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
