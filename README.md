# Low-risk-route-searching-system
## 项目介绍
这是一个基于Qt的低风险旅行路线查询系统。设定的背景为COVID-19疫情环境。系统中包含十个城市，城市之间有飞机、火车、汽车三种交通工具。不同的城市和不同的交通工具有不同的单位风险值。用户可选择旅客的出发地和目的地，以及选择旅行策略并设定旅行时间上限，系统会根据这些信息以及交通工具的时刻表计算出最佳路线，并实时显示在地图上。用户可以实时查看每一时刻旅客的状态，以及在最后查看整个行程的用时、风险值等。系统还会生成一份包含行程具体信息的日志文件。<br>
## 运行截图
### 开始界面：
 <img src="https://github.com/EricWang007/Low-risk-route-searching-system/blob/master/Begin-State1.JPG" width="800" /><br>
### 到达界面:
 <img src="https://github.com/EricWang007/Low-risk-route-searching-system/blob/master/Arrive-State1.JPG" width="800" /><br>
 ### 行程日志:
 <img src="https://github.com/EricWang007/Low-risk-route-searching-system/blob/master/journal1.JPG" width="300" /><br>
## 核心算法
程序的核心算法并不复杂，就是利用暴力的dfs算法，根据开始时间、出发时间、开始城市、目的城市四个参数，遍历交通表，搜索出所有可能的路线组合中风险最小的一种。<br>
风险计算公式如下:<br>
  #旅客在某城市停留的风险 = 该城市单位时间风险值 * 停留时间<br>
  #旅客乘坐某班次交通工具的风险 = 该交通工具单位时间风险值 * 该班次起点城市的单位风险值 * 乘坐时间<br> 
  #旅客一趟行程的总风险值 = 旅客在各个城市停留的风险值之和 + 旅客乘坐各个班次交通工具的风险值之和<br>
## 待修复的Bug
在不同分辨率的屏幕上字体大小会发生变化，导致文字出现偏移。
## 补充说明
本项目是北京邮电大学计算机学院数据结构课程设计的作业。第一次使用Qt，C和C++混着用，程序模块划分的很乱，故不上传源码了。就放个release记录一下自己的一点点小成果吧<br>
