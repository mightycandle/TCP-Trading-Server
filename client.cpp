#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
using namespace std;

int sock;
int port = 5555;
char IP[]="127.0.0.1";
struct sockaddr_in addr;

const int MAX=1025;
int read_socket = 0, read_stdin = 0;
char buffer[MAX],buffer1[MAX],buffer2[MAX];
pthread_t thread_1,thread_2;

void *read_terminal(void *vargp){
	while(1) {
		int n = 0;
		bzero(buffer1, MAX);
		while((buffer1[n++] = getchar()) != '\n');
		read_stdin = 1;
		while(read_stdin == 1);
	}
}

void *read_server(void *vargp){
    while(1) {
		bzero(buffer2, MAX);
        recv(sock, buffer2, MAX,0);
        read_socket=1;
        while(read_socket == 1); 
    }
}

void read_from_socket() {
	while(read_socket != 1);
	bzero(buffer, MAX);
	strcpy(buffer, buffer2);
	read_socket = 0;
}

int read_from_terminal() {
	fflush(stdout);
	while(1) {
		if(read_stdin == 1){
			return 1;
		}
	}
}

int main(){
	socklen_t addrlen;

	sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		cout << "[-] Socket error" << "\n";
		exit(1);
	}
	cout << "[+] TCP Client Socket created." << "\n";
	bzero(&addr,sizeof(addr));
	
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=port;

	int connect_status = connect(sock,(struct sockaddr*)&addr,sizeof(addr));
	if(connect_status!=0){
		cout << "[-] Connection failed." << "\n";
		exit(1);
	}
	cout << "[+] Connected to server." << "\n";

	pthread_create(&thread_1, NULL, read_server, NULL);
	pthread_create(&thread_2, NULL, read_terminal, NULL);

	while(1){
		bzero(buffer,MAX);

		cout << "Enter input: ";
		int inp = read_from_terminal();

		for(int i=0;i<MAX;i++){
			buffer[i] = buffer1[i];
		}
		read_stdin = 0;

		send(sock, buffer, MAX, 0);
		
		bzero(buffer,MAX);
		read_from_socket();

		cout << buffer << "\n";
	}
	return 0;
}
