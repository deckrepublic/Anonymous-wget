#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>       /* time */
#include "awget.h"
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 200 // max number of bytes we can get at once

Chain * derializeChain(const char*);

struct arg_struct {
    char *arg1;
    int arg2;
};
char * ssPrint(Chain *chain, char *ipaddress_picked, int port_picked)
{
	char *file_name;
	if(strlen(chain->url)>7){
		file_name = strrchr(chain->url+7,'/');
	}
	else{
		file_name = strrchr(chain->url,'/');
	}
	char *doublecheck;
	if(file_name == '\0'){
		file_name = "index.html";
	}
	else if((doublecheck = strrchr(file_name,'.'))==NULL){
		file_name = "index.html";
	}
	else{
		file_name = file_name+1;
	}
	printf("Request: %s\n", chain->url);

	list *ptr = newList();
	if(chain->num_ss != 0){
	 printf("chainlist is\n");
	 ptr = chain->content;
	 while(ptr->next != NULL){
		printf("<%s, %s>\n",ptr->IP_address, ptr->port);
		ptr = ptr->next;
	 }
	 printf("next SS is <%s, %d>\n", ipaddress_picked, port_picked);
	}else{
	 printf("chainlist is empty\n");
	 printf("issuing wget for file <%s>\n",file_name);
	}
	return file_name;
}
void * receiveMessage(void * arguments)
 {

	struct arg_struct *args = (struct arg_struct *)arguments;
    //get incoming buffer
    const char *read_buffer = args->arg1;


    Chain *oldChain = derializeChain(read_buffer);

    //time to construct the new chain to pass onto next ss
    list *ptr = oldChain->content; //iterator to pick off random ss
    list *create_ptr = newList(); //keeps track of new list to make the newest chain
    list *first = NULL; //keeps track of list to pass to chain create
	const char *ipaddresses[(oldChain->num_ss)];
	const char *ports[(oldChain->num_ss)];
	int picked_ss = 0;

	if(oldChain->num_ss > 0){
	 srand (time(NULL));
     picked_ss = (rand()%oldChain->num_ss);



     int count = 0;
     while(ptr->next != NULL){
    	if(count == picked_ss){
    		ipaddresses[count] = ptr->IP_address;
    		ports[count] = ptr->port;
    		ptr = ptr->next;
    		count++;
    	}
    	else if (first == NULL){
    		first = newList();
    		first->addressLen = ptr->addressLen;
    		first->IP_address = ptr->IP_address;
    		ipaddresses[count] = ptr->IP_address;
    		first->portLen = ptr->portLen;
    		first->port = ptr->port;
    		ports[count] = ptr->port;
    		first->next = newList();
    		create_ptr = first->next;
    		ptr = ptr->next;
    		count++;

    	}
    	else{
    		create_ptr->addressLen = ptr->addressLen;
    		  //char *cpy;
    		create_ptr->IP_address = ptr->IP_address;
    		ipaddresses[count] = ptr->IP_address;
    		    		//sprintf(cpy, "%d",);
    		create_ptr->portLen = ptr->portLen;
    		create_ptr->port = ptr->port;
    		ports[count] = ptr->port;
    		create_ptr->next = newList();
    		create_ptr = create_ptr->next;
    		ptr = ptr->next;
    		count++;
    	}

     }
	}
    if(first == NULL){
    	first = newList();
    }
    Chain *passing_chain = newChain(oldChain->url,(oldChain->num_ss)-1,first);
    if(oldChain->num_ss == 0)
    {
    	char *file_name = ssPrint(oldChain,NULL,NULL);
    	int sockfd;

    	sockfd = args->arg2;

    	char *command = "wget -qO- ";
    	extern FILE *popen();
    	const char *key[strlen(command)+strlen(oldChain->url)];
    	sprintf(key,"%s%s",command,oldChain->url);

    	FILE *file = popen(key, "r");


        if(!file){
          fprintf(stderr, "Could not open pipe for output.\n");

        }

        unsigned char buff[MAXDATASIZE];
        int count = 0;

        if (send(sockfd, file_name, MAXDATASIZE, 0) == -1)
                        	             {
                        	                     perror("send");
                        	             }

        while(fread(buff, MAXDATASIZE, 1, file)>0){
        	count++;

        	int total = 0;        // how many bytes we've sent
        	int len = MAXDATASIZE;
        	int bytesleft = len; // how many we have left to send
        	int n;
        	while(total < len) {
        	    n = send(sockfd, buff+total, bytesleft, 0);
        	    if (n == -1) { perror("send"); }
        	    total += n;
        	    bytesleft -= n;
        	}
        }
        int status = pclose(file);
        if (status == -1) {
        	fprintf(stderr, "Error with file stream");
        }
        if(count == 0){
        	fprintf(stderr, "Invalid URL for wget.\n");
        }
        printf("File received\n");
        printf("Relaying file\n");
        char *buffer = "****";
        if (send(sockfd, buffer, MAXDATASIZE+1, 0) == -1)
                	             {
                	                     perror("send");
                	             }
        close(sockfd);
    }
    else{

    	int sockfd;

    	sockfd = args->arg2;
        struct hostent *he;
        int port_number;
        char *ip_address;
        int socketfd, numbytes, ret;
        //char buf[MAXDATASIZE];
        pthread_t rThread;

        struct sockaddr_in their_addr; // connector’s address information

        if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
    		perror("socket");
    		exit(1);
        }


        ip_address = ipaddresses[picked_ss];
        port_number = atoi(ports[picked_ss]);

        struct in_addr ipv4addr;
        inet_pton(AF_INET, ip_address, &ipv4addr);
        he = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);

        their_addr.sin_family = AF_INET; // host byte order
        their_addr.sin_port = htons(port_number); // short, network byte order
        //their_addr.sin_addr.s_addr = inet_addr(ip_adress);
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(their_addr.sin_zero), 8); // zero the rest of the struct

        ssPrint(oldChain,ip_address,port_number);

        if (connect(socketfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1)
        {
    		perror("connect");
    		exit(1);
        }
        //serialize the chain
        int chainLength = chainSize(passing_chain);
        char *buffer = (char *)malloc(chainLength);


        serializeChain(passing_chain,buffer);
        //send file name
        unsigned char file_buffer[MAXDATASIZE];
        printf("waiting for file...\n");
        if (send(socketfd, buffer, chainLength, 0) == -1)
             {
                     perror("send");
             }
        ret = recvfrom(socketfd, file_buffer, MAXDATASIZE, 0, NULL, NULL);
        			    	if (ret < 0)
        			        {
        			            printf("Error receiving data!\n");
        			        }
        if (send(sockfd, file_buffer, MAXDATASIZE, 0) == -1)
        			    	{
        			    	    perror("send");
        			    	}
        int flag = 1;

        printf("Relaying file...\n");
        while(flag){
       		ret = recvfrom(socketfd, file_buffer, MAXDATASIZE, 0, NULL, NULL);
       		    	if (ret < 0)
       		        {
       		            printf("Error receiving data!\n");
       		        }
       		    	else if(file_buffer[0] == '*' && file_buffer[1] == '*' && file_buffer[2] == '*' && file_buffer[3] == '*'){

       		    		flag = 0;
       		    		printf("Goodbye!\n");
       		    		if (send(sockfd, file_buffer, MAXDATASIZE, 0) == -1)
       		    		       		    		{
       		    		       		    			perror("send");
       		    		       		    		}
       		    	}
       		    	else{
       		    		if (send(sockfd, file_buffer, MAXDATASIZE, 0) == -1)
       		    		{
       		    			perror("send");
       		    		}
       		    	}
        }

        close(socketfd);
        close(sockfd);

    }
    pthread_exit(NULL);
    return NULL;
 }

void sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}
int main(int argc, char *argv[])
{
//set up connection
 int sockfd, new_fd,ret, numbytes; // listen on sock_fd, new connection on new_fd
 struct sockaddr_in my_addr; // my address information
 struct sockaddr_in their_addr; // connector’s address information
 pthread_t rThread;
 int sin_size;
 struct sigaction sa;
 int yes=1;

 if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 {
    perror("socket");
    exit(1);
 }

 if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
 {
    perror("setsockopt");
    exit(1);
 }

 char szHostName[255];

 gethostname(szHostName, 255);
 struct hostent *host_entry;
 host_entry=gethostbyname(szHostName);
 char * szLocalIP;
 szLocalIP = inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list);

int port_number = 0;
if(argc > 1){
if(strcmp(argv[1], "-p") == 0){
 port_number = atoi(argv[2]);
}
}

 my_addr.sin_family = AF_INET; // host byte order
 my_addr.sin_port = htons(port_number); // short, network byte order
 my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP

 bzero(&(my_addr.sin_zero), 8); // zero the rest of the struct
 if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1)
 {
    perror("bind");
    exit(1);
 }
 


 if (listen(sockfd, BACKLOG) == -1)
 {
    perror("listen");
    exit(1);
 }
//print address and port listining on
struct sockaddr_in sin;
socklen_t len = sizeof(sin);
if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname");
else
 printf("awaiting connection on IP: %s port: %d\n",szLocalIP,ntohs(sin.sin_port));

 sa.sa_handler = sigchld_handler; // reap all dead processes
 sigemptyset(&sa.sa_mask);
 sa.sa_flags = SA_RESTART;

 if (sigaction(SIGCHLD, &sa, NULL) == -1)
 {
    perror("sigaction");
    exit(1);
 }
    while(1) { // main accept() loop
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1)
		{
			perror("accept");
			continue;
		}
		else{
			printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));

		}
		//into listening mode
		int ret;
		char buffer[MAXDATASIZE+1];

		memset(buffer, 0, MAXDATASIZE);
		ret = recvfrom(new_fd, buffer, MAXDATASIZE+1, 0, NULL, NULL);
		    	if (ret < 0)
		        {
		            printf("Error receiving data!\n");
		        }


		//create passing buffer for the thread
		char *passing_buffer = buffer;

		struct arg_struct args;
		args.arg1 = passing_buffer;
		args.arg2 = new_fd;

		ret = pthread_create(&rThread, NULL, receiveMessage, (void *)&args);
                if (ret)
                 {
                    printf("ERROR: Return Code from pthread_create() is %d\n", ret);
                    exit(1);  
                 }

        int flag = 1;
        /*while(flag){

        }
*/
	    }


         //pthread_exit(NULL);
 	 	close(new_fd); // parent doesn’t need this
        close(sockfd);
        return 0;
    }
 

