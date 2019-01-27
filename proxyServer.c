#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache_line
{
    char key[MAXLINE];
    char value[MAXLINE];
    int valid;
    int timestamp;
};

int cachetime = 0;

struct cache_line cache[ 10 ];

void cache_init()
{
    int i;
    for( i = 0; i < 10; ++i )
    {
        cache[ i ].valid = 0;
        strcpy(cache[ i ].key, "");
        strcpy(cache[ i ].value, "");
        int timestamp = 0;
    }
}

int cache_lookup( char searchKey[], char returnValue[])
{
    ++cachetime;
    int i;
    for( i = 0; i < 10; ++i )
    {
      if( cache[ i ].valid )
      {
        if (!!!strcmp(searchKey, cache[ i ].key))
        {
          strcpy(returnValue, cache[ i ].value);
          cache[ i ].timestamp = cachetime;
          return 1;
        }

      }
    }
    return 0;
}

void cache_update( char key[], char value[] )
{
    ++cachetime;
    if( cache_lookup( key, NULL ) )
        return;
    // We know key is NOT in cache
    int i;
    for( i = 0; i < 10; ++i )
    {
        if( !cache[ i ].valid )
        {
            strcpy(cache[ i ].key, key);
            strcpy(cache[ i ].value, value);
            cache[ i ].valid = 1;
            cache[ i ].timestamp = cachetime;
            return;
        }
    }
    // We know there are ZERO empty cache lines
    int oldest_time = cachetime;
    int oldest_indx = 0;
    for( i = 0; i < 10; ++i )
    {
        if( oldest_time > cache[ i ].timestamp )
        {
            oldest_time = cache[ i ].timestamp;
            oldest_indx = i;
        }
    }

    strcpy(cache[ oldest_indx ].key, key);
    strcpy(cache[ oldest_indx ].value, value);
    cache[ oldest_indx ].timestamp = cachetime;
}

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection = "Connection: close\r\n";
static const char *proxyConnection = "Proxy-Connection: close\r\n";

void parseToRequestLine(char rLine[MAXLINE], char *mPointer, char *path, char *hPointer);
void connectToServer(char *host, char *port, char rline[MAXLINE]);

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  char *lport = argv[1];
  int listenfd = Open_listenfd(lport);

  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  while(1){

      //Connect to client, store information for http request
      int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
      rio_t rioServer;
      Rio_readinitb(&rioServer, connfd);
      char buf[MAXLINE];
      Rio_readlineb(&rioServer, buf, MAXLINE);
      char method[MAXLINE];
      char url[MAXLINE];
      sscanf(buf,"%s %s",method,url);

      //Check cache for client request, and if so, send corresponding server response
      char returnData[MAXLINE];
      if(cache_lookup(url, returnData))
      {
        Rio_writen(connfd, returnData, strlen(returnData));
        Close(listenfd);
        return 0;
      }

      //If request not in cache, fetch response from server
      else
      {
        char *hostName = index(url,':');
        hostName += 2;
        *(hostName) = 0;
        hostName++;

        //Index port
        char *serverPort = index(hostName,':');
        *serverPort = 0;
        serverPort++;

        //Index path and query
        char *pAndQ = index(serverPort,'/');
        *pAndQ = 0;
        pAndQ++;

        printf("hostname: %s\n", hostName);
        printf("port: %s\n", serverPort);
        printf("path and query: %s\n", pAndQ);

        //Parse information recieved from client into http request
        char requestLine[MAXLINE];
        parseToRequestLine(requestLine, method, pAndQ, hostName);
        printf("%s\n", requestLine);

        //Connect to server, deliver request
        connectToServer(hostName, serverPort, requestLine);
        printf("%s\n",requestLine);

        //Send http response back to client
        Rio_writen(connfd, requestLine, strlen(requestLine));

        //Update cache
        cache_update(url, requestLine);
      }
  }
  Close(listenfd);
  return 0;
}

void parseToRequestLine(char rLine[MAXLINE], char *mPointer,
                        char *path, char *hPointer)
{
    strcpy(rLine, mPointer);
    strcat(rLine, " /");
    strcat(rLine, path);
    strcat(rLine, " HTTP/1.0\r\n");

    strcat(rLine, "Host: ");
    strcat(rLine, hPointer);
    strcat(rLine, "\r\n");

    strcat(rLine, user_agent_hdr);
    strcat(rLine, connection);
    strcat(rLine, proxyConnection);
    strcat(rLine,"\r\n");
}



void connectToServer(char *host, char *port, char rline[MAXLINE])
{
  int clientfd;
  rio_t rioClient;


  clientfd = Open_clientfd(host, port);
  Rio_readinitb(&rioClient, clientfd);

  Rio_writen(clientfd, rline, strlen(rline));
  printf("%s\n", rline);

  Rio_readn(clientfd, rline, MAXLINE);
  printf("%s\n", rline);
}
