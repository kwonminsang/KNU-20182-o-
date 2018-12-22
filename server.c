#include <stdio.h>

#include <stdio.h>

#include <unistd.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>

#include <string.h>

#include <sys/select.h>

#include <sys/time.h>

#include <signal.h>

#include <time.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>

#include <math.h>

 

#define MSGLEN 1024

#define MAXCLI 50

#define MAXQUEST 10

 

/*********************************

전역변수 영역

**********************************/

int quiz_flag=0, quiz_index=0, q_disp=0, alarm_flg=0;

//사용자 별명

char nick_list[MAXCLI][20];

int client_fd[MAXCLI];

 

/*********************************

퀴즈용 문제 및 답 열기

*********************************/

void open_quiz(char quest[][200] , char answer[][200]){

    FILE* q = fopen("quest","rt");

    FILE* a = fopen("answer","rt");

 

    if(!q || !a){

        fprintf(stderr, "퀴즈 파일이 없습니다.\n");

        exit(0);

    }

 

    int i=0,j=0;

    memset(quest, '\0', MAXQUEST*200);

    memset(answer, '\0', MAXQUEST*200);

    while (fgets(quest[i], 200, q) != NULL) {

        i++;

    }

    while (fgets(answer[j], 200, a) != NULL) {

        j++;

    }

    fclose(q);

    fclose(a);

}

 

/*********************************

클라이언트들에게 메시지 뿌리기

1. 클라이언트 목록이 있는 배열

2. 전달할 메시지

3. 메시지를 입력한 클라이언트의 배열 번호

**********************************/

void print_clients(char* msg, int current_client){

    int j,length;

    for(j=0;j<MAXCLI;j++){

        if(client_fd[j] != -1 && j!=current_client){

            //length=send(client_fd[j], msg, strlen(msg)+1, MSG_DONTWAIT);

            fsync(client_fd[j]);

            length=write(client_fd[j], msg, strlen(msg)+1);

            sync();

            printf("클라이언트 %d에게 [%s] 발송, 길이:%d\n",j, msg, length);

        }

    }

}

 

/*********************************

기본시간초과 핸들러

**********************************/

void alarm_handler(int sig){

    print_clients("20초 시간 초과~!\n",-1);

    quiz_flag = 0;

    quiz_index = 0;

    q_disp = 0;

    alarm_flg=0;

    print_clients("::::퀴즈가 종료되었습니다.:::::\n", -1);

}

 

/*********************************

강제시간초과 핸들러: 출력작업 안 함.

**********************************/

void alarm_handler2(int sig){

    quiz_flag = 0;

    quiz_index = 0;

    q_disp = 0;

    alarm_flg=0;

}

 

 

/*********************************

특정 문자 치환 함수

입력 문자열, 제거할 문자, 바꿀 문자

*********************************/

void replaceAChar(char * str, char o, char r){

    int i=0;

    while(str[i] != '\0'){

        if (str[i] == o){

            str[i] = r;

        }

        i++;

    }

}

 

/********************************

특정문자 카운트 함수

매개변수: 문자열

반환값: 개수

********************************/

int countChar(char* str, char c){

    int i=0, count=0;

    while(str[i] != '\0'){

        if(str[i]==c) count++;

        i++;

    }

    return count;

}

 

