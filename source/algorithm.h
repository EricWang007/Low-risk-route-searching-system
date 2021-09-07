#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <QDebug>

#define citynum 10 //城市个数
#define nan 100000000 //无穷大

//数据结构
typedef struct Route
{
    int ArriveCity;//该班次的目的城市
    struct Route *nextRoute;//同出发地的下一班次
    float risk;//该班次交通工具的单位风险值
    int StartTime;//该班次出发时间
    int ArriveTime;//该班次到达时间
    char num[10];//班次号
}Route;

typedef struct City
{
    float risk;//该城市的单位风险值
    char name[10];//该城市的英文名字
    int x;//在地图上的x坐标
    int y;//在地图上的y坐标
    Route *FirstRoute;//指向从该城市出发的个Route
}City;

typedef struct Plan
{
    int StartCity;//该班次出发城市
    int StartTime;//该班次出发时间
    int ArriveCity;//该班次到达城市
    int ArriveTime;//该班次到达时间
    char type[10];//该班次的交通工具名称
    char num[10];//班次号
}Plan;

//全局变量声明
extern float TotalRisk;//行程总风险值
extern int flag[citynum];//表示该city是否被遍历过
extern int length;//临时参数，表示经历了几次换乘
extern int TotalLength;//表示最终的最优路线经历了几次换乘
extern int time1;//临时参数，表示从开始到现在经历了多少时间
extern int TotalTime;//表示最终的路线经历的时间
extern int timelimit;
extern FILE *fp3;//指向输出日志文件
extern struct City city[citynum];//存储城市信息的结构体数组
extern struct Plan plan[citynum];//存储行程信息的结构体数组

//函数声明
extern int CityNum(char *p);//将城市英文名字转化为对应序号
extern int sub(int a, int b);//若a>=b,返回a-b，否则返回a-b+24
extern void Init();//将城市信息和各城市的交通表从文件读入结构体
extern int dfs(int sc, int st, int ac, float risk);//寻找满足条件的最优路径
//返回值为1代表找到更优路径，为0则代表没找到
extern int PassengerState(int state, int time, int *len, int day);//更新旅客状态，并将旅行状态写入行程日志文件

#endif // ALGORITHM_H

