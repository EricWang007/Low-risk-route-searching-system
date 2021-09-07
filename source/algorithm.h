#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <QDebug>

#define citynum 10 //���и���
#define nan 100000000 //�����

//���ݽṹ
typedef struct Route
{
    int ArriveCity;//�ð�ε�Ŀ�ĳ���
    struct Route *nextRoute;//ͬ�����ص���һ���
    float risk;//�ð�ν�ͨ���ߵĵ�λ����ֵ
    int StartTime;//�ð�γ���ʱ��
    int ArriveTime;//�ð�ε���ʱ��
    char num[10];//��κ�
}Route;

typedef struct City
{
    float risk;//�ó��еĵ�λ����ֵ
    char name[10];//�ó��е�Ӣ������
    int x;//�ڵ�ͼ�ϵ�x����
    int y;//�ڵ�ͼ�ϵ�y����
    Route *FirstRoute;//ָ��Ӹó��г����ĸ�Route
}City;

typedef struct Plan
{
    int StartCity;//�ð�γ�������
    int StartTime;//�ð�γ���ʱ��
    int ArriveCity;//�ð�ε������
    int ArriveTime;//�ð�ε���ʱ��
    char type[10];//�ð�εĽ�ͨ��������
    char num[10];//��κ�
}Plan;

//ȫ�ֱ�������
extern float TotalRisk;//�г��ܷ���ֵ
extern int flag[citynum];//��ʾ��city�Ƿ񱻱�����
extern int length;//��ʱ��������ʾ�����˼��λ���
extern int TotalLength;//��ʾ���յ�����·�߾����˼��λ���
extern int time1;//��ʱ��������ʾ�ӿ�ʼ�����ھ����˶���ʱ��
extern int TotalTime;//��ʾ���յ�·�߾�����ʱ��
extern int timelimit;
extern FILE *fp3;//ָ�������־�ļ�
extern struct City city[citynum];//�洢������Ϣ�Ľṹ������
extern struct Plan plan[citynum];//�洢�г���Ϣ�Ľṹ������

//��������
extern int CityNum(char *p);//������Ӣ������ת��Ϊ��Ӧ���
extern int sub(int a, int b);//��a>=b,����a-b�����򷵻�a-b+24
extern void Init();//��������Ϣ�͸����еĽ�ͨ����ļ�����ṹ��
extern int dfs(int sc, int st, int ac, float risk);//Ѱ����������������·��
//����ֵΪ1�����ҵ�����·����Ϊ0�����û�ҵ�
extern int PassengerState(int state, int time, int *len, int day);//�����ÿ�״̬����������״̬д���г���־�ļ�

#endif // ALGORITHM_H

