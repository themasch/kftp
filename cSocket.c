#include <iostream>
#include <strings.h>
#include <error.h>
#include <netdb.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include "cSocket.h"

#define MAXDATASIZE 1500


cSocket::cSocket()
{}

cSocket::cSocket(char *ip, int port)
{
    std::cerr << "try to connect.." << std::endl;
    ////////////////////////////////////////// VARIABLES
    struct  hostent      *he;               // var to solve dns names
    struct  sockaddr_in  address;           // save the address
    int                  con;               // to check the conneciton
    ////////////////////////////////////////// CODE
    if((he = gethostbyname(ip)) == NULL)    // resolve Host informations
    {
        herror("gethostbyname");            // give error if gethostbyname faild
        exit(1);                            // ... and close kftp
    }
    cSocket::mysock = socket(PF_INET, SOCK_STREAM, 0);// generate a socket
    address.sin_port   = htons(port);        // put the port in the address struct
    address.sin_family = AF_INET;           // we use IPv4 here
    address.sin_addr   = *((struct in_addr *)he->h_addr);    // put the address in the address struct
    memset(address.sin_zero, '\0', sizeof address.sin_zero); // fill sin_zero with.. zeros O.o
    con = connect(cSocket::mysock, (struct sockaddr *)&address, sizeof address);    // connect to the socket
    if(con == -1)
    {
        std::cerr << "[err] connection to the socket faild: " << std::endl;
        exit(1);
    }
    sockopen = true;
}

std::string cSocket::receiveStr()
{
    if(!sockopen)
    {
        return "socket closed!";
    }
    ////////////////////////////////////////// VARIABLES
    char                 sockbuff[1025];    // buffer to receive text
    int                  cnt;               // how much changes are at our sockets?
    int                  i;
    fd_set               sockset;           // set of our sockets
    struct timeval       timeout;           // need for select (timeout 0)
    std::string          linebuff;          // the full line
    ////////////////////////////////////////// CODE
    linebuff.clear();                       // clear linebuff
    for(i=0;i<=1024;i++)
    {
        sockbuff[i] = '\0';
    }
    while(1)
    {
        FD_ZERO(&sockset);
        FD_SET(mysock, &sockset);
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;
        cnt = select(mysock+1, &sockset, NULL, NULL, &timeout);
        if(!FD_ISSET(cSocket::mysock, &sockset))
        {
            //no more data
            return linebuff;
        } else {
            cnt = recv(cSocket::mysock, &sockbuff, 1024, 0);   // get 1024 bytes
            if(cnt == 0)
            {
                std::cerr << "[err] other side closed connection" << std::endl;
                exit(EXIT_FAILURE);
            } else if(cnt < 0)
            {
                perror("recv");
                exit(1);
            }
            sockbuff[1024] = '\0';
            linebuff.append(sockbuff);          // append the received data to the buffer
        }
        FD_CLR(mysock, &sockset);
    }
}

void cSocket::sendStr(std::string msg)
{
    if(send(cSocket::mysock, msg.c_str(), msg.length(), 0) == -1)
    {
        perror("send");
        exit(1);
    }
    return;
}

void cSocket::closeSock()
{
    close(cSocket::mysock);
    cSocket::sockopen = false;
}
