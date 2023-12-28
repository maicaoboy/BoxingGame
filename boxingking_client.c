#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <curses.h>
#include <pthread.h>
#include <string.h>

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

void moving(struct character *npc);
void clearmoving(struct character *npc);
void *gamereceiver(void* arg);



main(int ac, char *av[]){
        char input;
        pthread_t t;
	int i;
	int h;
	int hnpc;

	int sockfd;
	int psockfd;
	struct sockaddr_in addr;
	int fd;
	char buf[1024];
	int n;

	char msg[1024];
	char num[32];
	char msgid[16];
	char uid[16];
	char uname[20];
	char para[16];

	if(ac!=3){
		printf("Use: CMD IPADDR PORT_NUM\n");
		exit(1);
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("Cannot make socket");
		exit(1);
	}

	addr.sin_family=AF_INET;
	addr.sin_port=htons(atoi(av[2]));
	addr.sin_addr.s_addr=inet_addr(av[1]);

	if(connect(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))==-1){
		perror("Cannot connect");
		exit(1);
	}

	
	printf("Enter your username:");
	scanf("%s", npcs[0].name);
	printf("Welcome to BoxingKing...\n");

	/*inite game --msg: 1 uname */
	strcpy(msg, "1 ");
	strcat(msg, npcs[0].name);
	printf("msg:%s\n", msg);
	write(sockfd, msg, strlen(msg));

	n=read(sockfd, buf, sizeof(buf));
	initscr();
        noecho();
        crmode();
        clear();
        h=LINES*2/3;
        hnpc=LINES*2/3-1;
	for(i=0;i<MAX_EVENTS;i++){
                memset(npcs[i].name,0,20);
                npcs[i].id=-1;
                npcs[i].isalive=0;
                npcs[i].x=hnpc;
                npcs[i].y=0;
                npcs[i].state=1;
                npcs[i].hp=0;
		npcs[i].sockfd=-1;
        }

	psockfd=sockfd;
        pthread_create(&t, NULL, gamereceiver, &psockfd);
	sscanf(buf, "%s",msgid);
	if(strcmp(msgid,"26")==0){
		sscanf(buf, "%s%s%s", msgid, uid, para);
		npcs[0].id=atoi(uid);
		npcs[0].y=atoi(para);
		
		if(npcs[0].id==-1){
			printf("Server is full, please try again latter!\n");
	                close(sockfd);
        	        exit(0);

		}		
	}else {
		printf("Server error, please try again latter!\n");
		close(sockfd);
		exit(0);
	}


	
        npcs[0].x=hnpc;
        npcs[0].isalive=1;
	npcs[0].state=1;
	npcs[0].hp=5;
        moving(&npcs[0]);
	for(i=0;i<COLS;i++){
		move(h,i);
		addstr(BOTTOM);
	}
        move(LINES-1, COLS-1);
	refresh();

	

        while(input=getchar()){
                /*when user input*/
                if(!npcs[0].isalive) break;
		memset(msg, 0, 1024);
                if (input == 'a' && npcs[0].y>0) {
			/*go left*/
			bool flag=false;
			for(i=0;i<MAX_EVENTS;i++){
				if(npcs[i].id >0 && npcs[0].y > npcs[i].y && (npcs[0].y-npcs[i].y)<=3){
					flag=true;
					break;
				}
			}
			if(flag) continue;
			clearmoving(&npcs[0]);
			npcs[0].y--;
			npcs[0].state=1;
			moving(&npcs[0]);
			strcpy(msg, "3 ");
			sprintf(num, "%d", npcs[0].id);
			strcat(msg, num);
			strcat(msg," ");
			sprintf(num, "%d", npcs[0].y);
			strcat(msg, num);
			write(sockfd, msg, strlen(msg));
		}else if (input == 'd' && npcs[0].y<COLS-1){
			/*go right*/
			bool flag=false;
			for(i=0;i<MAX_EVENTS;i++){
				if(npcs[i].id >0 && npcs[i].y > npcs[0].y && (npcs[i].y-npcs[0].y)<=3){
					flag=true;
					break;
				}
			}
			if(flag) continue;
			clearmoving(&npcs[0]);
			npcs[0].y++;
			npcs[0].state=1;
			moving(&npcs[0]);
			strcpy(msg, "3 ");
			sprintf(num, "%d", npcs[0].id);
			strcat(msg, num);
			strcat(msg," ");
			sprintf(num, "%d", npcs[0].y);
			strcat(msg, num);
			write(sockfd, msg, strlen(msg));
		}else if(input == 'i'){
			/*defence left*/
			clearmoving(&npcs[0]);
			npcs[0].state=2;
			moving(&npcs[0]);
			strcpy(msg, "4 ");
			sprintf(num, "%d", npcs[0].id);
			strcat(msg, num);
			strcat(msg," ");
			sprintf(num, "%d", 1);
			strcat(msg, num);
			write(sockfd, msg, strlen(msg));
		}else if(input == 'o'){
			/*defence right*/
			clearmoving(&npcs[0]);
			npcs[0].state=3;
			moving(&npcs[0]);
			strcpy(msg, "4 ");
			sprintf(num, "%d", npcs[0].id);
			strcat(msg, num);
			strcat(msg," ");
			sprintf(num, "%d", 2);
			strcat(msg, num);
			write(sockfd, msg, strlen(msg));
		}else if(input == 'k'){
			/*attack left*/
			clearmoving(&npcs[0]);
			npcs[0].state=4;
			moving(&npcs[0]);
			strcpy(msg, "5 ");
			sprintf(num, "%d", npcs[0].id);
			strcat(msg, num);
			strcat(msg," ");
			sprintf(num, "%d", 1);
			strcat(msg, num);
			write(sockfd, msg, strlen(msg));
		}else if(input == 'l'){
			/*attack right*/
			clearmoving(&npcs[0]);
			npcs[0].state=5;
			moving(&npcs[0]);
			strcpy(msg, "5 ");
			sprintf(num, "%d", npcs[0].id);
			strcat(msg, num);
			strcat(msg," ");
			sprintf(num, "%d", 2);
			strcat(msg, num);
			write(sockfd, msg, strlen(msg));
			/*printf("%s", msg);*/
		}else{
			
		}

                move(LINES-1, COLS-1);
                refresh();
        }

        endwin();
}


