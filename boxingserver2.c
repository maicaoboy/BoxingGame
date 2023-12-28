#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <curses.h>
#include <pthread.h>


void handle_move(int sockfd, char* uid, char* para);
void handle_defence(int sockfd, char* uid, char* para);
void handle_attack(int sockfd, char* uid, char* para);
void handle_new(int sockfd,char* uname);

#define BLANK " "
#define BOTTOM "#"
#define MAX_EVENTS 20

struct character{
        char name[20];
        int id;
        int isalive;
        int x;
        int y;
        int state;
        int hp;
	int sockfd;
};


struct character npcs[MAX_EVENTS];
struct epoll_event ev, events[MAX_EVENTS];


void setnonblocking(int conn_sock){
	int flag;
	flag=fcntl(conn_sock,F_GETFL);
	flag|=O_NONBLOCK;
	fcntl(conn_sock,F_SETFD,flag);
}

main(int ac,char*av[]){
	struct sockaddr_in addr;
	char buf[1024];
	int num;
	char cmd[512];
	char path[512];
	
	int n;
	int sockfd;

        int listen_sock, conn_sock, nfds, epollfd;

	char msgid[16];
	char uid[16];
	char uname[20];
	char para[16];
	int j;	

	signal(SIGPIPE,SIG_IGN);
	
	listen_sock = socket(AF_INET, SOCK_STREAM,0);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(av[1]));
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listen_sock,(const struct sockaddr *)&addr,sizeof(struct sockaddr_in))== -1){
		perror("Cannot bind");
		exit(1);
	}

	listen(listen_sock,1);

	for(n=0; n < MAX_EVENTS; n++){
		memset(npcs[n].name,0,20);
		npcs[n].id=-1;
		npcs[n].isalive=0;
		npcs[n].x=0;
		npcs[n].y=0;
		npcs[n].state=1;
		npcs[n].hp=5;
		npcs[n].sockfd=-1;
	}

        epollfd = epoll_create(10);
        if (epollfd == -1) {
            perror("epoll_create");
            exit(EXIT_FAILURE);
        }

        ev.events = EPOLLIN;
        ev.data.fd = listen_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }

        for (;;) {
            nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                perror("epoll_pwait");
                exit(EXIT_FAILURE);
            }

            for (n = 0; n < nfds; ++n) {
                if (events[n].data.fd == listen_sock) {
                    conn_sock = accept(listen_sock,NULL,NULL);
                    if (conn_sock == -1) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    setnonblocking(conn_sock);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_sock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                                &ev) == -1) {
                        perror("epoll_ctl: conn_sock");
                        exit(EXIT_FAILURE);
                    }
                } else {
			sockfd=events[n].data.fd;
			if((num=read(sockfd,buf,sizeof(buf)))==0){
				close(sockfd);
			}else {
				sscanf(buf,"%s",msgid);
				printf("%s\n",buf);
				if(strcmp(msgid,"1")==0){
					sscanf(buf, "%s%s", msgid, uname);
					printf("%s %s %s\n", msgid, uname);
					handle_new(sockfd, uname);
				}else if(strcmp(msgid,"2")==0){
					sscanf(buf, "%s%s",msgid, uid);
					
				}else if(strcmp(msgid,"3")==0){
					sscanf(buf, "%s%s%s", msgid, uid, para);
					printf("%s %s %s\n", msgid, uid, para);
					handle_move(sockfd,uid, para);
				}else if(strcmp(msgid,"4")==0){
                                        sscanf(buf, "%s%s%s", msgid, uid, para);
					printf("%s %s %s\n", msgid, uid, para);
					handle_defence(sockfd, uid, para);
                                }else if(strcmp(msgid,"5")==0){
                                        sscanf(buf, "%s%s%s", msgid, uid, para);
					printf("%s %s %s\n", msgid, uid, para);
					handle_attack(sockfd, uid, para);
                                }
				memset(buf,0, 1024);
				
			}
			buf[0]='\0';
                }
	}

	}

}

void handle_move(int sockfd, char* uid, char* para){
	int j=0;
	int id=atoi(uid);
	char msg[256]={0};
        strcpy(msg, "31 ");
        strcat(msg, uid);
        strcat(msg, " ");
        strcat(msg,para);
        strcat(msg,"\n\r");

	for(j=0; j < MAX_EVENTS;j++){
		if(npcs[j].id!=-1 && npcs[j].sockfd != sockfd){
			write(npcs[j].sockfd, msg, strlen(msg));
		}
		if(npcs[j].id==id){
			npcs[j].y=atoi(para);
		}	
	}
}

