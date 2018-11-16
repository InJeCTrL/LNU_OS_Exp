#include<windows.h>
#include<conio.h>
#include<stdio.h>
#define MaxPageNum 3//每个作业只分配3个物理块
#define BlockSize 1024//物理块大小
typedef struct PageTableItem
{
	int Page;//页号
	int Exists;//存在标志
	int MemBlock;//主存中的块号
	int Edit;//修改标志
	int RAWBlock;//辅存中的块号
	int Time;//时长，FIFO时：页面本次在内存的总时长，LRU时：页面自上次使用后的总时长
}PageTableItem;//页表项
int Method = -1;//调页方式0:FIFO 1:LRU
int InitPageTable(PageTableItem **PageTbl,int PageItemNum)
{//初始化页表
	int i;
	(*PageTbl) = (PageTableItem*)malloc(PageItemNum*sizeof(PageTableItem));
	for (i=0;i<PageItemNum;i++)
	{//页表各行各列初始化
		(*PageTbl)[i].Edit = 0;
		(*PageTbl)[i].Exists = 0;
		(*PageTbl)[i].MemBlock = 0;
		(*PageTbl)[i].Page = i;
		(*PageTbl)[i].RAWBlock = i + 2;
		(*PageTbl)[i].Time = 0;
	}
	for (i=0;i<3;i++)
	{//前三个页面预装入内存
		(*PageTbl)[i].Exists = 1;
		(*PageTbl)[i].MemBlock = i * 10;
		(*PageTbl)[i].Time = i;
	}

	return 0;
}
int SelectMethod(void)
{//选择调页方式
	char ch;
	do
	{
		system("cls");
		printf("调页方式：\n");
		printf("0.FIFO\n");
		printf("1.LRU\n");
		printf("Input choice: ");
		ch = getch();
		switch (ch)
		{
			case '0':
				Method = 0;
				break;
			case '1':
				Method = 1;
				break;
			default:
				printf("Error input!Retry.");
				getch();
				break;
		}
	}while (Method == -1);//若调页方式未正确选择
	return 0;
}
int GetMaxP(PageTableItem *PageTbl,int PageItemNum)
{//得到内存中Time最大的页号
	int maxT = 0;//最大Time
	int maxP = 0;//最大Time对应的页号
	int i;
	for (i=0;i<PageItemNum;i++)
	{//找出Time最大的页表号
		if (PageTbl[i].Exists && maxT < PageTbl[i].Time)
		{
			maxP = i;
			maxT = PageTbl[i].Time;
		}
	}
	return maxP;
}
int IntTrans(PageTableItem *PageTbl,int Page,int PageItemNum)
{//模拟缺页中断
	int maxP = 0;//最大Time对应的页号
	int i;
	maxP = GetMaxP(PageTbl,PageItemNum);//得到内存中Time最大的页号
	PageTbl[maxP].Exists = 0;//页面换出
	PageTbl[maxP].Time = 0;//Time清零
	PageTbl[Page].Exists = 1;//新页面换入
	PageTbl[Page].MemBlock = Page * 10;//页面在内存的块号赋值
	return PageTbl[Page].MemBlock * BlockSize;//返回块号
}
int TransVA(PageTableItem *PageTbl,int VA,int PageItemNum)
{//逻辑地址转物理地址
	int Page,RA;//页号，页内偏移
	int tBlock;//块号
	int i;
	Page = VA / BlockSize;//计算页号
	RA = VA % BlockSize;//计算页内偏移
	if (Page >= PageItemNum)
		return -1;//逻辑地址越界
	for (i=0;i<PageItemNum;i++)
		if (PageTbl[i].Exists == 1)
			PageTbl[i].Time++;//所有内存中的页面总时长+1
	if (PageTbl[Page].Exists == 1)
	{//页在内存中
		if (Method == 1)
			PageTbl[Page].Time = 0;//LRU，总时长清零
		return PageTbl[Page].MemBlock * BlockSize + RA;//块号*块长+页内偏移
	}
	else
	{//页不在内存中
		tBlock = IntTrans(PageTbl,Page,PageItemNum);//触发缺页中断
		return tBlock * BlockSize + RA;//块号*块长+页内偏移
	}
}
int ShowPageInfo(PageTableItem *PageTbl,int PageItemNum)
{//显示页表情况
	int i;
	printf("页号 存在标志 主存块号 修改标志 辅存块号\n");
	for (i=0;i<PageItemNum;i++)
	{
		printf("%d%8d%10d%10d%8d\n",i,PageTbl[i].Exists,PageTbl[i].MemBlock,PageTbl[i].Edit,PageTbl[i].RAWBlock);
	}
	printf("----------------------------------------------\n");
	return 0;
}

int main(void)
{
	PageTableItem *PageTable = NULL;//页表
	int PageNum;//页数(页表项数)
	int VirtualAddr;//逻辑地址
	int retPA;//返回的物理地址
	char Continue;//继续标志

	SelectMethod();//选择调页方式
	system("cls");

	printf("作业页数：");
	scanf("%d",&PageNum);
	InitPageTable(&PageTable,PageNum);//初始化页表并预装入前三个页面
	
	do
	{
		system("cls");
		ShowPageInfo(PageTable,PageNum);//显示页表
		printf("逻辑地址(块长 %d B)：",BlockSize);
		scanf("%d",&VirtualAddr);
		retPA = TransVA(PageTable,VirtualAddr,PageNum);//尝试将逻辑地址转换为物理地址
		if (retPA == -1)
			printf("逻辑地址越界！\n");
		else
			printf("物理地址：%d\n",retPA);
		printf("继续(1)或结束(0)？ ");
		Continue = getch();
	}while(Continue == '1');

	free(PageTable);
	system("cls");
	printf("Press any key to quit.\n");
	getch();
	return 0;
}
