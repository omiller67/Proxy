#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

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

      //Index host
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
      *pAndQ = 0;//after2ndcolumn is now port number
      pAndQ++;//getting our path and query

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

      /*char *ptr;
      //char *
      //int request = parse_uri(pAndQ, )

      //if static
      if (!strstr(pAndQ, "cgi-bin"))
      {
        //If statement for requests ending in just '/'?

      }

      else
      {
         ptr = index(pAndQ, ’?’);
         if (ptr)
         {
           strcpy(cgiargs, ptr+1);
           *ptr = ’\0’;
         }
      }*/


  }
  Close(listenfd);
  return 0;
}

/*int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    //Static content
    if (!strstr(uri, "cgi-bin"))
    {      //line:netp:parseuri:isstatic
	     strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
	     strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1
	     strcat(filename, uri);                           //line:netp:parseuri:endconvert1
	if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
	    strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
	return 1;
    }
    else {                          //line:netp:parseuri:isdynamic
	ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	}
	else
	    strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
	strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
	strcat(filename, uri);                           //line:netp:parseuri:endconvert2
	return 0;
    }
}

//Dynamic content
else
{
  //uri = path;
  ptr = index(path, '?');
  if(ptr)
  {
    strcpy(cgiargs, ptr+1);
    *ptr = '\0';
  }


}
$end parse_uri*/


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
