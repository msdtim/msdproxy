#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char *user_agent = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_str = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding = "Accept-Encoding: gzip, deflate\r\n";
static const char *proxy_conn = "Proxy-Connection: close\r\n";
static const char *conn_str = "Connection: close\r\n";

void *justdoit(void *connfd);
int parse_uri(char *uri, char *hostname, char *pathname, int *port);
int read_requesthdrs(rio_t *rio, char *hostname, char *requesthdrs);
void proxyerror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
void int_handler(int sig);

void int_handler(int sig)
{
    printf("Exit\n");
    //free_cache();
    exit(0);
}



int main(int argc, char **argv)
{
    //pthread_t tid;
    Signal(SIGPIPE, SIG_IGN);
    Signal(SIGINT, int_handler);
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    
    int listenfd = Open_listenfd(port);
    int *connfd;
    struct sockaddr_in clientaddr;
    
    socklen_t clientlen = sizeof(struct sockaddr_in);
    
    while (1) {
        connfd = malloc(sizeof(int));
        *connfd = Accept(listenfd,(SA *)&clientaddr, &clientlen);
        
        //Pthread_create(&tid, NULL, justdoit, connfd);
        justdoit(connfd);
    }
    return 0;
}


void *justdoit(void *connfd)
{
    int client = *((int *) connfd);
    Free(connfd);
    Pthread_detach(pthread_self());

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], pathname[MAXLINE];
	int port;    
	//char newuri[MAXLINE];

    rio_t rio;
    Rio_readinitb(&rio, client);
    if (Rio_readlineb(&rio, buf, MAXLINE) < 0){
        Close(client);
        return NULL;
    }

    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        proxyerror(client, method, "501", "Not Implemented",
                    "Tiny Proxy does not implement this method");
        Close(client);
        return NULL;
    }
    else{
		if (parse_uri(uri, hostname, pathname, &port) == 0){
			proxyerror(client, uri, "400", "Bad request",
                       "Tiny Proxy cannot parse the URI");
            Close(client);
            return NULL;
		}
    }
    char requesthdrs[MAXLINE];
    
    int read_ret = read_requesthdrs(&rio, hostname, requesthdrs);
    if (read_ret == -1) {
        Close(client);
        return NULL;
    }
    else if (read_ret == -0){
        proxyerror(client, uri, "500", "Internal server error",
                   "Tiny Proxy cannot handle such a long request header");
        Close(client);
        return NULL;
    }
	
    
    int server = open_clientfd(hostname, port);
    
    
    //printf("port is: %d\n", port);
    //printf("something about git\n");
    printf("server is: %d\n", server);
    
    //printf("GET Line is: %s\n", buf);
    if (server == -2){
        proxyerror(client, uri, "404", "Not found",
                    "Tiny Proxy got DNS (gethostbyname) error");
        //Close(server);
        Close(client);
        return NULL;
    }
    if (server == -1){
        proxyerror(client, uri, "403", "Forbidden",
                   "Tiny Proxy cannot connect server");
        //Close(server);
        Close(client);
        return NULL;
    }

    sprintf(buf, "GET %s HTTP/1.0\r\n", pathname);
    sprintf(buf, "%s%s", buf, user_agent);
    sprintf(buf, "%s%s", buf, accept_str);
    sprintf(buf, "%s%s", buf, accept_encoding);
    sprintf(buf, "%s%s", buf, conn_str);
    sprintf(buf, "%s%s", buf, proxy_conn);
    if (rio_writen(server, buf, strlen(buf)) < 0) {
        Close(client);
        Close(server);
        return NULL;
    }
    if (rio_writen(server, requesthdrs, strlen(requesthdrs)) < 0) {
        Close(client);
        Close(server);
        return NULL;
    }
    if (rio_writen(server, "\r\n", 2) < 0) {
        Close(client);
        Close(server);
        return NULL;
    }
    
    rio_t rio2;
    
    char head[MAXBUF];
    char body[MAX_OBJECT_SIZE];
    int bodysize;
    int len;
    
    memset(head, 0, sizeof(head));
    memset(body, 0, sizeof(body));
    memset(buf, 0, sizeof(buf));
    Rio_readinitb(&rio2, server);
    
    //read from the sever, then write on the client
    //read header
    while((strcmp(buf, "\r\n") != 0) && (strcmp(buf, "\n") != 0)){
        if (Rio_readlineb(&rio2, buf, MAXLINE) < 0){
            Close(client);
            Close(server);
            return NULL;
        }
        sprintf(head, "%s%s", head, buf);
        //printf("BUF: %s\n", buf);
        //if((strcmp(buf, "\r\n") == 0) || (strcmp(buf, "\n") == 0))
        //    break;
    }
    
    //write header
    if (rio_writen(client, head, strlen(head)) < 0) {
        Close(client);
        Close(server);
        return NULL;
    }
    
    //printf("HEAD: %s\n", head);
    
    //read and write body
    bodysize = 0;
    while((len = Rio_readnb(&rio2, body, MAX_OBJECT_SIZE)) > 0){
        //printf("BODY: %s\n", body);
        if (rio_writen(client, body, len) < 0) {
            Close(client);
            Close(server);
            return NULL;
        }
        bodysize += len;
    }
    
	Close(client);
    Close(server);
    return NULL;
}

