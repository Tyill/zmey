//
// zmey Project
// Copyright (C) 2018 by Contributors <https://github.com/Tyill/zmey>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <vector>
#include <string>
#include <thread>
#include <iostream>
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

// static void
// exit_nicely(PGconn *conn)
// {
//     PQfinish(conn);
//     exit(1);
// }

using namespace std;

int main(int argc, char* argv[])
{
    // if (argc != 2)
    // {
    //   std::cerr << "Usage: async_tcp_echo_server <port>\n";
    //   return 1;
    // }

    //std::string port = "2033";

    // asio::io_context ioc;
    // tcp::resolver resolver(ioc);
    // tcp::resolver::query query("localhost", port,tcp::resolver::query::canonical_name);
    // tcp::resolver::results_type endpoints = resolver.resolve(query);
    std::string err;
    ZM_Tcp::startServer(ZM_Tcp::connectPoint("127.0.0.1:2034"), err);

    // ZM_Tcp::setErrorSendCBack([](const std::string& addr, int port, const std::string& data){

    //   std::cout << "err" << std::endl;
    // });

    map<string, string> vals{make_pair("my", "pool"), make_pair("mydd", "spool")};
    string data = ZM_Aux::serialn(vals);

    map<string, string> vals2 = ZM_Aux::deserialn(data);

    //for (;;)
    {
      std::string buf;
      buf.resize(12800);

      ZM_Tcp::sendData(ZM_Tcp::connectPoint("127.0.0.1:2033"), buf);

      // tcp::socket socket(ioc);
      // asio::connect(socket, endpoints);

      // std::vector<char> buf(12800);
      // asio::error_code error;
      
      // size_t len = socket.write_some(asio::buffer(buf), error);

      // if (error == asio::error::eof)
      //   break; // Connection closed cleanly by peer.
      // else if (error)
      //   throw asio::system_error(error); // Some other error.

      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } 

    ZM_Tcp::stopServer(); 

  return 0;
}


//   const char *conninfo;
//     PGconn     *conn;
//     PGresult   *res;
//     int         nFields;
//     int         i,
//                 j;

//     /*
//      * If the user supplies a parameter on the command line, use it as the
//      * conninfo string; otherwise default to setting dbname=postgres and using
//      * environment variables or defaults for all other connection parameters.
//      */
//     if (argc > 1)
//         conninfo = argv[1];
//     else
//         conninfo = "dbname = postgres";

//     /* Make a connection to the database */
//     conn = PQconnectdb(conninfo);

//     /* Check to see that the backend connection was successfully made */
//     if (PQstatus(conn) != CONNECTION_OK)
//     {
//         fprintf(stderr, "Connection to database failed: %s",
//                 PQerrorMessage(conn));
//         exit_nicely(conn);
//     }

//     /* Set always-secure search path, so malicious users can't take control. */
//     res = PQexec(conn,
//                  "SELECT pg_catalog.set_config('search_path', '', false)");
//     if (PQresultStatus(res) != PGRES_TUPLES_OK)
//     {
//         fprintf(stderr, "SET failed: %s", PQerrorMessage(conn));
//         PQclear(res);
//         exit_nicely(conn);
//     }

//     /*
//      * Should PQclear PGresult whenever it is no longer needed to avoid memory
//      * leaks
//      */
//     PQclear(res);

//     /*
//      * Our test case here involves using a cursor, for which we must be inside
//      * a transaction block.  We could do the whole thing with a single
//      * PQexec() of "select * from pg_database", but that's too trivial to make
//      * a good example.
//      */

//     /* Start a transaction block */
//     res = PQexec(conn, "BEGIN");
//     if (PQresultStatus(res) != PGRES_COMMAND_OK)
//     {
//         fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
//         PQclear(res);
//         exit_nicely(conn);
//     }
//     PQclear(res);

//     /*
//      * Fetch rows from pg_database, the system catalog of databases
//      */
//     res = PQexec(conn, "DECLARE myportal CURSOR FOR select * from pg_database");
//     if (PQresultStatus(res) != PGRES_COMMAND_OK)
//     {
//         fprintf(stderr, "DECLARE CURSOR failed: %s", PQerrorMessage(conn));
//         PQclear(res);
//         exit_nicely(conn);
//     }
//     PQclear(res);

//     res = PQexec(conn, "FETCH ALL in myportal");
//     if (PQresultStatus(res) != PGRES_TUPLES_OK)
//     {
//         fprintf(stderr, "FETCH ALL failed: %s", PQerrorMessage(conn));
//         PQclear(res);
//         exit_nicely(conn);
//     }

//     /* first, print out the attribute names */
//     nFields = PQnfields(res);
//     for (i = 0; i < nFields; i++)
//         printf("%-15s", PQfname(res, i));
//     printf("\n\n");

//     /* next, print out the rows */
//     for (i = 0; i < PQntuples(res); i++)
//     {
//         for (j = 0; j < nFields; j++)
//             printf("%-15s", PQgetvalue(res, i, j));
//         printf("\n");
//     }

//     PQclear(res);

//     /* close the portal ... we don't bother to check for errors ... */
//     res = PQexec(conn, "CLOSE myportal");
//     PQclear(res);

//     /* end the transaction */
//     res = PQexec(conn, "END");
//     PQclear(res);

//     /* close the connection to the database and cleanup */
//     PQfinish(conn);

//     return 0;
