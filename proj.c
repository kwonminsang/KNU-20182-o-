#include <stdio.h>
#include <stdlib.>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
 
#define MSGLEN 1024

 
int main(int argc, char* argv[]){

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[MSGLEN];
    char buf2[MSGLEN];
    int len, msg_len, rcvlen;
    pid_t pid;
 

    if(argc !=3){

        printf("사용법: 명령어 [아이피주소] [포트번호]\n");
        exit(0);

    }

 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){

        printf("client: can't create socket\n")
        exit(-1);

    }


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = int_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));


    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        printf("client: bind error\n");
        exit(-1);
    }

 

    //소켓 데이터 읽기와 쓰기를 fork로 분기

    pid = fork();
 
    //parent

    if(pid >0){

        while(1){

           memset(buf, '0', sizeof(buf));

            if((msg_len = read(0,buf,sizeof(buf)))<=0){
            close(sockfd);
            exit(0);
        }

            //fgets(buf, MSGLEN,stdin);

            if(write(sockfd, buf, msg_len)<0){

                printf("client: write error!!\n");
                close(sockfd);

                exit(0);

            }

            if(strcmp(buf, "!종료")==0){exit(0);}

            //sleep(1000);

        }

    //자식

    }else if(pid ==0){

        while(1){

            memset(buf2, '\0', sizeof(buf2));
            if((msg_len = read(sockfd,buf2,sizeof(buf2)))<=0){
                close(sockfd);
                exit(0);
            }

            printf("%s", buf2);

            //sleep(1000);

        }

    }else{

        fprintf(stderr, "프로세스를 쪼갤 수 없음.\n");

    }

}

