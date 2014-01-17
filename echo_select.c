#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define N 2 // MAX client number

int main(){
	//Create socket
	int sock = socket (AF_INET, SOCK_STREAM,0);
	if (sock<0){
		printf ("Can't create socket.\n");
		return 1;
	}
	
	// To non blocking mode
	fcntl(sock,F_SETFL,(fcntl(sock,F_GETFL,0))|O_NONBLOCK);
	
	//Bind socket
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(1337);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind (sock, (struct sockaddr *)&address, sizeof (address)) < 0){
		printf ("Can't bind.\n");
		return 1;
	};
	
	//Select array
	int clients_arr[N];// array of sockets. element=-1 means there is no socket
	for (int i=0;i<N;i++)
		clients_arr[i]=-1;
	fd_set clients;
	int max_sock = 1;// first argment for select

	//time for select
	struct timeval timer;
	timer.tv_sec=1;
	timer.tv_usec=0;

	//Buffer for data
	char buf[1024];
	
	listen (sock, N);

	printf ("Server started.\n");
	while (1){
		//check for new socket
		int new_client = accept (sock,NULL,NULL);
		if (new_client >= 0){
			int i=0;
			while (i<N && clients_arr[i]!=-1)// find place for new client 
				i++;
			if (i<N){ // there is place for new client
				max_sock=(max_sock>=new_client+1)?max_sock:new_client+1;
				clients_arr[i]=new_client;
				printf ("Connected client %d.\n",i);
			} else { // there is no place for new client
				printf("There is no place for new clint.\n");
				shutdown(new_client,2);
				close(new_client);
			}
		}
		
		FD_ZERO(&clients);//make read array for select
		for (int i=0;i<N;i++)
			if (clients_arr[i] != -1)
				FD_SET(clients_arr[i],&clients);
		int select_ret = select(max_sock,&clients,NULL,NULL,&timer);

		if (select_ret > 0){
			//find active client
			for (int i = 0 ; i < N ; i++){
				if (FD_ISSET(clients_arr[i],&clients) != 0){
					int bytes_read = recv (clients_arr[i],buf,1024,0);
					if (bytes_read==0){ // disconect client if recieved 0 bytes
						printf("Disconnected client %d.\n",i);
						close(clients_arr[i]);
						clients_arr[i] = -1;
					} else { //echo
						printf("Got message from client %d.\n",i);
						send (clients_arr[i],buf,bytes_read,0);
					}
				}
			}
		}
	}
	return 0;
}
