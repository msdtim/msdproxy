#include "csapp.h"
#include "cache.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char *user_agent = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_str = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding = "Accept-Encoding: gzip, deflate\r\n";
static const char *proxy_conn = "Proxy-Connection: close\r\n";
static const char *conn_str = "Connection: close\r\n";

// Global value


void *justdoit(void *connfd);
int parse_uri(char *uri, char *hostname, char *pathname, int *port);
int read_requesthdrs(rio_t *rp, char *host, char *requesthdrs);

int main(int argc, char **argv)
{
    //pthread_t tid;

    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    
    int listenfd = Open_listenfd(port);
    int *connfd;
    struct sockaddr_in clientaddr;
    // struct Cache_List *
    init_cache();
    //printf("lalallal\n");
    
    socklen_t clientlen = sizeof(struct sockaddr_in);
    
    while (1) {
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd,(SA *)&clientaddr, &clientlen);
        
        //Pthread_create(&tid, NULL, justdoit, connfd);
        justdoit(connfd);
        
    }
}

void *justdoit(void *connfd)
{
    int fd = *((int *) connfd);
    //printf("in the thread: \n");
    //Pthread_detach(pthread_self());

    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
	char hostname[MAXLINE], pathname[MAXLINE];
	int port;    
	char newuri[MAXLINE];
    // int incache;        // if request find in cache, =1, else, =0


    rio_t rio;
    printf("fd is: %d\n", fd);
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    
    sscanf(buf, "%s %s %s", method, uri, version);
    //printf("%s %s %s\n", method, uri, version);
    if (strcasecmp(method, "GET")) {
        printf("we did not got something\n");
        return NULL;

    }
    else{
        printf("we got something\n");
		int flag = parse_uri(uri, hostname, pathname, &port);

		if (flag != 1){
			printf("parse failed");
		}
    }
    char requesthdrs[MAXLINE];
    
    
    int ret = read_requesthdrs(&rio, hostname, requesthdrs);
    
	//got the hostname and the pathname, port = 80
	
	//forward request 

	//handleing request header
	
	//strcat(newuri, " http://");
	//strcat(newuri, hostname);
	//strcat(newuri, ":80");
	/*if (pathname[0] != '\0'){
		strcat(newuri, " /");
		strcat(newuri, pathname);
	}
	else{
		strcat(newuri, " http://");
		strcat(newuri, hostname);
		
		//strcat(newuri, ":80");
		strcat(newuri, " /");
	}
	strcat(newuri, " HTTP/1.0");
	newuri[strlen(newuri)] = '\0';*/

	//int clientfd = Open_clientfd(hostname, 80);//!!!!!!!!!!!!!!!!!!!!!!
	/*rio_t* rio2;
	int n = 0;
	Rio_readinitb(&rio2, clientfd);
	

	if (pathname[0] != '\0'){
		strcpy(newuri, method);
		strcat(newuri, " /");
		strcat(newuri, pathname);
		strcat(newuri, " HTTP/1.0");
		newuri[strlen(newuri)] = '\0';
		printf("newuri = %s\n", newuri);
		Rio_writen(clientfd,newuri,strlen(newuri));
	}
	else{
		strcpy(newuri, method);
		strcat(newuri, " http://");
		strcat(newuri, hostname);
		
		//strcat(newuri, ":80");
		
		strcat(newuri, "//");
		strcat(newuri, " HTTP/1.0");
		newuri[strlen(newuri)] = '\0';
		printf("newuri = %s\n", newuri);
		Rio_writen(clientfd,newuri,strlen(newuri));
	}*/


	/*while ((n = Rio_readlineb(&rio,buf,MAXLINE)) != 0){
		if (strstr(buf, "User-Agent:") != NULL) {
			strcpy(buf,user_agent);
		}
		else if (strstr(buf, "Accept:") != NULL) {
			strcpy(buf,accept_str);	
		}
		else if (strstr(buf, "Accept-Encoding:") != NULL) {
			strcpy(buf,accept_encoding);
		}
		else if (strstr(buf, "Proxy-Connection:") != NULL) {
			strcpy(buf,proxy_conn);
		}
		else if (strstr(buf, "Connection:") != NULL) {
			strcpy(buf,conn_str);
		}
		
		n = strlen(buf);


		Rio_writen(clientfd, buf,n);

		if (!strcmp(buf, "\r\n")){
			//Rio_writen(clientfd, (char* )conn_str, strlen(conn_str));
			//Fputs(conn_str,stdout);
			printf("reach the end\n");
			break;		
		}
		Fputs(buf,stdout);
	}*/
	
    struct Block *block;
    if ((block = find_block(uri)) != NULL) {
        printf("Find Block with the uri: %s\n", block->uri);
        //printf("%s\n", block->head);

        Rio_writen(fd, block->head, strlen(block->head));
        Rio_writen(fd, block->body, strlen(block->body));
        move_to_tail(block);
        Close(fd);
        return NULL;
    }

    printf("Didn't find Block :( \n");
    
    //clientfd is server
    int clientfd = open_clientfd(hostname, port);
    //printf("port is: %d\n", port);
    //printf("something about git\n");
    printf("clientfd is: %d\n", clientfd);
    
    sprintf(buf, "GET %s HTTP/1.0\r\n", pathname);
    //printf("GET Line is: %s\n", buf);
    
    Rio_writen(clientfd, buf, strlen(buf));
    Rio_writen(clientfd, (void *)user_agent, strlen(user_agent));
    Rio_writen(clientfd, (void *)accept_str, strlen(accept_str));
    Rio_writen(clientfd, (void *)accept_encoding, strlen(accept_encoding));
    Rio_writen(clientfd, (void *)conn_str, strlen(conn_str));
    Rio_writen(clientfd, (void *)proxy_conn, strlen(proxy_conn));
    Rio_writen(clientfd, requesthdrs, strlen(requesthdrs));
    
    //printf("Requesthdrs Line is: %s\n", requesthdrs);
    
    Rio_writen(clientfd, "\r\n", 2);
    
    
    
    
    

	//int listenfd = Open_listenfd(80);
    
    rio_t rio2;
    
    char head[MAXBUF];
    char body[MAX_OBJECT_SIZE];
    int bodysize = -1;
    int bodylen, headlen;
    
    memset(head, 0, sizeof(head));
    memset(body, 0, sizeof(body));
    
    Rio_readinitb(&rio2, clientfd);
    
    int headsize = 0;
    while(1){
        headlen = Rio_readlineb(&rio2, buf, MAXLINE);
        sprintf(head, "%s%s", head, buf);
        headsize += headlen;
        //printf("BUF: %s\n", buf);
        if((strcmp(buf, "\r\n") == 0) || (strcmp(buf, "\n") == 0))
            break;
    }
    //printf("head: %s\n", head);
    Rio_writen(fd, head, strlen(head));
    
    bodysize = 0;
    //int counter=0;
    while((bodylen = Rio_readnb(&rio2, body, MAX_OBJECT_SIZE)) > 0){
        //counter++;
        //printf("BODY: %s\n\n\ncounter: %d\n", body, counter);
        Rio_writen(fd, body, bodylen);
        bodysize += bodylen;
    }

    update_cache(uri, head, body, headsize, bodysize);

    print_cache();

    // add_block(uri, body, bodysize);
    
	/*while((n = Rio_readlineb(&rio2,buf,MAXLINE)) != 0){
		printf("%s\n", buf);
		Rio_writen(fd,buf,n);
	}*/
	Close(fd);
    Close(clientfd);
	//read_requesthdrs(&rio, clientfd);
	

    return NULL;
}