void handle_defence(int sockfd, char* uid, char* para){
	int j=0;
	int id=atoi(uid);
	char msg[256]={0};
        strcpy(msg, "32 ");
        strcat(msg, uid);
        strcat(msg, " ");
        strcat(msg,para);
        strcat(msg,"\n\r");

	for(j=0; j < MAX_EVENTS;j++){
		if(npcs[j].id!=-1 && npcs[j].sockfd != sockfd){
			write(npcs[j].sockfd, msg, strlen(msg));
		}
		if(npcs[j].id==id){
                        npcs[j].state=atoi(para)+1;
                }
	}
}

void handle_attack(int sockfd, char* uid, char* para){
	int j=0;
	int a=-1;
	int id=atoi(uid);
	char msg[256]={0};
	char num[128]={0};
        strcpy(msg, "34 ");
        strcat(msg, uid);
        strcat(msg, " ");
        strcat(msg,para);
        strcat(msg,"\n\r");

	for(j=0; j < MAX_EVENTS;j++){
		if(npcs[j].id!=-1){
			write(npcs[j].sockfd, msg, strlen(msg));
		}
		if(npcs[j].id==id){
			a=j;
                        npcs[j].state=atoi(para)+3;
                }
	}
	int orient = atoi(para);
	int aid = atoi(uid);
	int did;

	if(a==-1||a==MAX_EVENTS) return;
	printf("pointttttttt1\n");

	for(j=0; j<MAX_EVENTS;j++){
		if(j==a || npcs[j].id == -1) continue;
		if(orient==1){
			if(npcs[j].y < npcs[a].y && (npcs[a].y-npcs[j].y)<=4 && npcs[j].state != 3){
				npcs[j].hp--;
				break;
			}			
		}else{
			if(npcs[a].y < npcs[j].y && (npcs[j].y-npcs[a].y)<=4 && npcs[j].state != 2){
				npcs[j].hp--;
				break;
			}				
		}	
	}
	if(j==MAX_EVENTS) return;
	printf("pointttttttt2\n");
	strcpy(msg, "27 ");
       	sprintf(num, "%d %d\n\t", npcs[j].id, npcs[j].hp);
        strcat(msg, num);
	
	for(j=0;j<MAX_EVENTS;j++ ){
		if(npcs[j].id != -1){
			write(npcs[j].sockfd, msg, strlen(msg));
			printf("Bloooooooo%d:%s\n",j,msg);
		}
	}
}


void handle_new(int sockfd,char* uname){
	char msg[256]={0};
	char num[256]={0};
	strcpy(msg, "26 ");
	int id = rand();
	int y = rand() % 70;
	int j;
	for(j = 0; j < MAX_EVENTS; j++){
		if(npcs[j].id==-1){
			strcpy(npcs[j].name,uname);
			npcs[j].id=id;
			npcs[j].isalive=1;
			npcs[j].y=y;
			npcs[j].state=1;
			npcs[j].hp=5;
			npcs[j].sockfd=sockfd;
			break;
		}
	}
	if(j!=MAX_EVENTS){
		/*msg: 26 id y */
        	sprintf(num, "%d", id);
	        strcat(msg, num);
        	strcat(msg, " ");
        	sprintf(num, "%d", y);
	        strcat(msg, num);
	}else {
		/*msg: 26 -1 -1*/
		sprintf(num, "%d", -1);
                strcat(msg, num);
                strcat(msg, " ");
                sprintf(num, "%d", -1);
                strcat(msg, num);
	}
	strcat(msg,"\t\n");
	write(sockfd, msg, strlen(msg));
	
	/*synchonize player*/
	usleep(300000);
	if(j == MAX_EVENTS) return;
	int k=j;
	for(j=0; j < MAX_EVENTS; j++){
		if(npcs[j].id != -1 && npcs[j].sockfd != sockfd){
			/*send all players on server to new user*/
			memset(msg, 0, 256);
			memset(num, 0, 256);
			strcpy(msg, "33 ");
			strcat(msg, npcs[j].name);
			sprintf(num, " %d %d %d %d %d", npcs[j].id, npcs[j].y, npcs[j].state, npcs[j].hp, npcs[j].sockfd);
			strcat(msg, num);
			strcat(msg,"\t\n");
			write(sockfd, msg, strlen(msg));
			
			/*brocast new user to all player*/
			memset(msg, 0, 256);
			memset(num, 0, 256);
			strcpy(msg, "29 ");
			strcat(msg, npcs[k].name);
			sprintf(num, " %d %d %d %d %d", npcs[k].id, npcs[k].y, npcs[k].state, npcs[k].hp, npcs[k].sockfd);
			strcat(msg, num);
			strcat(msg,"\t\n");
			write(npcs[j].sockfd, msg, strlen(msg));
			
		}
	}
}
