/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256],x,y[2],z[2],dtime[2];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     FILE *fp;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     printf("ESCUCHANDO cliente\n");
     listen(sockfd,5);     
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
       error("ERROR on accept");

     /*SE SUPONE QUE AQUI YA SE HIZO EL HANDSHAKE*/
     int message_counter=0;
     while(1){
       printf("Waiting for data\n");
       bzero(buffer,256);
      
       n = read(newsockfd,x,6);
       if (n < 0) error("ERROR reading from socket,x ");
    
       printf("Here is the message number %d:\n%s\n",message_counter++,x);
       
       /* fp=fopen("A3_recieve_turn_tcp32.txt", "a+"); */
       /* fprintf(fp, "%s\n",buffer); */
       /* fclose(fp) */;
       /* if (message_counter==2000000){ */
       /* 	 n = write(newsockfd,"C",1); */
       /* 	 if (n < 0) error("ERROR writing to socket, closing conection anyway"); */
       /* 	 break; */
       /* } */
       /*END OF COMM*/
     }
     close(newsockfd);

     
     close(sockfd);
     return 0; 
}