void *gamereceiver(void* arg){
	char name[20];
	char msg[256];
	char msgid[16];
	char id[16];
	char y[16];
	char state[16];
	char hp[16];
	char sockfd[16];
	int psockfd=*(int*)arg;
	int n;
	int j;
	int k=0;

	while(1){
		memset(msg, 0, 256);
		memset(name, 0 ,20);
		memset(msgid ,0, 16);
		memset(id ,0, 16);
		memset(y ,0, 16);
		memset(state ,0, 16);
		memset(hp ,0, 16);
		memset(sockfd ,0, 16);
		n=read(psockfd, msg, sizeof(msg));
		if(n <= 0){
			continue;
		}
		sscanf(msg, "%s", msgid);
		if(strcmp(msgid, "26")==0){
			
		}else if(strcmp(msgid, "27")==0){
			/*hp update*/
			sscanf(msg,"%s %s %s", msgid, id, hp);
			int idd=atoi(id);
			for(j=0; j < MAX_EVENTS; j++){
				if(npcs[j].id == idd){
					npcs[j].hp=atoi(hp);
					if(npcs[j].hp<0) npcs[j].hp=0;
					break;
				}
			}
			clearmoving(&npcs[j]);
			moving(&npcs[j]);
			move(LINES-1, COLS-1);
			refresh();
			int l;
			for(l=0; l< MAX_EVENTS;l++){
				if(npcs[l].id != -1){
					clearmoving(&npcs[l]);
					moving(&npcs[l]);
					move(LINES-1, COLS-1);
					refresh();
				}
			}	
		}else if(strcmp(msgid, "28")==0){
			/*user died*/
			sscanf(msg,"%s %s", msgid, id);
			int idd=atoi(id);
                        for(j=0; j < MAX_EVENTS; j++){
                                if(npcs[j].id == idd){
                                        npcs[j].id=-1;
					npcs[j].isalive=0;
					break;
                                }
                        }
			clearmoving(&npcs[j]);
			move(LINES-1,COLS-1);
			refresh();
                }else if(strcmp(msgid, "29")==0){
			/*new user*/
			sscanf(msg, "%s %s %s %s %s %s %s", msgid, name, id, y, state, hp, sockfd);
			for(j=0; j < MAX_EVENTS; j++){
				if(npcs[j].id==-1){
					strcpy(npcs[j].name,name);
					npcs[j].id=atoi(id);
					npcs[j].isalive=1;
					npcs[j].x=LINES*2/3-1;
					npcs[j].y=atoi(y);
					npcs[j].state=atoi(state);
					npcs[j].hp=atoi(hp);
					npcs[j].sockfd=atoi(sockfd);
					break;	
				}
			}
			if(j != MAX_EVENTS){
				moving(&npcs[j]);
				move(LINES-1,COLS-1);
				refresh();
			}
                }else if(strcmp(msgid, "30")==0){
			/*game over*/
                }else if(strcmp(msgid, "31")==0){
			/*moving*/
			sscanf(msg, "%s %s %s", msgid, id, y);
			int idd=atoi(id);
			for(j=0; j < MAX_EVENTS; j++){
                                if(npcs[j].id==idd){
					clearmoving(&npcs[j]);
					move(LINES-1,COLS-1);
					refresh();
                                        npcs[j].y=atoi(y);
					npcs[j].state=1;
                                        break;
                                }
                        }
			if(j != MAX_EVENTS){
                                moving(&npcs[j]);
				move(LINES-1,COLS-1);
				refresh();
                        }
                }else if(strcmp(msgid, "32")==0){
			sscanf(msg, "%s %s %s", msgid, id, y);
                        int idd=atoi(id);
                        for(j=0; j < MAX_EVENTS; j++){
                                if(npcs[j].id==idd){
                                        clearmoving(&npcs[j]);
					move(LINES-1,COLS-1);
					refresh();
					npcs[j].state=atoi(y)+1;
					moving(&npcs[j]);
					move(LINES-1,COLS-1);
	                                refresh();
                                        break;
                                }
                        }
                }else if(strcmp(msgid, "33")==0){
			sscanf(msg, "%s %s %s %s %s %s %s", msgid, name, id, y, state, hp, sockfd);
                        for(j=0; j < MAX_EVENTS; j++){
                                if(npcs[j].id==-1){
                                        strcpy(npcs[j].name,name);
                                        npcs[j].id=atoi(id);
                                        npcs[j].isalive=1;
                                        npcs[j].x=LINES*2/3-1;
                                        npcs[j].y=atoi(y);
                                        npcs[j].state=atoi(state);
                                        npcs[j].hp=atoi(hp);
                                        npcs[j].sockfd=atoi(sockfd);
					moving(&npcs[j]);
					move(LINES-1,COLS-1);
                                	refresh();
                                        break;
                                }
                        }
                }else if(strcmp(msgid, "34")==0){
                        sscanf(msg, "%s %s %s", msgid, id, y);
                        int idd=atoi(id);
                        for(j=0; j < MAX_EVENTS; j++){
                                if(npcs[j].id==idd){
                                        clearmoving(&npcs[j]);
					move(LINES-1,COLS-1);
					refresh();
                                        npcs[j].state=atoi(y)+3;
					moving(&npcs[j]);
					move(LINES-1,COLS-1);
                                	refresh();
                                        break;
                                }
                        }
                }






	}
}

