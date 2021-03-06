//    File: server.cpp
//    Copyright (C) 2015  Patrik Faistaver
//    
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h> // inet_ntoa();

using namespace std;

struct TThr
 {
   pthread_t  m_Thr;
   int        m_DataFd;
 };


int openSrvSocket ( const char * name, int port )
 {
   struct addrinfo * ai;
   char portStr[10];
   
   snprintf ( portStr, sizeof ( portStr ), "%d", port );

   /* Adresa, kde server posloucha. Podle name se urci typ adresy
    * (IPv4/6) a jeji binarni podoba
    */
   if ( getaddrinfo ( name, portStr, NULL, &ai ) )
    {
      printf ( "Getaddrinfo() error...\n" );
      return -1;
    }
   /* Otevreni soketu, typ soketu (family) podle navratove hodnoty getaddrinfo,
    * stream = TCP
    */
   int fd = socket ( ai -> ai_family, SOCK_STREAM, 0 );
   if ( fd == -1 )
    {
      freeaddrinfo ( ai );
      printf ( "Socket() error...\n" );
      return -1;
    }

   /* napojeni soketu na zadane sitove rozhrani
    */
   if ( bind ( fd, ai -> ai_addr, ai -> ai_addrlen ) == -1 )
    {
      close ( fd );
      freeaddrinfo ( ai );
      printf ( "Bind() error...\n" );
      return -1;
    }
   freeaddrinfo ( ai );
   /* prepnuti soketu na rezim naslouchani (tedy tento soket nebude vyrizovat
    * datovou komunikaci, budou po nem pouze chodit pozadavky na pripojeni.
    * 10 je max velikost fronty cekajicich pozadavku na pripojeni.
    */
   if ( listen ( fd, 10 ) == -1 )
    {
      close ( fd );
      printf ( "Listen() error...\n" );
      return -1;
    }
   return fd;
 }

void getBacklightReply(int sockFD) {
    char buffer [27] = {0x01, 0x30, 0x30,
                        'A',//monitor ID
                        'D',//message type
                        '1', '2', //message length
                        0x02, //STX
                        '0', '0', // result
                        '0', '0', '1', '0', // opCodePage and opCode
                        '0', '0', //operation type code
                        '0', '0', '6', '4', //max value
                        '0', '0', '3', '2', //current value
                        0x03, // ETX
                        'X',   // check code,
                        0x0D}; // delimiter
      write ( sockFD, buffer, 27 );
}
void setBacklightReply(int sockFD) {
    char buffer [27] = {0x01, 0x30, 0x30,
                        'A',//monitor ID
                        'F',//message type
                        '1', '2', //message length
                        0x02, //STX
                        '0', '0', // result
                        '0', '0', '1', '0', // opCodePage and opCode
                        '0', '0', //operation type code
                        '0', '0', '6', '4', //max value
                        '0', '0', '5', '0', //current value
                        0x03, // ETX
                        'X',   // check code,
                        0x0D}; // delimiter
      write ( sockFD, buffer, 27 );
}
void saveCurrentSettingsReply(int sockFD) {
    char buffer [15] = {0x01, 0x30, 0x30,
                        'A',//monitor ID
                        'B',//message type
                        '0', '6', //message length
                        0x02, //STX
                        '0', '0', 
                        '0', 'C', // command code
                        0x03, // ETX
                        'X',   // check code,
                        0x0D}; // delimiter
      write ( sockFD, buffer, 15 );
}
void powerStatusReadReply (int sockFD) {
    char buffer [27] = {0x01, 0x30, 0x30,
                        'A',//monitor ID
                        'B',//message type
                        '1', '2', //message length
                        0x02, //STX
                        '0', '2', //reserved 
                        '0', '0', //result
                        'D', '6', //command code
                        '0', '0', //type
                        '0', '0', '0', '4', //total num of power modes
                        '0', '0', '0', '3', //current power mode (suspend)
                        0x03, // ETX
                        'X',   // check code,
                        0x0D}; // delimiter
      write ( sockFD, buffer, 27 );
}
void powerControlReply (int sockFD) {
    char buffer [23] = {0x01, 0x30, 0x30,
                        'A',//monitor ID
                        'B',//message type
                        '0', 'E', //message length
                        0x02, //STX
                        '0', '0', //result
                        'C', '2', '0', '3', 'D', '6',//power control reply command code
                        '0', '0', '0', '1', //current power mode (ON)
                        0x03, // ETX
                        'X',   // check code,
                        0x0D}; // delimiter
      write ( sockFD, buffer, 23 );
    
}

/* obsluha jednoho klienta (vsechny jeho zpravy)
 */
bool readNPrintData(int sockFD) {
      char buffer[200];
      int l = read ( sockFD, buffer, sizeof ( buffer ));
      // nulova delka -> uzavreni spojeni klientem
      if ( ! l ) return false;
      printf("---------------------\n");
      for ( int i = 0; i < l; i ++ ) printf("0x%02x(%c)|", buffer[i], buffer[i]);
      printf("\n");
      printf("---------------------\n");
      return true;
}
void * serveClient ( TThr * thrData )
 {
   while ( 1 )
    {
//        if (!readNPrintData(thrData->m_DataFd)) break;
//        powerControlReply(thrData->m_DataFd);
       //==============================================
//        if (!readNPrintData(thrData->m_DataFd)) break;
//        powerStatusReadReply(thrData->m_DataFd);
       //==============================================
        if (!readNPrintData(thrData->m_DataFd)) break;
        getBacklightReply(thrData->m_DataFd);
       //==============================================
//        if (!readNPrintData(thrData->m_DataFd)) break;
//        setBacklightReply(thrData->m_DataFd);
       //==============================================
//        if (!readNPrintData(thrData->m_DataFd)) break;
//        saveCurrentSettingsReply(thrData->m_DataFd);
       //==============================================

       // spojeni nebylo ukonceno, jeste mohou prijit dalsi data.
    }
//   printf("\n"); //needed to flush the output stream (ONLY FOR DEBUG)
   close ( thrData -> m_DataFd );
   printf ( "Close connection\n" );
   delete thrData;
   return NULL;
 }

int main ( int argc, char ** argv )
 {
    /*192.168.0.10 is the LCD IP*/
    /*7142 is the LCD Port*/
   int fd = openSrvSocket ( "localhost", 12345);
   if ( fd < 0 ) return 1;
   
   printf("Server v1.3\n");
   pthread_attr_t attr;
   pthread_attr_init ( &attr );
   pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_DETACHED );
   while ( 1 )
    {
    printf("Listening...\n");
      struct sockaddr remote;
      socklen_t remoteLen = sizeof ( remote );
      TThr * thrData = new TThr;
      // vyckame na prichozi spojeni
      thrData -> m_DataFd = accept ( fd, &remote, &remoteLen );
      printf ( "New connection (%s)\n", inet_ntoa(((struct sockaddr_in*)&remote)->sin_addr) );
      // pro kazdeho klienta vyrobime vlastni vlakno, ktere jej obslouzi
      pthread_create ( &thrData -> m_Thr, &attr, (void*(*)(void*)) serveClient, (void*) thrData );
    }
   pthread_attr_destroy ( &attr );
   // servery bezi stale, sem se rizeni nikdy nedostane.
   close ( fd );
   return 0;
 }
