#include<stdio.h>
#include<conio.h>
#include<windows.h>
#define ProcNum 5//进程数5
#define ResNum 3//资源类数3
typedef struct ProcInfo
{
	int Max[ProcNum][ResNum];//进程的最大资源需求
	int Allocation[ProcNum][ResNum];//进程已分配的资源
	int Need[ProcNum][ResNum];//进程尚需的资源
}ProcInfo;//进程-资源信息表
int Work[ResNum];//当前可用资源+某进程释放出的资源
int Available[ResNum];//当前可用资源数
ProcInfo PStbl;//进程-资源信息表
int Finish[ProcNum] = {0};//进程是否结束
int AllOver[ProcNum] = {0};//标志进程是否完全结束
int InitPSA(void)
{//初始化进程信息表、当前可用资源数
//ProcNum(Max-1)+1 <= Available
	int i,j;
	for (i=0;i<ProcNum;i++)
	{
		for (j=0;j<ResNum;j++)
		{
			PStbl.Max[i][j] = rand() % 19 + 2;//Max:2~20
			PStbl.Need[i][j] = rand() % (PStbl.Max[i][j]) + 1;//Need:1~Max
			PStbl.Allocation[i][j] = PStbl.Max[i][j] - PStbl.Need[i][j];//Allocation=Max-Need
		}
	}
	for (i=0;i<ResNum;i++)
		Available[i] = rand() % 11 + 30;//Available:30~40
	return 0;
}
int ShowPStbl(void)
{//显示资源分配表
	int i,j;
	system("cls");
	for (i=0;i<ProcNum;i++)
	{
		printf("/////////////////////////\nProc%d:\nMax:",i);
		for (j=0;j<ResNum;j++)
		{
			printf(" %d",PStbl.Max[i][j]);
		}
		printf("\nAllocation:");
		for (j=0;j<ResNum;j++)
		{
			printf(" %d",PStbl.Allocation[i][j]);
		}
		printf("\nNeed:");
		for (j=0;j<ResNum;j++)
		{
			printf(" %d",PStbl.Need[i][j]);
		}
		printf("\n/////////////////////////\n");
	}
	printf("\nAvailable:");
	for (j=0;j<ResNum;j++)
	{
		printf("%d ",Available[j]);
	}
	return 0;
}
int IsProcAllOver(int ProcIndex)
{//进程是否实际完全结束
	int i;
	for (i=0;i<ResNum;i++)
		if (PStbl.Need[ProcIndex][i] != 0)
			return 0;
	return 1;
}
int BankerCheck(int ProcIndex,int *Req,int *RetPL)
{//银行家检查算法，返回0：安全，1：不安全，2：不合法
	int i,j,n,k = 0;
	int FinishNum = 0;//结束的进程数
	int AllOverNum = 0;//已经完全结束的进程数
	int EnableNum;//本次检查可用的进程数
	int Sec = 1;//表示是否安全，1：不安全，0：安全
	///////////////////////////////
	for (i=0;i<ResNum;i++)
	{//检查资源请求是否合法
		if (PStbl.Need[ProcIndex][i] < Req[i] || Available[i] < Req[i])
		{
			break;//若资源序列非法或资源无法申请则跳出
		}
	}
	if (i < ResNum)
	{//资源合法检查失败
		return 2;
	}
	///////////////////////////////
	for (i=0;i<ResNum;i++)
	{//资源分配表预修改
		PStbl.Allocation[ProcIndex][i] += Req[i];//Allocation++
		PStbl.Need[ProcIndex][i] -= Req[i];//Need--
		Available[i] -= Req[i];//Available--
		Work[i] = Available[i];//Work赋初值
	}
	///////////////////////////////
	for (i=0;i<ProcNum;i++)
	{//Finish归零
		Finish[i] = 0;
		AllOverNum += AllOver[i];
	}
	EnableNum = ProcNum - AllOverNum;//当前可用进程数
	///////////////////////////////
	for (i=0;i<ProcNum;i++)
	{//安全序列归-1
		RetPL[i] = -1;
	}
	///////////////////////////////
	for (n=0;n<ProcNum;n++)
	{//循环(进程数)次
		for (i=0;i<ProcNum;i++)
		{//每个进程检查
			if (Finish[i] == 1 || AllOver[i] == 1)
				continue;//该进程已结束，判断下一个进程
			for (j=0;j<ResNum;j++)
			{//检查Need是否小等于Work
				if (PStbl.Need[i][j] > Work[j])
				{//若发现有Need大于Work则跳过此进程
					break;
				}
			}
			if (j == ResNum)
			{//若Need小等于Work则Work++，Finish=1
				for (j=0;j<ResNum;j++)
					Work[j] += PStbl.Allocation[i][j];
				Finish[i] = 1;
				FinishNum++;
				RetPL[k++] = i;//更新安全序列
			}
		}
		if (FinishNum == EnableNum)
		{//结束进程数等于当前可用进程数则退出循环
			Sec = 0;
			break;
		}
	}
	///////////////////////////////
	for (i=0;i<ResNum;i++)
	{//撤销资源分配表预修改
		PStbl.Allocation[ProcIndex][i] -= Req[i];//Allocation--
		PStbl.Need[ProcIndex][i] += Req[i];//Need++
		Available[i] += Req[i];//Available++
	}
	return Sec;
}
int Request(void)
{//资源请求
	int R[ResNum];//资源请求序列
	int ProcIndex;//要请求资源的进程序号
	int RetPL[ProcNum] = {-1};//返回的安全序列
	int ret;
	int i;
	printf("\n请求资源的进程序号：");
	scanf("%d",&ProcIndex);
	printf("\n资源请求序列(%d)：",ResNum);
	for (i=0;i<ResNum;i++)
		scanf("%d",&R[i]);
	ret = BankerCheck(ProcIndex,R,RetPL);
	if (ret == 1)
	{
		printf("\n此次资源请求拒绝分配 -- 不安全\n");
	}
	else if (ret == 2)
	{
		printf("\n此次资源请求拒绝分配 -- 不合法\n");
	}
	else
	{
		printf("\n安全序列：");
		for (i=0;i<ProcNum && RetPL[i] != -1;i++)
			printf(" %d",RetPL[i]);//输出安全序列
		printf("\n此次资源请求成功\n");
		for (i=0;i<ResNum;i++)
		{//修改资源分配表
			PStbl.Allocation[ProcIndex][i] += R[i];//Allocation++
			PStbl.Need[ProcIndex][i] -= R[i];//Need--
			Available[i] -= R[i];
		}
	}
	for (i=0;i<ProcNum;i++)
	{
		if (IsProcAllOver(i))
			AllOver[i] = 1;//若进程Need为0，则视为进程已完全结束
	}
	
	return 0;
}

int main(void)
{
	char ch;
	int Q = 0;//退出标志
	srand(GetTickCount());
	InitPSA();//初始化进程资源表
	do
	{
		system("cls");
		printf("1.资源申请\n");
		printf("2.显示资源分配表\n");
		printf("0.退出\n");
		printf("\n输入选项\n");
		ch = getch();
		switch (ch)
		{
			case '1'://显示并请求输入
				ShowPStbl();
				Request();
				break;
			case '2'://显示分配表
				ShowPStbl();
				break;
			case '0':
				Q = 1;//标志程序退出
				printf("\n任意键退出\n");
				break;
			default:
				printf("\n错误请重新输入，任意键重试\n");
				break;
		}
		getch();
	}while (Q != 1);
	return 0;
}