void moving(struct character* npc){
	if(npc->state==1){
		/*stand*/
		move(npc->x,npc->y-1);
        	addstr("|");
	        move(npc->x,npc->y+1);
        	addstr("|");
	        move(npc->x-1,npc->y-1);
        	addstr("|");
	        move(npc->x-1,npc->y+1);
        	addstr("|");
	        move(npc->x-2,npc->y);
        	addstr("*");
	        move(npc->x-3,npc->y);
        	addstr("*");
	        move(npc->x-3,npc->y-1);
        	addstr("-");
	        move(npc->x-3,npc->y+1);
        	addstr("-");
	        move(npc->x-4,npc->y);
        	addstr("@");
		move(npc->x-5,npc->y);
		char buf[2]={0};
		sprintf(buf,"%d",npc->hp);
                addstr(buf);
	}else if(npc->state==2){
		/*defence left*/
		move(npc->x,npc->y-1);
                addstr("|");
                move(npc->x,npc->y+1);
                addstr("|");
                move(npc->x-1,npc->y-1);
                addstr("|");
                move(npc->x-1,npc->y+1);
                addstr("|");
                move(npc->x-2,npc->y);
                addstr("*");
                move(npc->x-3,npc->y);
                addstr("*");
                move(npc->x-3,npc->y-1);
                addstr("|");
                move(npc->x-3,npc->y+1);
                addstr("-");
                move(npc->x-4,npc->y);
                addstr("@");
		move(npc->x-5,npc->y);
                char buf[2]={0};
                sprintf(buf,"%d",npc->hp);
                addstr(buf);
	}else if(npc->state==3){
		/*defence right*/
		move(npc->x,npc->y-1);
                addstr("|");
                move(npc->x,npc->y+1);
                addstr("|");
                move(npc->x-1,npc->y-1);
                addstr("|");
                move(npc->x-1,npc->y+1);
                addstr("|");
                move(npc->x-2,npc->y);
                addstr("*");
                move(npc->x-3,npc->y);
                addstr("*");
                move(npc->x-3,npc->y-1);
                addstr("-");
                move(npc->x-3,npc->y+1);
                addstr("|");
                move(npc->x-4,npc->y);
                addstr("@");
		move(npc->x-5,npc->y);
                char buf[2]={0};
                sprintf(buf,"%d",npc->hp);
                addstr(buf);
        }else if(npc->state==4){
		/*attack left*/
		move(npc->x-1,npc->y-1);
                addstr("-");
                move(npc->x,npc->y+1);
                addstr("|");
                move(npc->x-1,npc->y-2);
                addstr("-");
                move(npc->x-1,npc->y-3);
                addstr("-");
                move(npc->x-1,npc->y-4);
                addstr("-");
                move(npc->x-1,npc->y+1);
                addstr("|");
                move(npc->x-2,npc->y);
                addstr("*");
                move(npc->x-3,npc->y);
                addstr("*");
                move(npc->x-3,npc->y-1);
                addstr("-");
                move(npc->x-3,npc->y+1);
                addstr("-");
                move(npc->x-4,npc->y);
                addstr("@");
		move(npc->x-5,npc->y);
                char buf[2]={0};
                sprintf(buf,"%d",npc->hp);
                addstr(buf);
        }else if(npc->state==5){
		/*attack right*/
		move(npc->x,npc->y-1);
                addstr("|");
                move(npc->x-1,npc->y+1);
                addstr("-");
                move(npc->x-1,npc->y-1);
                addstr("|");
                move(npc->x-1,npc->y+2);
                addstr("-");
                move(npc->x-1,npc->y+3);
                addstr("-");
                move(npc->x-1,npc->y+4);
                addstr("-");
                move(npc->x-2,npc->y);
                addstr("*");
                move(npc->x-3,npc->y);
                addstr("*");
                move(npc->x-3,npc->y-1);
                addstr("-");
                move(npc->x-3,npc->y+1);
                addstr("-");
                move(npc->x-4,npc->y);
                addstr("@");
		move(npc->x-5,npc->y);
                char buf[2]={0};
                sprintf(buf,"%d",npc->hp);
                addstr(buf);
        }

}



