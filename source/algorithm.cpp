#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <QDebug>
#include "algorithm.h"

#define citynum 10 //城市个数
#define nan 100000000 //无穷大

//全局变量定义
enum TraficType{car=2, train=5, plane=9};//不同交通工具的单位风险值
float TotalRisk = nan;//行程总风险值
int flag[citynum] = {0};//表示该city是否被遍历过
int length = 0;//临时参数，表示经历了几次换乘
int TotalLength = 0;//表示最终的最优路线经历了几次换乘
int time1 = 0;//临时参数，表示从开始到现在经历了多少时间
int TotalTime = 0;//表示最终的路线经历的时间
int timelimit = 100;
FILE *fp3;//指向输出日志文件
struct City city[citynum];//存储城市信息的结构体数组
struct Plan plan[citynum];//存储行程信息的结构体数组

//函数实现
int CityNum(char *p)//将城市英文名字转化为对应序号
{
    int i=0;
    while(strcmp(city[i].name, p) != 0)
        i++;
    return i;
}

int sub(int a, int b)//若a>=b,返回a-b，否则返回a-b+24
{
    if(a >= b)
        return(a-b);
    else return(a-b+24);
}

void Init()//将城市信息和各城市的交通表从文件读入结构体
{
    int i;
    FILE *fp1, *fp2;
    int starttime, arrivetime, startcity, arrivecity;
    char temp0[10], temp1[10], temp2[10], temp3[10];
    Route *p, *q;

    if((fp3 = fopen("journal.txt", "w+")) == NULL)
    {
        printf("Can not open journal.txt");
        exit(0);
    }
    if((fp1 = fopen("city.txt", "r")) == NULL)
    {
        printf("Can not open city.txt");
        exit(0);
    }
    if((fp2 = fopen("routetable.txt", "r")) == NULL)
    {
        printf("Can not open routetable.txt");
        exit(0);
    }
    i = 0;
    while(fscanf(fp1, "%s %f %d %d", city[i].name, &(city[i].risk), &(city[i].x), &(city[i].y)) != EOF)
    {
        city[i].FirstRoute = NULL;
        i++;
    }
    while(fscanf(fp2, "%s %s %s %d %d %s", temp0, temp1, temp2, &starttime, &arrivetime, temp3) != EOF)
    {
        startcity = CityNum(temp1);
        arrivecity = CityNum(temp2);
        q = (Route*)malloc(sizeof(Route));
        q->ArriveCity = arrivecity;
        q->ArriveTime = arrivetime;
        q->StartTime = starttime;
        strcpy(q->num, temp3);
        if(temp0[0] == 'p')
            q->risk = plane;
        else if(temp0[0] == 'c')
            q->risk = car;
        else
            q->risk = train;
        q->nextRoute = NULL;

        p = city[startcity].FirstRoute;
        if(p == NULL)
            city[startcity].FirstRoute = q;
        else
        {
            while(p->nextRoute != NULL)
                p = p->nextRoute;
            p->nextRoute = q;
        }
    }
    fclose(fp1);
    fclose(fp2);

}

int dfs(int sc, int st, int ac, float risk)//寻找满足条件的最优路径
//返回值为1代表找到更优路径，为0则代表没找到
{
    Route *p;
    float temp;
    int arrivecity, starttime, arrivetime;
    int better = 0, result = 0;

    length++;
    flag[sc] = 1;//flag[city]=1表示该city已经被遍历过
    p = city[sc].FirstRoute;
    while(p)
    {
        arrivecity = p->ArriveCity;
        if(flag[arrivecity] == 1)
        {
            p = p->nextRoute;
            continue;
        }
        arrivetime = p->ArriveTime;
        starttime = p->StartTime;
        temp = sub(starttime, st)*(city[sc].risk) + sub(arrivetime, starttime)*(p->risk)*(city[sc].risk);
        time1 += sub(starttime, st) + sub(arrivetime, starttime);

        if(arrivecity == ac && risk + temp <= TotalRisk && time1 <= timelimit)//到达终点，且为更优路径
        {
            TotalLength = length;
            TotalTime = time1;
            TotalRisk = risk+temp;
            better = 1;
            plan[length-1].ArriveCity = ac;
            plan[length-1].ArriveTime = arrivetime;
            plan[length-1].StartCity = sc;
            plan[length-1].StartTime = starttime;
            strcpy(plan[length-1].num, p->num);
            if(p->risk == car)
                strcpy(plan[length-1].type, "car");
            else if(p->risk == plane)
                strcpy(plan[length-1].type, "plane");
            else
                strcpy(plan[length-1].type, "train");
        }
        else if(arrivecity != ac)//未到达终点
        {
            result = dfs(arrivecity, arrivetime, ac, risk+temp);//继续搜索下一条路径
            if(result)//如果找到了更优路径，说明本段为更优路径，更新plan
            {
                better = 1;
                plan[length-1].ArriveCity = arrivecity;
                plan[length-1].ArriveTime = arrivetime;
                plan[length-1].StartCity = sc;
                plan[length-1].StartTime = starttime;
                strcpy(plan[length-1].num, p->num);
                if(p->risk == car)
                    strcpy(plan[length-1].type, "car");
                else if(p->risk == plane)
                    strcpy(plan[length-1].type, "plane");
                else
                    strcpy(plan[length-1].type, "train");
            }
        }
        time1 -= sub(starttime, st) + sub(arrivetime, starttime);
        p = p->nextRoute;
    }
    flag[sc] = 0;
    length--;

    return better;
}

int PassengerState(int state, int time, int *len, int day)//更新旅客状态，并将旅行状态写入行程日志文件
{
    printf("day%d %d:00 ", day, time);
    fprintf(fp3, "day%d %d:00 ", day, time);
    if(state == 0 && plan[*len].StartTime != time)
    {
        printf("Waiting in %s\n", city[plan[*len].StartCity].name);
        fprintf(fp3, "Waiting in %s\n", city[plan[*len].StartCity].name);
    }

    else if((state == 0 && time == plan[*len].StartTime) || (state == 1 && plan[*len].ArriveTime != time))
    {
        printf("%s ==> ", city[plan[*len].StartCity].name);
        printf("%s by %s", city[plan[*len].ArriveCity].name, plan[*len].type);
        printf(" %s\n", plan[*len].num);
        fprintf(fp3, "%s ==> ", city[plan[*len].StartCity].name);
        fprintf(fp3, "%s by %s", city[plan[*len].ArriveCity].name, plan[*len].type);
        fprintf(fp3, " %s\n", plan[*len].num);
        state = 1;
    }
    else if(state == 1 && time == plan[*len].ArriveTime)
    {
        printf("Arrive at %s\n", city[plan[*len].ArriveCity].name);
        fprintf(fp3, "Arrive at %s\n", city[plan[*len].ArriveCity].name);
        state = 2;
        if((*len)+1 < TotalLength && time == plan[(*len)+1].StartTime)
        {
            (*len)++;
            printf("                 %s ==> ", city[plan[*len].StartCity].name);
            printf("%s by %s", city[plan[*len].ArriveCity].name, plan[*len].type);
            printf(" %s\n", plan[*len].num);
            fprintf(fp3, "                 %s ==> ", city[plan[*len].StartCity].name);
            fprintf(fp3, "%s by %s", city[plan[*len].ArriveCity].name, plan[*len].type);
            fprintf(fp3, " %s\n", plan[*len].num);
            state = 1;
        }
    }

    return state;
}

