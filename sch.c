#define _GNU_SOURCE
#include<linux/kernel.h>
#include<linux/module.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h> 
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>

static long long int *count;
static int *remain;
int main(){
    
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);

    char sp[10];   //Scheduling Policy.
    char *fifo="FIFO",*sjf="SJF",*psjf="PSJF",*rr="RR";
    int n;      //Number of Process
    count = mmap(NULL,sizeof *count,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    remain = mmap(NULL,sizeof(int)*15,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    *count = 0;
    scanf("%s",sp);
    scanf("%d",&n);


    char pn[15][10]={""};  //Process Name
    int r[15],t[15];    //Ready time, Exec time
    int Q[15],qcount = 0,p_in_q = 0;  //Queue,process in queue
    pid_t Qpid[15];

    for(int i=0;i<n;i++){
        scanf("%s %d %d",pn[i],&r[i],&t[i]);
    }

    long double timeunit;
    struct timespec begin,test;
    clock_gettime(CLOCK_REALTIME, &begin);
    { volatile unsigned long a; for(a=0;a<1000000UL;a++); *count += 1;} 
    clock_gettime(CLOCK_REALTIME, &test);
    timeunit = (long double)(test.tv_nsec - begin.tv_nsec)*(0.000000001);

    if(strcmp(sp,fifo)==0){
        nice(0);
        clock_gettime(CLOCK_REALTIME, &begin);
        for(int i=0;i<n;i++){
            pid_t pid;
            while(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) < r[i]*timeunit){
                clock_gettime(CLOCK_REALTIME, &test);
            }
            pid = vfork();
            if(pid == 0){
                struct timespec start,end;
                unsigned long sec,nsec;
                long stime,etime;
                stime = syscall(352);
                clock_gettime(CLOCK_REALTIME, &start);
                clock_gettime(CLOCK_REALTIME, &end);
                while(((long double)(end.tv_sec-start.tv_sec)+(long double)(end.tv_nsec-start.tv_nsec)*(0.000000001)) < (long double)t[i]*timeunit){
                    clock_gettime(CLOCK_REALTIME, &end);
                }
                clock_gettime(CLOCK_REALTIME, &end);
                etime = syscall(352);
                int mypid = getpid();
                printf("%s %d\n",pn[i],mypid);
                syscall(351,mypid,stime,etime);
                exit(0);
            }
        }
    }
    else if (strcmp(sp,sjf)==0){
        nice(0);
        clock_gettime(CLOCK_REALTIME, &begin);
        for(int i=0;i<n;i++){
            clock_gettime(CLOCK_REALTIME, &test);
            if(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) > r[i]*timeunit){
                Q[qcount++]=i;
                p_in_q++;
                continue;
            }
            else{
                if(p_in_q > 0){
                    int mint = __INT_MAX__;
                    int minp = -1;
                    for(int j=0;j<qcount;j++){
                        if(t[j]<mint){
                            mint = t[j];
                            minp = j;
                        }
                    }
                    pid_t pid;
                    p_in_q --;
                    pid = vfork();
                    if(pid == 0){
                        struct timespec start,end;
                        long stime,etime;
                        stime = syscall(352);
                        clock_gettime(CLOCK_REALTIME, &start);
                        clock_gettime(CLOCK_REALTIME, &end);
                        while(((long double)(end.tv_sec-start.tv_sec)+(long double)(end.tv_nsec-start.tv_nsec)*(0.000000001)) < (long double)t[minp]*timeunit){
                            clock_gettime(CLOCK_REALTIME, &end);
                        }
                        clock_gettime(CLOCK_REALTIME, &end);
                        etime = syscall(352);
                        int mypid = getpid();
                        printf("%s %d\n",pn[i],mypid);
                        syscall(351,mypid,stime,etime);
                        exit(0);
                    }
                    t[minp]=__INT_MAX__;
                    i--;
                }
                else{
                    while(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) < r[i]*timeunit){
                        clock_gettime(CLOCK_REALTIME, &test);
                    }
                    i--;
                }
            }   
        }
        while(p_in_q > 0){
            int mint = __INT_MAX__;
            int minp = -1;
            for(int j=0;j<qcount;j++){
                if(t[j]<mint){
                    mint = t[j];
                    minp = j;
                }
            }
            pid_t pid;
            p_in_q --;
            pid = vfork();
            if(pid == 0){
                struct timespec start,end;
                clock_gettime(CLOCK_REALTIME, &start);
                clock_gettime(CLOCK_REALTIME, &end);
                long stime,etime;
                stime = syscall(352);
                while(((long double)(end.tv_sec-start.tv_sec)+(long double)(end.tv_nsec-start.tv_nsec)*(0.000000001)) < (long double)t[minp]*timeunit){
                    clock_gettime(CLOCK_REALTIME, &end);
                }
                clock_gettime(CLOCK_REALTIME, &end);
                etime = syscall(352);
                int mypid = getpid();
                syscall(351,mypid,stime,etime);
                exit(0);
            }
            t[minp]=__INT_MAX__;
        }
    }
    else if (strcmp(sp,rr)==0){
        int nowp = -1;
        struct sched_param param,tpar;
        param.sched_priority = 50;
        int wrong = sched_setscheduler(0,SCHED_RR,&param);
	    if(wrong < 0 )printf("error:%s\n",strerror(errno));
	    sched_getparam(0,&tpar);
        clock_gettime(CLOCK_REALTIME,&begin);
        clock_gettime(CLOCK_REALTIME,&test);
        for(int i=0;i<n;i++){Qpid[i]=-1;}
        for(int i=0;i<n;i++){
            clock_gettime(CLOCK_REALTIME,&test);
            while(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) < r[i]*timeunit && p_in_q == 0){
                clock_gettime(CLOCK_REALTIME,&test);
            }
            if(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) > r[i]*timeunit){
                Q[qcount++]=i;
                p_in_q++;
                Qpid[i] = fork();
                if(Qpid[i]==0){
            		sched_getparam(0,&tpar);
                    if(tpar.sched_priority==50){
                        param.sched_priority = 1;
                        sched_setscheduler(0,SCHED_FIFO,&param);
                    }
                    struct timespec start,end;
                    long stime,etime;
                    stime = syscall(352);
                    clock_gettime(CLOCK_REALTIME, &start);
                    clock_gettime(CLOCK_REALTIME, &end);
                    while(t[i]>0){
                        sched_getparam(0,&tpar);
                        struct timespec rrstart,rrend;
                        clock_gettime(CLOCK_REALTIME,&rrstart);
                        clock_gettime(CLOCK_REALTIME,&rrend);
                        if(t[i]>500){
                            t[i] -= 500;
                            while(((long double)(rrend.tv_sec-rrstart.tv_sec)+(long double)(rrend.tv_nsec-rrstart.tv_nsec)*(0.000000001)) < (long double)500*timeunit){
                                clock_gettime(CLOCK_REALTIME, &rrend);
                            }
                            param.sched_priority = 1;
                            sched_setscheduler(0,SCHED_FIFO,&param);
                        }
                        else{
                        sched_getparam(0,&tpar);
                            while(((long double)(rrend.tv_sec-rrstart.tv_sec)+(long double)(rrend.tv_nsec-rrstart.tv_nsec)*(0.000000001)) < (long double)t[i]*timeunit){
                                clock_gettime(CLOCK_REALTIME, &rrend);
                            }
                            clock_gettime(CLOCK_REALTIME,&end);
                            etime = syscall(352);
                            int mypid = getpid();
                            printf("%s %d\n",pn[i],mypid);
                            syscall(351,mypid,stime,etime);
                            exit(0);
                        }
                    }
                }
            }
            else if(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) < r[i]*timeunit && p_in_q > 0){
         
            int x = (nowp+1)%n;
            while(1){
                if(Qpid[x] > 0 && t[x]>0){
                    nowp = x;
                    break;
                }
                else x = (x+1)%n;
            }
                param.sched_priority = 99;
                sched_setscheduler(Qpid[nowp],SCHED_FIFO,&param);
                t[nowp] -= 500;
                if(t[nowp]<=0)p_in_q--;
                i--;
            }
        }
        while(p_in_q > 0){
            int x = (nowp+1)%n;
            while(1){
                if(Qpid[x] > 0 && t[x]>0){
                    nowp = x;
                    break;
                }
                else x = (x+1)%n;
            }
            param.sched_priority = 98;
            if(sched_setscheduler(0,SCHED_FIFO,&param)<0)printf("error:%s\n",strerror(errno));
            sched_getparam(0,&tpar);
            param.sched_priority = 99;
            int ppid = Qpid[nowp];
            if(sched_setscheduler(ppid,SCHED_FIFO,&param)<0)printf("error:%s\n",strerror(errno));
            t[nowp] -=500;
            if(t[nowp]<=0)p_in_q--;
        }
    }
    else if (strcmp(sp,psjf)==0){
        int nowp = -1;
        struct sched_param param,tpar;
        param.sched_priority = 50;
        int wrong = sched_setscheduler(0,SCHED_RR,&param);
	    if(wrong < 0 )printf("error:%s\n",strerror(errno));
	    sched_getparam(0,&tpar);
        clock_gettime(CLOCK_REALTIME,&begin);
        clock_gettime(CLOCK_REALTIME,&test);
        for(int i=0;i<n;i++){Qpid[i]=-1;}
        for(int i=0;i<n;i++){
            clock_gettime(CLOCK_REALTIME,&test);
            while(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) < r[i]*timeunit && p_in_q == 0){
                clock_gettime(CLOCK_REALTIME,&test);
            }
            if(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) > r[i]*timeunit){
                Q[qcount++]=i;
                *count = qcount;
                p_in_q++;
                Qpid[i] = fork();
                if(Qpid[i]==0){
            		sched_getparam(0,&tpar);
                    if(tpar.sched_priority==50){
                        param.sched_priority = 1;
                        sched_setscheduler(0,SCHED_RR,&param);
                    }
                    struct timespec start,end;
                    clock_gettime(CLOCK_REALTIME, &start);
                    clock_gettime(CLOCK_REALTIME, &end);
                    long stime,etime;
                    stime = syscall(352);
                    while(t[i]>0){
                        struct timespec rrstart,rrend;
                        clock_gettime(CLOCK_REALTIME,&rrstart);
                        clock_gettime(CLOCK_REALTIME,&rrend);
                        int nowunit = (rrstart.tv_sec-begin.tv_sec + (long double)(rrstart.tv_nsec-begin.tv_nsec)*(0.000000001))/timeunit;
                        if(*count < n && nowunit+t[i]>r[*count]+10){
                            t[i] -= (r[*count])-nowunit;
                            while(((long double)(rrend.tv_sec-rrstart.tv_sec)+(long double)(rrend.tv_nsec-rrstart.tv_nsec)*(0.000000001)) < (long double)((r[*count])-nowunit)*timeunit){
                                clock_gettime(CLOCK_REALTIME, &rrend);
                            }
                            param.sched_priority = 1;
                            sched_setscheduler(0,SCHED_FIFO,&param);
                        }
                        else{
                            while(((long double)(rrend.tv_sec-rrstart.tv_sec)+(long double)(rrend.tv_nsec-rrstart.tv_nsec)*(0.000000001)) < (long double)t[i]*timeunit){
                                clock_gettime(CLOCK_REALTIME, &rrend);
                            }
                            clock_gettime(CLOCK_REALTIME,&end);
                            etime = syscall(352);
                            int mypid = getpid();
                            printf("%s %d\n",pn[i],mypid);
                            syscall(351,mypid,stime,etime);
                            exit(0);
                        }
                    }
                }
            }
            else if(((long double)(test.tv_sec-begin.tv_sec)+(long double)(test.tv_nsec-begin.tv_nsec)*(0.000000001)) < r[i]*timeunit && p_in_q > 0){
                int minp = -1,mint=__INT_MAX__;
                for(int j=0;j<qcount;j++){
                    if(t[j]<mint && t[j]>0){
                        mint = t[j];
                        minp = j;
                    }
                }
                param.sched_priority = 99;
                sched_setscheduler(Qpid[minp],SCHED_FIFO,&param);
                t[minp] -= 100;
                if(t[minp]<=0)p_in_q--;
                i--;
            }
        }
        while(p_in_q > 0){
            int minp = -1,mint=__INT_MAX__;
            for(int j=0;j<qcount;j++){
                if(t[j]<mint && t[j]>0){
                    mint = t[j];
                    minp = j;
                }
            }
            param.sched_priority = 99;
            sched_setscheduler(Qpid[minp],SCHED_FIFO,&param);
            t[minp] -= 100;
            if(t[minp]<=0)p_in_q--;
        }
    }
    return 0;
}