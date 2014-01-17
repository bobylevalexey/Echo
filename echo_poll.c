#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

#define N 10 // MAX client number

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
	
	//Poll array
	struct pollfd clients[N];
	for (int i = 0 ; i < N ; i++){
		clients[i].fd = -1;// all clients disabled at the begin
		clients[i].events = POLLIN;
	}

	//Buffer for data
	char buf[1024];
	
	listen (sock, N);

	printf ("Server started.\n");
	while (1){
		//check for new socket
		int new_client = accept (sock,NULL,NULL);
		if (new_client >= 0){
			int i = 0;
			while (i<N && clients[i].fd != -1)//find place for new client in clients array
				i++;
			if (i<N){
				clients[i].fd = new_client;
				printf ("Connected client %d.\n",i);
			} else { // there is no place for new client
				printf("There is no place for new client.\n");
				shutdown(new_client,2);
				close(new_client);
			}
		}

		int poll_ret = poll(clients,N,1000);
		
		if (poll_ret > 0){
			//find active client
			for (int i = 0 ; i < N ; i++){
				if (clients[i].revents & POLLIN){
					int bytes_read = recv (clients[i].fd,buf,1024,0);
					if (bytes_read==0){ // disconect client if recieved 0 bytes
						printf("Disconnected client %d.\n",i);
						close(clients[i].fd);
						clients[i].fd = -1;
					} else { //echo
						printf("Got message from client %d.\n",i);
						send (clients[i].fd,buf,bytes_read,0);
					}
					break;
				}
			}
		}
	}
	return 0;
}
