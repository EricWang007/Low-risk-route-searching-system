#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <QDebug>
#include "algorithm.h"

#define citynum 10 //���и���
#define nan 100000000 //�����

//ȫ�ֱ�������
enum TraficType{car=2, train=5, plane=9};//��ͬ��ͨ���ߵĵ�λ����ֵ
float TotalRisk = nan;//�г��ܷ���ֵ
int flag[citynum] = {0};//��ʾ��city�Ƿ񱻱�����
int length = 0;//��ʱ��������ʾ�����˼��λ���
int TotalLength = 0;//��ʾ���յ�����·�߾����˼��λ���
int time1 = 0;//��ʱ��������ʾ�ӿ�ʼ�����ھ����˶���ʱ��
int TotalTime = 0;//��ʾ���յ�·�߾�����ʱ��
int timelimit = 100;
FILE *fp3;//ָ�������־�ļ�
struct City city[citynum];//�洢������Ϣ�Ľṹ������
struct Plan plan[citynum];//�洢�г���Ϣ�Ľṹ������

//����ʵ��
int CityNum(char *p)//������Ӣ������ת��Ϊ��Ӧ���
{
    int i=0;
    while(strcmp(city[i].name, p) != 0)
        i++;
    return i;
}

int sub(int a, int b)//��a>=b,����a-b�����򷵻�a-b+24
{
    if(a >= b)
        return(a-b);
    else return(a-b+24);
}

void Init()//��������Ϣ�͸����еĽ�ͨ����ļ�����ṹ��
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

int dfs(int sc, int st, int ac, float risk)//Ѱ����������������·��
//����ֵΪ1�����ҵ�����·����Ϊ0�����û�ҵ�
{
    Route *p;
    float temp;
    int arrivecity, starttime, arrivetime;
    int better = 0, result = 0;

    length++;
    flag[sc] = 1;//flag[city]=1��ʾ��city�Ѿ���������
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

        if(arrivecity == ac && risk + temp <= TotalRisk && time1 <= timelimit)//�����յ㣬��Ϊ����·��
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
        else if(arrivecity != ac)//δ�����յ�
        {
            result = dfs(arrivecity, arrivetime, ac, risk+temp);//����������һ��·��
            if(result)//����ҵ��˸���·����˵������Ϊ����·��������plan
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

int PassengerState(int state, int time, int *len, int day)//�����ÿ�״̬����������״̬д���г���־�ļ�
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