int main(int argc, char *argv[]){

    int sockfd, confd;

    struct sockaddr_in cliaddr, servaddr;

    int addrlen;

    fd_set read_fdset;

 

    int client_num = 0;

    char buf[MSGLEN], buf2[MSGLEN], buf3[MSGLEN];//클라이언트 메시지 수신용 버퍼와 명령어 인식을 위해 두는 사본 버퍼

    char msg[MSGLEN+20];

    int  msg_len,maxfdp1, nready=0, i;

 

    //퀴즈데이터 변수

    char quest[MAXQUEST][200], answer[MAXQUEST][200];

 

    //퀴즈 데이터 열기

    open_quiz(quest, answer);

 

    if(argc != 2){

        printf("사용법: ./실행파일명 [포트번호] \n");

        exit(0);

    }

 

    if((sockfd = socket(AF_INET, SOCK_STREAM,0))<0){

        printf("server: can't create socket\n");

        exit(-1);

    }

 

    servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//wild card

    servaddr.sin_port = htons(atoi(argv[1]));

 

    if((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0){

        printf("server: bind error\n");

        exit(-1);

    }

 

    listen(sockfd, 5);

 

    for(i=0; i<MAXCLI; i++){

        client_fd[i] = -1;

    }

 

    FD_ZERO(&read_fdset);

    maxfdp1 = sockfd + 1;

 

    while(1){

        FD_SET(sockfd, &read_fdset);

            //for(i=0;i<MAXCLI;i++)printf("%d, ",client_fd[i]);

    //printf("\n");

        //소켓이 살아있는 클라이언트 소켓을 비트마스크에 넣는다.

        for(i=0;i<MAXCLI;i++){

            if(client_fd[i] != -1){

                //printf("%d번 클라이언트가 살아있으므로 비트마스크에 삽입\n",i);

                FD_SET(client_fd[i], &read_fdset);

            }

        }

//printf("nready: %d \n", nready);

        if((nready = select(maxfdp1, &read_fdset, NULL, NULL, NULL)) < 0){

            //select 에러가 나면 다시 돌려봐

            //printf("select error...\n");

            continue;

        }

 

        if(FD_ISSET(sockfd, &read_fdset)){

            addrlen = sizeof(cliaddr);

            confd = accept(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);

            if(confd < 0){

                printf("server accept error\n");

                exit(-1);

            }

 

            if(client_num >= MAXCLI){

                printf("too many clients\n");

                close(confd);

                continue;

            }

            printf("새 접속자다! (confd=%d)...\n", confd);

 

            for(i=0;i<MAXCLI;i++){

                if(client_fd[i] != -1){continue;}

                client_fd[i] = confd;

 

                maxfdp1++;

                client_num++;

 

                break;

            }

            print_clients("새 접속자가 들어왔어요!!\n", -1);

        }

        for(i=0;i<MAXCLI;i++){

            if(client_fd[i] == -1){

                continue;

            }

            if(FD_ISSET(client_fd[i], &read_fdset)){

                memset(buf, '\0', sizeof(buf));

                memset(buf2, '\0', sizeof(buf));

                memset(buf3, '\0', sizeof(buf));

 

                //클라이언트로부터 메시지 수신

                msg_len=read(client_fd[i], buf,MSGLEN);

                //수신된 메시지에서 줄바꿈문자 제거

                replaceAChar(buf, '\n', ' ');

                strcpy(buf2, buf);

                if(buf[0] != 0 && strcmp(buf, "!종료 ")!=0){

                    printf("%d, client_num: %d, nready: %d, %d 번째: %s\n",countChar(buf,' '), client_num, nready, i, buf);

                }else{

                    if(buf[0] == 0) printf("클라이언트 %d가 강제종료!\n", i);

                    printf("%d번 종료\n",i);

                    //퇴실한 손님은 소켓을 닫아줌

                    FD_CLR(client_fd[i], &read_fdset);

                    close(client_fd[i]);

                    client_fd[i] = -1;

                    maxfdp1--;

                    client_num--;

 

                    //종료하였으므로 다른 클라이언트들에게 통보

                    memset(msg, '\0',MSGLEN+20);

                    if(nick_list[i][0]=='['){

                        sprintf(msg,"%s님이 퇴실하였습니다.\n", nick_list[i]);

                    }else{

                        sprintf(msg,"%d번 손님이 퇴실하였습니다.\n",i);

                    }

                    print_clients(msg, -1);

                    //퇴실한 클라이언트의 닉네임, 버퍼도 초기화

                    memset(nick_list[i], '\0', 20);

                    memset(buf, '\0', sizeof(buf));

                    memset(buf2, '\0', sizeof(buf));

                    memset(buf3, '\0', sizeof(buf));

                    continue;

                    //nready--;

                }

 

                //의미없는 입력에 대한 처리

                if(countChar(buf,' ') == strlen(buf)){

                    printf("그냥 엔터 또는 공백\n");

                    continue;

                }

 

                //한명의 사용자가 입력한 메시지를 접속한 사람들에게 뿌리는 부분

                //여기에서 명령어에 대해 처리함

                memset(msg, '\0',MSGLEN+20);

                strcpy(buf3,strtok(buf2," "));

                if(strcmp(buf,"!인원 ")==0){

                    if(nick_list[i][0]=='['){

                        sprintf(msg,"%s %s\n", nick_list[i], buf);

                    }else{

                        sprintf(msg,"%d번:%s\n", i, buf);

                    }

                    print_clients( msg, i);

                    memset(msg, '\0',MSGLEN+20);

                    sprintf(msg,"::::현재접속자:%d명::::\n", client_num);

                    print_clients(msg, -1);

                }else if(strcmp(buf,"!퀴즈시작 ")==0){

                    if(alarm_flg == 0){

                        if (signal(SIGALRM, alarm_handler) ==SIG_ERR){

                            fprintf(stdout, "핸들링 에러!\n");

                        }

                        alarm_flg=1;

                        alarm(20);

                    }

                    //퀴즈 카운트가 이0보다크면 퀴즈가 진행중이다.

                    if(quiz_flag==0){

                        quiz_flag = 1; 
                        srand(time(NULL));

                        quiz_index = random() % 8;

			
			
                        print_clients("::::굿모팝스퀴즈 시작!:::::\n", -1);

                    }else{

                        print_clients("퀴즈가 아직 진행중입니다.\n", -1);

                        q_disp=0;

                    }

                }else if(strcmp(buf,"!퀴즈종료 ")==0){

                    printf(">> 퀴즈종료 진입 %s, flg:%d\n", buf, alarm_flg);

                    if(alarm_flg==1){

                        alarm_flg=0;

                        printf(">>퀴즈 알람종료 \n");

                        if (signal(SIGALRM, alarm_handler2) ==SIG_ERR){

                            fprintf(stdout, "핸들링 에러!\n");

                        }

                        alarm(0);

                    }

                     if(quiz_flag==1){

                        quiz_flag = 0;

                        quiz_index = 0;

                        q_disp = 0;

                        print_clients("::::퀴즈가 종료되었습니다.:::::\n", -1);

                     }

                }else if(strcmp(buf3,"!닉")==0){

                    sprintf(nick_list[i], "[%s", strtok(NULL," "));

                    nick_list[i][strlen(nick_list[i])]=']';//줄바꿈문자를 ]로 대체

                    sprintf(msg,"%d번:%s\n", i, buf);

                    print_clients(msg, i);

                    memset(msg, '\0',MSGLEN+20);

                     sprintf(msg,":::: %d의 변경된 닉네임:%s ::::\n",i, nick_list[i]);

                    print_clients( msg, -1);

                    printf("%d번:%s\n변경된 닉네임:%s", i, buf,nick_list[i]);

                }else{

                    //닉네임이 있으면 닉네임으로 출력

                    if(nick_list[i][0]=='['){

                        sprintf(msg,"%s %s\n", nick_list[i], buf);

                        print_clients(msg, i);

                    }else{

                        sprintf(msg,"%d번:%s\n", i, buf);

                        print_clients(msg, i);

                    }

                }

 

                //퀴즈가 진행중일경우에 대한 처리

                if(quiz_flag==1){

                    memset(buf2, '\0', sizeof(buf));

                    strcpy(buf2, buf);

                    buf2[strlen(buf2)-1] = '\n';

                    printf(">>%d 퀴즈 플래그 (%s) \n", quiz_flag,buf2);

                    if(q_disp==0){

                        print_clients( ":::::::문제::::::\n", -1);

                        print_clients(quest[quiz_index], -1);

                        printf(">>퀴즈인덱스 : %d\n", quiz_index);

                        printf(">>>>%s<<<<\n", quest[quiz_index]);

                        q_disp = 1;

                    }

                        if(strcmp(buf2, answer[quiz_index])==0){

                            memset(msg, '\0',MSGLEN+20);

                            if(nick_list[i][0]=='['){

                                sprintf(msg,"%s님, 정답입니다 ^^\n", nick_list[i]);

                                print_clients(msg, -1);

                            }else{

                                sprintf(msg,"%d번 손님, 정답입니다 ^^\n", i);

                                print_clients(msg, -1);

                            }

                            quiz_flag = 0;

                            quiz_index = 0;

                            if(alarm_flg==1){

                                printf(">>퀴즈 알람종료 \n");

                                if (signal(SIGALRM, alarm_handler2) ==SIG_ERR){

                                    fprintf(stdout, "핸들링 에러!\n");

                                }

                                alarm_flg = 0;

                                alarm(0);

                            }

                        }

                }

            }

        }//End Of While

    }

}
