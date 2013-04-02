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

#define BACKLOG 10	 // cuantas conexiones dejar en cola
#define MAXDATASIZE 1000 // maximo numero de bytes q se leen a la vez


/*Espera a todos los procesos de muertos.
* Utiliza una llamada sin bloqueo para asegurarse de que este gestor de señales no se
* bloquee si el hijo fue limpiado en otra parte del programa. */

void sigchld_handler(int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0);//Espera a que el proceso cambie de estado
}

void *get_in_addr(struct sockaddr *sa)//  obtener la dirreccion ip
{
    if (sa->sa_family == AF_INET) {//Dice si usa protocolo IPv4
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*) sa)->sin6_addr); // o si usa protocolo IPv6
}


int client(int argc, char *argv[])//la funcion client
{

    int num_socket, num_bytes;
    char bufferCadena[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char *cadena;
    int interruptor = 0;
    cadena = "Finalizar";
    FILE *arch2; 


    //por si no digita la ip
    if (argc != 3) {
        fprintf(stderr, "Falto la ip o el puerto de enlace\n");
        exit(1);
    }
    while (strcmp(bufferCadena, cadena) != 0) {
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

        if ((num_bytes = recv(num_socket, bufferCadena, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
		
        bufferCadena[num_bytes] = '\0';
        printf("client: recibe %s\n", bufferCadena);
        
        arch2 = fopen("patito.gif", "wb");
        fwrite(bufferCadena, 1, 1, arch2);
        fclose(arch2);
        

    }
    close(num_socket);
    return 0;
}

int server(int argc, char *argv[])// la funcion Server
{
    int num_socket, new_fd; // sock_fd, nueva coneccion new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // conector que almacena informacion de la dirrecion
    socklen_t sin_size; //informacion del tamaño del socket
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int interruptor = 0;
    FILE *arch; 


    if (argc != 2) {
        fprintf(stderr, "Falto puerto de enlace \n");
        exit(1);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // uso la ip de la maquina

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "Informacion de la dirrecion ip: %s\n", gai_strerror(rv));
        return 1;
    }

    // busca una ip para conectarse
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((num_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {// creacion del socket
            perror("server: socket");
            continue;
        }

        if (setsockopt(num_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
		//se asisgnan valores a la estructura my_addr
        if (bind(num_socket, p->ai_addr, p->ai_addrlen) == -1) {//buscando el cliente
            close(num_socket);
            perror("server: bind");
            continue;
        }

        break;
    }
	//ERROR si falla la la busqueda
    if (p == NULL) {
        fprintf(stderr, "server: fallo la busqueda\n");
        return 2;
    }

    freeaddrinfo(servinfo); // informacion de la conexion
    
	//Se habilita el socket para poder recibir conexiones
    if (listen(num_socket, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // obtener todos los procesos muertos
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {//funcion que controla la señal del
        //socket para su correcto funcionamiento
        perror("sigaction");
        exit(1);
    }

    printf("server:esperando para conectarse...\n");

    while (1) { // esperando a que acepte la conexion
        sin_size = sizeof their_addr;
        //se llama el accept() y el servidor queda en espera de conexiones
        new_fd = accept(num_socket, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        if (interruptor == 0) {
            printf("server: conexion recibida de: %s\n", s);
            interruptor = 1;
        }
        if (!fork()) { // esto es un hijo de proceso
            close(num_socket); // los hijos no necesita escuchar el socket
            char cadena;
            printf("Digite el nombre del archivo a enviar: ");
            scanf("%s",&cadena);
            char *archivo= &cadena;
            arch = fopen(archivo, "rb");
            
            if (arch==NULL) perror ("Error al abrir el archivo");
            
            if (send(new_fd, arch, 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
            
            
        }
        close(new_fd); // el proceso padre no necesita esto por eso lo cerramos
    }

    return 0;
}



int main(int argc, char *argv[]){
	int padre;
	padre=fork();
	
	if(padre!=0){
		sigchld_handler(2);
		server(argc, argv);
	}
	else{
		client(argc, argv);
	}
	return 0;
}


