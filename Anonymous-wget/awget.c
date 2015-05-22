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
#define MAXDATASIZE 256

//nice print out on what awget is doing
void awgetPrint(Chain *chain, char *ipaddress_picked, int port_picked)
{
	printf("Request: %s\n", chain->url);
	printf("chainlist is\n");
	list *ptr = newList();
	ptr = chain->content;
	while(ptr->next != NULL){
		printf("<%s, %s>\n",ptr->IP_address, ptr->port);
		ptr = ptr->next;
	}
	printf("<%s, %d>\n", ipaddress_picked, port_picked);
	printf("next SS is <%s, %d>\n", ipaddress_picked, port_picked);

}
//reader
int main(int argc, char *argv[])
{
//setup URL and chainfile and beginnings of the struct
    char *url;
    char *chainfile;
    FILE *cfile_in;
    int number_ss = 0;
    list *first = newList();
    
// check to see how many arguments
    if (argc == 1)
    {
        fputs("Need at least a URL argument for awget!\n",stderr);
        exit(1);
    }
    else if (argc == 2)
    {
        url = argv[1];
        chainfile = "chaingang.txt";
    }
    else if (argc == 4)
    {
    	if(strcmp(argv[2], "-c") == 0){
    		url = argv[1];
    		chainfile = argv[3];
    	}
    	else if(strcmp(argv[1], "-c") == 0){
    		url = argv[3];
    		chainfile = argv[2];
    	}
    }
    else{
    	fputs("Command usage!\n", stderr);
    }
//try to open chainfile
    cfile_in = fopen(chainfile, "r");
//if no file exist print error and exit
    if (cfile_in==NULL)
    {
        fputs("No chain file available!\n", stderr);
        exit(1);
    }
//proceed to set up values
    fscanf(cfile_in,"%d", &number_ss);
//read in file store ipaddresses and ports
    const char *ipaddresses[number_ss];
    const char *ports[number_ss];
    int count = 0;
    //portion memory for an array of strings
    while(count < number_ss)
    {
        ipaddresses[count] = malloc(20 *sizeof(char));
        ports[count] = malloc(20 *sizeof(char));
        count++;
    }
    count = 0;
    while ( count < number_ss)
        {
        	fscanf(cfile_in,"%s %s",ipaddresses[count], ports[count]);
        	count++;
        }
    fclose(cfile_in);
    count = 0;
    list *ptr = newList();

    //set random address to connect to
    srand (time(NULL));
    int picked_ss = (rand()%number_ss);
    //debug
    //picked_ss = 0;

    //now read in values and set up the structs
    int picked_ss_flag = picked_ss;
    while ( count < number_ss)
    {
    	//fscanf(cfile_in,"%s %d",ipaddresses[count], &ports[count]);
        if(count == picked_ss_flag)
    	{
    		count++; //ignore picked server to pass on to ss this will reduce the list
    	}
    	else if(picked_ss_flag == 0)
    	{
    		first->addressLen = strlen(ipaddresses[count]);
    		  //char *cpy;
    		first->IP_address = ipaddresses[count];
    		    		//sprintf(cpy, "%d",);
    		first->portLen = strlen(ports[count]);
    		first->port = ports[count];
    		first->next = newList();
    		ptr = first->next;
    		count++;
    		picked_ss_flag = number_ss + 1;
    	}
    	else if(count == 0)
    	 {
    	    		first->addressLen = strlen(ipaddresses[count]);
    	    		  //char *cpy;
    	    		first->IP_address = ipaddresses[count];
    	    		    		//sprintf(cpy, "%d",);
    	    		first->portLen = strlen(ports[count]);
    	    		first->port = ports[count];
    	    		first->next = newList();
    	    		ptr = first->next;
    	    		count++;
    	 }
    	else
    	{
    		ptr->addressLen = strlen(ipaddresses[count]);
    		//char *cpy;
    		ptr->IP_address = ipaddresses[count];
    		//sprintf(ptr->port, "%d",ports[count]);
    		ptr->portLen = strlen(ports[count]);
    		ptr->port = ports[count];
    		ptr->next = newList();
    		ptr = ptr->next;
    		count++;
    	}
        //printf("%s %d\n",ipaddresses[count], ports[count]);

    }

    //now we can create the Chain struct!
    Chain *chaingang = newChain(url,number_ss-1,first);
    
    //set up socket connection
    struct hostent *he;
    int port_number;
    char *ip_address;
    int sockfd, numbytes, ret;
    //char buf[MAXDATASIZE];
    pthread_t rThread;
    
    struct sockaddr_in their_addr; // connector’s address information
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
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
    
    if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1)
    {
		perror("connect");
		exit(1);
    }
    //serialize the chain
    int chainLength = chainSize(chaingang);
    char *buffer = (char *)malloc(chainLength);

    awgetPrint(chaingang,ip_address,port_number);
    serializeChain(chaingang,buffer);

    if (send(sockfd, buffer, chainSize(chaingang), 0) == -1)
         {
                 perror("send");
         }

    char *f_buffer = (char *)malloc(MAXDATASIZE);
    printf("waiting for file...\n");
    int flag = 1;
    unsigned char file_buffer[MAXDATASIZE];
	ret = recvfrom(sockfd, f_buffer, MAXDATASIZE, 0, NULL, NULL);
			    	if (ret < 0)
			        {
			            printf("Error receiving data!\n");
			        }
	printf("Received file <%s>\n", f_buffer);
	FILE *file_write;
	file_write = fopen(f_buffer, "wb");
    while(flag){
		ret = recvfrom(sockfd, file_buffer, MAXDATASIZE, 0, NULL, NULL);
		    	if (ret < 0)
		        {
		            printf("Error receiving data!\n");
		        }
		    	else if(file_buffer[0] == '*' && file_buffer[1] == '*' && file_buffer[2] == '*' && file_buffer[3] == '*'){

					flag = 0;
					//printf("%s\n",file_buffer);
					//printf("bytes recieved:%d\n",strlen(file_buffer));
					printf("Goodbye!\n");
				}
		    	else{
		    		//printf("%s\n",file_buffer);
		    		//printf("bytes recieved:%d\n",strlen(file_buffer));
		    		fwrite(file_buffer , 1 , MAXDATASIZE, file_write );
		    	}


    }
    fclose(file_write);
    close(sockfd);
    return 0;
}