int read_requesthdrs(rio_t *rp, char *host, char *requesthdrs)
{
    char buf[MAXLINE];
    
    char ignore[5][20] = {"User-Agent:", "Accept:", "Accept-Encoding:", "Connection:", "Proxy-Connection:"};
    int skim;
    int hostexist = 0;
    int len = 0;
    int buflen, i;
    
    while(1) {
        if (Rio_readlineb(rp, buf, MAXLINE-2) < 0)
            return -1;
        if (strncmp(buf, "\r\n", 2) == 0 || strncmp(buf, "\n", 1) == 0)
            break;
        buflen = strlen(buf);
        //skim the specified fields
        skim = 0;
        for (i = 0; i < 5; i++)
            if (!strncasecmp(buf, ignore[i], strlen(ignore[i]))) {
                skim = 1;
                break;
            }
        if (skim) continue;
        if (!strncasecmp(buf, "Host: ", 5))
            hostexist = 1;
        // adjust '\n' to "\r\n"
        if (buf[buflen-2] != '\r') {
            buf[buflen-1] = '\r';
            buf[buflen] = '\n';
            buf[++buflen] = '\0';
        }
        
        len += buflen;
        if (len < MAXBUF) {
            strcpy(requesthdrs, buf);
            requesthdrs += buflen;
        }
        else
            return 0;
    }
    if (!hostexist) {
        len += 7+strlen(host);
        if (len < MAXBUF) {
            strcpy(requesthdrs, "Host: ");
            requesthdrs += 6;
            strcpy(requesthdrs, host);
            strcpy(requesthdrs + strlen(host), "\r\n");
        }
        else
            return 0;
    }
    return 1;
}

int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
	//printf("parsing... %s\n", uri);
	
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;
	
    
    if (strncasecmp(uri, "http://", 7) != 0) {
        return 0;
    }
    
    // Extract the host name
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
	//printf("host is: %s\n", hostname);
    
    // Extract the path
    pathbegin = strchr(hostbegin, '/');

    if (pathbegin == NULL) {
        strcpy(pathname, "/");
    }
    else {
        pathbegin++;
        strcpy(pathname, "/");
        strcat(pathname, pathbegin);
    }
    
    *port = 80; /* default */
    
    if (*hostend == ':')
        *port = atoi(hostend + 1);
    
    /*char buf[MAXLINE];
    
    // if the uri doesn't begin with "http:", error
    if( (strncasecmp(uri, "http://", 7)) != 0)
    {
        return 0;
    }
    else{
        strcpy(buf, uri+7);
    }
    
    
    if( strstr(buf, "/") == NULL){
        // no path exist
        strcpy(pathname, "/");
        strcpy(hostname, buf);
    }
    else{
        int hostlen = strcspn(buf, "/");
        strcpy(pathname, buf + hostlen);
        strncpy(hostname, buf, hostlen);
        hostname[hostlen] = '\0';
    }*/
	
	//printf("hostname %s, pathname %s\n", hostname, pathname);
	
    return 1;
}
