/* Instituto Tecnológico de Costa Rica
 * Administración de Tecnologías de Información
 * Lenguajes de Programacion
 * Primera Tarea
 *
 * Elaborado por:
 *  Andrea Exarahi
 *  Thomas Bertsch
 * 
 * client.c -- un socket stream en c
 * basado en el cliente de esta pagina
 * http://beej.us/guide/bgnet/output/html/multipage/clientserver.html
 */

//declaracion de librerias
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>




#define MAXDATASIZE 1000 // maximo numero de bytes q se leen a la vez

void *get_in_addr(struct sockaddr *sa)//  obtener la dirreccion ip
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr); //si usa protocolo IPv4
    }
    return &(((struct sockaddr_in6*) sa)->sin6_addr); //si usa protocolo IPv6
}

int main(int argc, char *argv[])//main del programa client
{

    int num_socket, num_bytes2;
    char bufferCadena[MAXDATASIZE];
    int *bufferCad;
    char bufferNmb[100];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char *cadenaS;
    int interruptor = 0;
    cadenaS = "Finalizar";
    FILE *arch2; 
    char buffRsv;


    //por si no digita la ip
    if (argc != 3) {
        fprintf(stderr, "Falto la ip o el puerto de enlace\n");
        exit(1);
    }
    while (strcmp(bufferCadena, cadenaS) != 0) {
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
            fprintf(stderr, "Informacion de la dirrecion ip: %s\n", gai_strerror(rv));
            return 1;
        }

        // busca una ip para conectarse
        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((num_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }

            if (connect(num_socket, p->ai_addr, p->ai_addrlen) == -1) {
                close(num_socket);
                perror("client: conectado");
                continue;
            }
            break;
        }

        if (p == NULL) {
            fprintf(stderr, "client: Fallo a conectar\n");
            return 2;
        }
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
        if (interruptor == 0) {
            printf("client: conectando a %s\n", s);
        }
        interruptor = 1;
        freeaddrinfo(servinfo); // listo!
        
        
		if((num_bytes2 = recv(num_socket, bufferNmb, 100, 0)) == -1){ //buffer nombre del archivo recibido
			perror("recv nombre");            
			}
			
			printf("client: recibe %s\n", bufferNmb);
		
		*bufferCad = 246;
		
        arch2 = fopen("tr", "wb");
		recv(num_socket, bufferCad, 100, 0); //cantidad de bytes a recv
		
		printf("client: recibe %d cantidad de bytes\n", *bufferCad);
			
            int i=0;
            
            printf("client: recibe %d cantidad de bytes\n", bufferCad);
            
            
            while(*bufferCad!=i){
				  recv(num_socket, &buffRsv, 1, 0);
				  fwrite(&buffRsv, 1, 1, arch2);
				  i++;
		  }
		  fclose(arch2);
 
    
        
        

    }
    close(num_socket);
    return 0;
}