void clearmoving(struct character *npc){
	if(npc->state==1||npc->state==2||npc->state==3){
		move(npc->x,npc->y-1);
	        addstr(BLANK);
        	move(npc->x,npc->y+1);
	        addstr(BLANK);
        	move(npc->x-1,npc->y-1);
	        addstr(BLANK);
        	move(npc->x-1,npc->y+1);
	        addstr(BLANK);
        	move(npc->x-2,npc->y);
	        addstr(BLANK);
        	move(npc->x-3,npc->y);
	        addstr(BLANK);
        	move(npc->x-3,npc->y-1);
	        addstr(BLANK);
        	move(npc->x-3,npc->y+1);
	        addstr(BLANK);
        	move(npc->x-4,npc->y);
	        addstr(BLANK);
		move(npc->x-5,npc->y);
                addstr(BLANK);
	}else if(npc->state==4){
		move(npc->x-1,npc->y-1);
        	addstr(BLANK);
	        move(npc->x,npc->y+1);
        	addstr(BLANK);
        	move(npc->x-1,npc->y-2);
	        addstr(BLANK);
        	move(npc->x-1,npc->y-3);
	        addstr(BLANK);
        	move(npc->x-1,npc->y-4);
	        addstr(BLANK);
        	move(npc->x-1,npc->y+1);
	        addstr(BLANK);
        	move(npc->x-2,npc->y);
	        addstr(BLANK);
        	move(npc->x-3,npc->y);
	        addstr(BLANK);
        	move(npc->x-3,npc->y-1);
        	addstr(BLANK);
	        move(npc->x-3,npc->y+1);
        	addstr(BLANK);
	        move(npc->x-4,npc->y);
        	addstr(BLANK);
		move(npc->x-5,npc->y);
                addstr(BLANK);
	}else if(npc->state==5){
		move(npc->x,npc->y-1);
        	addstr(BLANK);
        	move(npc->x-1,npc->y+1);
	        addstr(BLANK);
        	move(npc->x-1,npc->y-1);
	        addstr(BLANK);
        	move(npc->x-1,npc->y+2);
	        addstr(BLANK);
        	move(npc->x-1,npc->y+3);
	        addstr(BLANK);
        	move(npc->x-1,npc->y+4);
	        addstr(BLANK);
        	move(npc->x-2,npc->y);
	        addstr(BLANK);
        	move(npc->x-3,npc->y);
	        addstr(BLANK);
        	move(npc->x-3,npc->y-1);
	        addstr(BLANK);
        	move(npc->x-3,npc->y+1);
	        addstr(BLANK);
        	move(npc->x-4,npc->y);
	        addstr(BLANK);
		move(npc->x-5,npc->y);
                addstr(BLANK);
	}
}
