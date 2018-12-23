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