int read_requesthdrs(rio_t *rio, char *hostname, char *hdrs)
{
    char buf[MAXLINE];
    
    int ignore;
    int hostexist = 0;
    int len = 0;
    int buflen;
    
    //while(1) {
    while((strcmp(buf, "\r\n") != 0) && (strcmp(buf, "\n") != 0)){
        if (Rio_readlineb(rio, buf, MAXLINE-2) < 0)
            return -1; //can't read header
        
        //if (strncmp(buf, "\r\n", 2) == 0 || strncmp(buf, "\n", 1) == 0)
        //    break;
        
        buflen = strlen(buf);

        ignore = 0;
        if (strncasecmp(buf,"User-Agent:",11) == 0)
            ignore = 1;
        if (strncasecmp(buf,"Accept:",7) == 0)
            ignore = 1;
        if (strncasecmp(buf,"Accept-Encoding:",16) == 0)
            ignore = 1;
        if (strncasecmp(buf,"Connection:",11) == 0)
            ignore = 1;
        if (strncasecmp(buf,"Proxy-Connection:",17) == 0)
            ignore = 1;
        
        if (ignore) continue;
        
        if (!strncasecmp(buf, "Host:", 5))
            hostexist = 1;
        
        // adjust '\n' to "\r\n"
        if (buf[buflen-2] != '\r') {
            buf[buflen-1] = '\r';
            buf[buflen] = '\n';
            buf[++buflen] = '\0';
        }
        
        len += buflen;
        if (len < MAXBUF) {
            strcpy(hdrs, buf);
            hdrs += buflen;
        }
        else
            return 0; //header too large
    }
    
    if (hostexist == 0) {
        len += 7+strlen(hostname);
        if (len < MAXBUF) {
            strcpy(hdrs, "Host: ");
            hdrs += 6;
            strcpy(hdrs, hostname);
            strcpy(hdrs + strlen(hostname), "\r\n");
        }
        else
            return 0; //header too large
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
	
    return 1;
}

void proxyerror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];
    
    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Proxy, developed by Qin Liu & Shuda Mo </em>\r\n", body);
    
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    
    if (rio_writen(fd, buf, strlen(buf)) < 0){
        //Close(fd);
        return;
    }
    sprintf(buf, "Content-type: text/html\r\n");
    if (rio_writen(fd, buf, strlen(buf)) < 0){
        //Close(fd);
        return;
    }
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    if (rio_writen(fd, buf, strlen(buf)) < 0){
        //Close(fd);
        return;
    }
    if (rio_writen(fd, body, strlen(buf)) < 0){
        //Close(fd);
        return;
    }
}

