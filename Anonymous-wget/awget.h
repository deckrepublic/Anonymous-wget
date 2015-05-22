#include <stdio.h>
#include <stdlib.h>
#include <string.h> // string operations
#include <stdint.h> // fixed width integers

//linked list to hold ip address and its port number
typedef struct item{
  uint8_t addressLen;   // this is an unsigned 8-bit integer
  char *IP_address;
  uint8_t portLen;
  char *port;
  struct item *next;
} list;
// main struct holds the URL, how many stepping stones, and the linked list
typedef struct{
  uint8_t urlLen;   // this is an unsigned 8-bit integer
  char *url;
  uint8_t num_ss;
  list *content;
} Chain;

Chain* newChain(char *, int , list *);
list*  newList();
void serializeChain(Chain *item, char *buffer);
int listSize(list *item);
int chainSize(Chain *item);

//function to serialize the main struct Chain
void serializeChain(Chain *item, char *buffer)
{
    int seeker = 0;  // integer to keep record of the wrinting position in 'buffer'

    memcpy(&buffer[seeker], &item->urlLen, sizeof(item->urlLen));
    seeker += sizeof(item->urlLen); // move seeker ahead by a byte

    // copy characters from character array to the buffer (URL address)
    memcpy(&buffer[seeker], item->url, item->urlLen);
    seeker += item->urlLen; // ... and move the seeker ahead by the amount of
    			      //   characters in the array.
    //copy the number of servers into the buffer
    memcpy(&buffer[seeker], &item->num_ss, sizeof(item->num_ss));
    seeker += sizeof(item->num_ss); // ... and move the seeker ahead by the amount of
    			      //   characters in the array.

    list *ptr = newList();
    ptr = item->content;
    while(ptr->next != NULL) /* copy contents of the linked list in buffer as long there
      		      are items in the list. In this example, loop is
      		      done three times.  */
      {

        memcpy(&buffer[seeker], &ptr->addressLen, sizeof(ptr->addressLen));
        seeker += sizeof(ptr->addressLen); // move seeker ahead by a byte

        // copy characters from character array to the buffer (IP address)
        memcpy(&buffer[seeker], ptr->IP_address, ptr->addressLen);
        seeker += ptr->addressLen; // ... and move the seeker ahead by the amount of
        			      //   characters in the array.
        //copy the port number into the buffer
        memcpy(&buffer[seeker], &ptr->portLen, sizeof(ptr->portLen));
        seeker += sizeof(ptr->portLen); // ... and move the seeker ahead by the amount of
        			      //   characters in the array.
        memcpy(&buffer[seeker], ptr->port, sizeof(ptr->port));
        seeker += ptr->portLen; // ... and move the seeker ahead by the amount of


        ptr = ptr->next; // move on to the next item (or node) in the list
      }

}
//deserialize the chain
Chain * derializeChain(const char *buffer){
	//initial values

	uint8_t url_Len = buffer[0];
	char *chain_url_in = (char *)malloc(url_Len);
	int numb_ss;
	int end = strlen(buffer);
	list *first = newList();
	list *ptr = NULL;

	int seeker = 1;

	strncpy(chain_url_in, buffer+1, url_Len);
	chain_url_in[url_Len] = '\0';
	seeker+=url_Len;

    numb_ss = buffer[seeker];
	seeker++;


	while (seeker < end-1)
	{
		if(ptr==NULL){
			first->addressLen = buffer[seeker]; //deserialize address length
			seeker++;

			//deserialize IP address
			first->IP_address = malloc(first->addressLen);
			strncpy(first->IP_address, buffer+seeker, first->addressLen);
			first->IP_address[first->addressLen] = '\0';
			seeker+=first->addressLen;
			//deserialize port number
			first->portLen = buffer[seeker];
			seeker++;
			first->port = malloc(first->portLen);
			strncpy(first->port, buffer+seeker, first->portLen);
			first->port[first->portLen] = '\0';
			seeker+=first->portLen;


			first->next = newList();
			ptr = newList();
			ptr = first->next;
		}
		else{
			ptr->addressLen = buffer[seeker]; //deserialize address length
			seeker++;

			//deserialize IP address
			ptr->IP_address = malloc(ptr->addressLen);
			strncpy(ptr->IP_address, buffer+seeker, ptr->addressLen);
			ptr->IP_address[ptr->addressLen] = '\0';
			seeker+=ptr->addressLen;
			//deserialize port number
			ptr->portLen = buffer[seeker];
			seeker++;
			ptr->port = malloc(ptr->portLen);
			strncpy(ptr->port, buffer+seeker, ptr->portLen);
			ptr->port[ptr->portLen] = '\0';
			seeker+=ptr->portLen;


			ptr->next = newList();
			ptr = ptr->next;
		}
	}

	return newChain(chain_url_in,numb_ss,first);

}
int listSize(list *item)
{
  int size = 0;

  while (item->next != NULL) {
    size += item->addressLen;         /* add addressLen bytes to 'size' */
    size += sizeof(item->addressLen); /* add 4 bytes to 'size' */
    size += sizeof(item->port);       /* add port len bytes to 'size'   */
    item = item->next;              /* ... and to the batmobil! */
  }
  return size;
}
int chainSize(Chain *item)
{
  int size = listSize((item->content));

    size += item->urlLen;         /* add addressLen bytes to 'size' */
    size += sizeof(item->urlLen); /* add 4 bytes to 'size' */
    size += sizeof(item->num_ss);       /* add port len bytes to 'size'   */

  return size;
}

Chain* newChain(char *url_in, int num_ss_in, list *list_in)
{
	Chain *chaingang = (Chain*)malloc(sizeof(Chain));
	chaingang->urlLen = strlen(url_in);
	chaingang->url = url_in;
	chaingang->num_ss = num_ss_in;
	chaingang->content = list_in;
	return chaingang;
}
list* newList()
{
	list *element = (list*)malloc(sizeof(list));
	return element;
}
