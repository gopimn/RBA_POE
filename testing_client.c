#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[11]="34;34;34;34";
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");



    /* HANDSHAKEADO*/

    while (1){
      printf("Sending some test data %s\n", buffer);
      //bzero(buffer,256);
      //fgets(buffer,255,stdin); /* SCANF VS FGETS*/
      n = write(sockfd,buffer,strlen(buffer));
      if (n < 0) {
	error("ERROR writing to socket, server must have closed the communication");
	break;
      }
      //bzero(buffer,256);
      //n = read(sockfd,buffer,255);
      //if (n < 0) 
      //    error("ERROR reading from socket");
      // printf("%s\n",buffer);

      /*END THE COMM*/
    }
    close(sockfd);
    return 0;
}
