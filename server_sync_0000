/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
signed long c_t(long value) {
  if (value > 8388607) {
    value = value - 16777216;
  }
  return value;
}
int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[155],x[6],y[6],z[6];  // 165 MAXIMO mandando caracteres y 5 registros.
     char fileName[64];
     time_t t = time(NULL);
     struct tm tm  = *localtime(&t);
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     FILE *fp;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     /*BEGIN OF NAMING*/
 sprintf(fileName,"TEST_A3-%d-%d-%dT%d-%d-%d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
     /*END OF NAMING*/
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
       printf("Se escribira en el archivo %s \n",fileName);
 printf("Waiting for data\n");
 while(1){
   bzero(buffer,155);
       n = read(newsockfd,buffer,155);
       if (n < 0) error("ERROR reading from socket ");
       printf("%d\n%s\n",message_counter++,buffer);
       //  fp=fopen(fileName,"a+");
       // fprintf(fp, "%s",buffer);
       //   fprintf(fp, "\n");
       //    fclose(fp);
       if (message_counter==60000){
	 printf("CAPTURA TERMINADA M Y GIGAS!\n");
	 printf("Se escribira en el archivo %s \n",fileName);
       	 n = write(newsockfd,"C",1);
       	 if (n < 0) error("ERROR writing to socket, closing conection anyway");
       	 break;
       }
       //      END OF COMM
 }
     close(newsockfd);
     close(sockfd);
     return 0; 
}


/*
que es sincronizado
20 % complejidad con 1 ms
ptp -> 
*/
