#include "io_helper.h"
#include "request.h"
#include "semaphore.h"
#include "pthread"
#define MAXBUF (8192)

#define BUFFER_SIZE 10
int counter = 0;
//
//	TODO: add code to create and manage the buffer
//  look at slides   p.thread_create
thread_buffer(int fd){
  int buff[BUFFER_SIZE];
  sem_t sem;
  int count = 0;
  sem_init(&sem, 0, count);
    while(count<0){
      sem_wait(&sem);
      //threads doing stuff
      thread_request_serve_static(void* arg)
      sem_post(&sem);


pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
struct webRequest{
  int fd;
  char fname[MAXBUF];
  int size;
};

webRequest globalBuffer[20];


//
//	TODO: add code to create and manage the buffer
//  look at slides   p.thread_create
thread_buffer(int mode, int fd, char *filename, int filesize){
  if(filesize == -1){
    request_error(fd, filename, "101", "Sorter Error", "Incorrect Sorter Used");
    abort();
  }
  //Stuff to check buffer in for loop.

  pthread_mutex_lock(&lock); //Safe way to make double sure no double taking

  //grab from buffer
  //grab from buffer and store in buffRequest
  request_serve_static(buffRequest.fd, etc);
  //counter--;
  pthread_mutex_unlock(&lock);
}


//int buff[BUFFER_SIZE];
//sem_t sem;
//int count = 0;
//sem_init(&sem, 0, count);
  //while(count<0){
    //sem_wait(&sem);
    //threads doing stuff
    //request_serve_static(int fd, char *filename, int filesize)
    //sem_post(&sem);

//}

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
//
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

//
// Fetches the requests from the buffer and handles them (thread logic)
//
dataType threads[10];
void* thread_request_serve_static(int arg, int fd, char *filename)
{
	// TODO: write code to actualy respond to HTTP requests
  if(0){//FIFO
      thread_buffer(0, fd, *filename, -1)
  }
  if(1){ //SFF
      long start = ftell(fd);
      //take smallest in buffer
      long smallest = 100000;
      
      fseek(fd,0L, SEEK_END);
      long end = ftell(fd);
      fseek(fd, start, SEEK_SET);
      int size = end;

      thread_buffer(1, fd, *filename, filesize)
  }
  else{//random
      stuff.random
      thread_buffer(2, fd, *filename, -1);
  }
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

    if (strcasecmp(method, "GET")) {
      request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
      return;
      }

      
		// TODO: Make if for breakout attack
    // put request in array
		// TODO: write code to add HTTP requests in the buffer based on the scheduling policy

    webRequest newRequest = {fd, filename, sbuf.st_size};
    // if statement checking buffer and add global var
    


    } else {
		request_error(fd, filename, "501", "Not Implemented", "server does not serve dynamic content request");
    }
}
