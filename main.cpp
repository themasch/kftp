#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include "cSocket.h"

// Version
#define  VERSION  "0.0.0.4_ALPHA"

// Konstanten:
#define MAXDATASIZE  1500  						// max  number  of  bytes  we  can  get  at  once  --  bei  ethernet  sollte  dies  1500  sein..  (MTU)
#define DEBUG        0                          // wollen wir debugausgaben?

// Funktionen
void debug(std::string text)
{
    if(DEBUG) std::cout << "[dbg]" << text << std::endl;
}

void versionInfo()
{
  std::cout  <<  "Kftp  v"  <<  VERSION << " (c) 2008  by Aljoscha V., Daniel S. and Mark S.";
  std::cout  << std::endl << "----------------------------------------" << std::endl;
}

cSocket pasvmode(std::string recbuf)
{
    // entering passive mode
    int         x = recbuf.length();
    int         i, start, ipsep, ccount, port;
    std::string ipbuf, portbuf;
    for(i=0;i<x;i++)
    {
        if(recbuf.at(i) == '(')
        {
            start = i+1;
            break;
        }
    }
    recbuf.erase(0, start);
    recbuf.erase(recbuf.length()-4);
    x = recbuf.length();
    ccount = 0;
    for(i=0;i<x;i++)
    {
        if(recbuf.at(i) == ',')
        {
            ccount++;
            if(ccount == 4)
            {
                ipsep = i;
                break;
            }
        }
    }
    ipbuf = recbuf;
    ipbuf.erase(ipsep, x-1);
    portbuf = recbuf;
    portbuf.erase(0, ipsep+1);
    if(atoi(portbuf.c_str()) > 255)
    {
        std::cerr << "possible buffer overflow" << std::endl;
        exit(1);
    }
    port = atoi(portbuf.c_str())*256;
    portbuf.erase(0, 4);
    if(atoi(portbuf.c_str()) > 255)
    {
        std::cerr << "possible buffer overflow" << std::endl;
        exit(1);
    }
    port = port + atoi(portbuf.c_str());
    // port ready!
    for(i=0;i<ipbuf.length();i++)
    {
        if(ipbuf.at(i) == ',') ipbuf.at(i) = '.';
    }
    std::cout << ">> Entering PASV-Mode: " << ipbuf << ":" << port << std::endl;
    return cSocket((char *)ipbuf.c_str(), port);
}

int main(int  argc,  char  *argv[])
{
  //  Wir  kontrollieren  ob  wir  mit  2  Parametern  gerufen  wurden,  schonmal  besser  als  segmentation  fault
  unsigned int  myport;            //  unser Port unsigned int (1 - 65535)
  bool          loop;              // bool zum checken ob das prog beendet werden soll
  bool          messageloop;       // hat der server noch was zu sagen?
  bool          bInPasvMode;       // sind wir im pasvmode?
  fd_set        handles;
  std::string   recbuf;
  std::string   pasvbuf;
  std::string   in;
  std::string   cmpbuf;
  cSocket       pasvsock;
  versionInfo();
  if(argc!=3)
  {
    if(argc!=2)
    {
      // weder ip|hostname noch port
      std::cout  <<  "Please  supply  an  Hostname  or  a  IP  address,  and  a  Port  to  connect  to\n";
      std::cout  <<  "Usage:  "  <<  argv[0]  <<  "  ip/hostname  [port]\n";
      exit(1);
    }  else  {
      //  default  port
      std::cout  <<  "No  port  given,  using  default  port  21\n";
      myport  =  21;
    }
  }  else  {
    // ip|hostname und port da
    myport  =  atoi(argv[2]);
  }
  cSocket mysock = cSocket(argv[1], myport);
  std::cout  <<  "Connecting  to:  "  <<  argv[1]  <<  ":"  <<  myport  << std::endl;;
  loop  =  true;
  messageloop = false;
  bInPasvMode = false;
    try
    {
        while(1)
        {
            FD_ZERO(&handles);
            FD_SET(0, &handles);
            FD_SET(mysock.mysock, &handles);
            select(mysock.mysock+1, &handles, 0, 0, NULL);
            if(FD_ISSET(mysock.mysock, &handles))
            {
                recbuf = mysock.receiveStr();
                std::cout << recbuf;
             /*   if(!messageloop)
                {
                    switch(atoi(recbuf.c_str()))
                    {
                        case 999:
                            exit(1);
                        break;
                        case 221:
                            loop = false;
                        break;
                        case 227:
                            pasvsock = pasvmode(recbuf);
                            bInPasvMode = true;
                        break;
                        case 150:
                            std::cout << "<<" << "dirlisting: " << std::endl;
                            while(true)
                            {
                                pasvbuf = pasvsock.receiveStr();
                                std::cout << "<<" << pasvbuf;
                                if(atoi(pasvbuf.c_str()) == 999) break;
                            }
                            std::cout << "<<" << "end of dirlisting " << std::endl;
                        break;
                    }
                }*/
            }
            if(FD_ISSET(0, &handles))
            {
            getline(std::cin, in);
            in.append("\n");
            mysock.sendStr(in);
            }
        }
    }
  catch (std::out_of_range& e) {
    std::cout << "Out of range: " << e.what() << "\n";
  }
  catch (std::exception& e) {
    std::cout << "Some other exception: " << e.what() << "\n";
  }
  mysock.closeSock();
  std::cout  <<  "----------------------------------------\n";
  return  0;
}

