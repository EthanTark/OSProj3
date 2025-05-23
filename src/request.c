#include "io_helper.h"
#include "request.h"
#include "semaphore.h"
#include "pthread.h"
#define MAXBUF (8192)

pthread_t threads[10];

int counter = 0;//Counter for organizer
int tCount = 0; //counter for thread taker
int small = 10000; //Keeping track of smallest file
int curr_buff_size = 0;//Keeping track of things in buffer

pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
typedef struct { //Struct to hold data for HTTP request
  int fd; //File Data
  char fname[MAXBUF]; // File Name
  int size; // Size of File
  int counter; // Counter for SFF
 } webRequest;

webRequest globalBuffer[20]; // Buffer to hold request in

//Remember the buffer takes things from back and they will move to front
// below default values are defined in 'request.h'
int num_threads = DEFAULT_THREADS; //Num of Threads
int buffer_max_size = DEFAULT_BUFFER_SIZE; //Buffer Max Size
int scheduling_algo = DEFAULT_SCHED_ALGO; // What Algorithm will be run

//
//	---TODO: add code to create and manage the shared global buffer of requests
//	HINT: You will need synchronization primitives.
//		pthread_mutuex_t lock_var is a viable option.
//

//
// Sends out HTTP response in case of errors
//
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXBUF], body[MAXBUF];
    
    // Create the body of error message first (have to know its length for header)
    sprintf(body, ""
	    "<!doctype html>\r\n"
	    "<head>\r\n"
	    "  <title>CYB-3053 WebServer Error</title>\r\n"
	    "</head>\r\n"
	    "<body>\r\n"
	    "  <h2>%s: %s</h2>\r\n" 
	    "  <p>%s: %s</p>\r\n"
	    "</body>\r\n"
	    "</html>\r\n", errnum, shortmsg, longmsg, cause);
    
    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    write_or_die(fd, buf, strlen(buf));
    
    sprintf(buf, "Content-Type: text/html\r\n");
    write_or_die(fd, buf, strlen(buf));
    
    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    write_or_die(fd, buf, strlen(buf));
    
    // Write out the body last
    write_or_die(fd, body, strlen(body));
    
    // close the socket connection
    close_or_die(fd);
}

//
// Reads and discards everything up to an empty text line
//
void request_read_headers(int fd) {
    char buf[MAXBUF];
    
    readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n")) {
	readline_or_die(fd, buf, MAXBUF);
    }
    return;
}

//
// Return 1 if static, 0 if dynamic content (executable file)
// Calculates filename (and cgiargs, for dynamic) from uri
int request_parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;
    
    if (!strstr(uri, "cgi")) { 
	// static
	strcpy(cgiargs, "");
	sprintf(filename, ".%s", uri);
	if (uri[strlen(uri)-1] == '/') {
	    strcat(filename, "index.html");
	}
	return 1;
    } else { 
	// dynamic
	ptr = index(uri, '?');
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	} else {
	    strcpy(cgiargs, "");
	}
	sprintf(filename, ".%s", uri);
	return 0;
    }
}

//
// Fills in the filetype given the filename
//
void request_get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html")) 
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif")) 
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg")) 
	strcpy(filetype, "image/jpeg");
    else 
	strcpy(filetype, "text/plain");
}

//
// Handles requests for static content
//
void request_serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXBUF], buf[MAXBUF];
    
    request_get_filetype(filename, filetype);
    srcfd = open_or_die(filename, O_RDONLY, 0);
    
    // Rather than call read() to read the file into memory, 
    // which would require that we allocate a buffer, we memory-map the file
    srcp = mmap_or_die(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close_or_die(srcfd);
    
    // put together response
    sprintf(buf, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: OSTEP WebServer\r\n"
	    "Content-Length: %d\r\n"
	    "Content-Type: %s\r\n\r\n", 
	    filesize, filetype);
       
    write_or_die(fd, buf, strlen(buf));
    
    //  Writes out to the client socket the memory-mapped file 
    write_or_die(fd, srcp, filesize);
    munmap_or_die(srcp, filesize);
}

int grabber(){ //Special function for grabbing the correct request
    if(scheduling_algo==0){ //FIFO
            return 0; //what ever is first in Buffer should be right
  }
    if(scheduling_algo==1){ //SFF
        int index = 0; //Where to start in buffer
        int smallest = small; //Getting default smallest value of 100000
        for (int i=0; i < curr_buff_size; i++){    // Iterate through buffer
            globalBuffer[i].counter++; //counter for how long request been in buffer
            if(globalBuffer[i].counter>=20){ // checking that request isn't starved
                return i; //return it if starving
            }
            if (globalBuffer[i].size < smallest){ // checking for smallest request
                smallest = globalBuffer[i].size; //updating smallest
                index = i; //updating place of smallest
            }   
            return index; //return the place of smallest
        }
    return (rand() % curr_buff_size); //Random
    }
}

//
// Fetches the requests from the buffer and handles them (thread logic)
//
//dataType threads[10];
void* thread_request_serve_static(void* arg)
{
    
    // TODO: write code to actualy respond to HTTP requests
    // Pull from global buffer of requests
    while(counter<=20){ //checking on buffer requests
        int curr = grabber(); //grab place of request based on algo
        webRequest threadRequest = globalBuffer[curr]; //putting in another place
        pthread_mutex_lock(&lock); //Safe way to make double sure no double taking
        
        request_serve_static(threadRequest.fd, threadRequest.fname, threadRequest.size); //Use thread to do request
        curr_buff_size--; //minus from buffer size
        pthread_mutex_unlock(&lock); //let other thread take request
    }
    return NULL;
}


//
// Initial handling of the request
//
void request_handle(int fd) {
    int is_static;
    struct stat sbuf;
    char buf[MAXBUF], method[MAXBUF], uri[MAXBUF], version[MAXBUF];
    char filename[MAXBUF], cgiargs[MAXBUF];
    
    // get the request type, file path and HTTP version
    readline_or_die(fd, buf, MAXBUF);
    sscanf(buf, "%s %s %s", method, uri, version);
    printf("method:%s uri:%s version:%s\n", method, uri, version);

    // verify if the request type is GET or not
    if (strcasecmp(method, "GET")) {
	request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
	return;
    }
    request_read_headers(fd);
    
    // check requested content type (static/dynamic)
    is_static = request_parse_uri(uri, filename, cgiargs);
    
    // get some data regarding the requested file, also check if requested file is present on server
    if (stat(filename, &sbuf) < 0) {
	request_error(fd, filename, "404", "Not found", "server could not find this file");
	return;
    }
    
    // verify if requested content is static
    if (is_static) {
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
		request_error(fd, filename, "403", "Forbidden", "server could not read this file");
		return;
	}

    if(strstr(filename, "..")){ //prevention for traversal attack
        request_error(fd, filename, "101", "Forbidden", "Unauthorized access, server could not read this file");
		return;
    }
    
	// TODO: directory traversal mitigation	
	// TODO: write code to add HTTP requests in the buffer
    webRequest newRequest = {fd, filename, sbuf.st_size, 0}; //getting needed info for struct
    if(curr_buff_size<20){ //checking for buffer size
        globalBuffer[curr_buff_size] = newRequest; // making buffer for request
        curr_buff_size++; //increasing size for what in buffer
    }

    // if statement checking buffer and add global var
    


    } else {
	request_error(fd, filename, "501", "Not Implemented", "server does not serve dynamic content request");
    }
}
