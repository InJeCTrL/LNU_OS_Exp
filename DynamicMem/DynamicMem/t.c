#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#define VeryLittle 10//最小剩余碎片大小
typedef struct FreeArea
{
	long ImageBase;//分区起始地址
	long Size;//分区长度
	int Status;//本条未分配记录是否有效
}FreeArea;//空闲分区说明表项
typedef struct AllocArea
{
	long ImageBase;//分区起始地址
	long Size;//分区长度
	int Status;//本条分区使用记录是否有效
	int ID;//分配的作业(进程)ID
}AllocArea;//分区使用说明表项
typedef struct MemInfo
{
	FreeArea *FreeAreaTbl;//空闲分区说明表首地址
	AllocArea *AllocAreaTbl;//分区使用说明表首地址
	long FreeNum;//空闲分区说明表表项数
	long AllocNum;//分区使用说明表表项数
}MemInfo;//内存动态分配情况
MemInfo *MemInf = NULL;//用户区内存总览
int InitMemMap(void)
{//初始化空闲分区说明表与分区使用说明表
	int i;
	if (MemInf = (MemInfo*)malloc(sizeof(MemInfo)))
	{
		MemInf->AllocNum = 20;//分区使用说明表表项数初始20
		MemInf->FreeNum = 20;//空闲分区说明表表项数初始20
		if (MemInf->AllocAreaTbl = (AllocArea*)malloc(MemInf->AllocNum * sizeof(AllocArea)))
		{
			if (MemInf->FreeAreaTbl = (FreeArea*)malloc(MemInf->FreeNum * sizeof(FreeArea)))
			{
				for (i=0;i<MemInf->AllocNum;i++)
				{//分区使用表内容初始化
					MemInf->AllocAreaTbl[i].Status = 0;
				}
				for (i=0;i<MemInf->FreeNum;i++)
				{//空闲分区表内容初始化
					MemInf->FreeAreaTbl[i].Status = 0;
				}
				MemInf->FreeAreaTbl[0].ImageBase = 64;//假设系统区占用低64B，用户区起始为64B
				MemInf->FreeAreaTbl[0].Size = 2048 - 64;//假设内存总大小为2K
				MemInf->FreeAreaTbl[0].Status = 1;//此条空闲分区记录有效，可用于动态分配
				return 0;
			}
			else
			{//内存分配失败
				free(MemInf->AllocAreaTbl);
				free(MemInf);
				return 1;
			}
		}
		else
		{//内存分配失败
			free(MemInf);
			return 1;
		}
	}
	else
		return 1;//用户区内存总览初始化失败
}
int MovetoAllocTbl(int ID,long _Size,long ImageBase)
{//动态分配内存时从空闲表搬移到使用表
	int i;
	for (i=0;i<MemInf->AllocNum;i++)
	{
		if (MemInf->AllocAreaTbl[i].Status == 0)
		{//某条分区使用记录无效状态
			MemInf->AllocAreaTbl[i].ID = ID;//赋予作业(进程)ID
			MemInf->AllocAreaTbl[i].ImageBase = ImageBase;//赋予分配的内存基址
			MemInf->AllocAreaTbl[i].Size = _Size;//赋予分配的内存块大小
			MemInf->AllocAreaTbl[i].Status = 1;//标记当前状态为已分配
			break;
		}
	}
	if (i == MemInf->AllocNum)
	{//分区使用表暂满，无状态为0
		MemInf->AllocNum += 10;
		MemInf->AllocAreaTbl = (AllocArea*)realloc(MemInf->AllocAreaTbl,MemInf->AllocNum*sizeof(AllocArea));//扩充分区使用表容量
		for (i=MemInf->AllocNum-1;i>=MemInf->AllocNum-9;i--)
			MemInf->AllocAreaTbl[i].Status = 0;
		MovetoAllocTbl(ID,_Size,ImageBase);//再次查找
	}

	return 0;
}
int MemFFAllocate(int ID,long _Size,long *ImageBase)
{//尝试首次适应动态分配内存，0：成功，1：失败
	int i,j;
	for (i=0;i<MemInf->FreeNum;i++)
	{
		if (MemInf->FreeAreaTbl[i].Status == 1)
		{//某条空闲记录可用
			if (MemInf->FreeAreaTbl[i].Size >= _Size)
			{//本空闲空间大小足够
				*ImageBase = MemInf->FreeAreaTbl[i].ImageBase;
				if (_Size + VeryLittle >= MemInf->FreeAreaTbl[i].Size)
				{//碎片过小，直接分配整个分区
					MemInf->FreeAreaTbl[i].Status = 0;//整个分区分出
					MovetoAllocTbl(ID,MemInf->FreeAreaTbl[i].Size,MemInf->FreeAreaTbl[i].ImageBase);//空闲分区表->分区使用表
				}
				else
				{//分配后剩余大于最小碎片
					MovetoAllocTbl(ID,_Size,MemInf->FreeAreaTbl[i].ImageBase);//空闲分区表->分区使用表
					MemInf->FreeAreaTbl[i].ImageBase += _Size;//空闲表的IB++
					MemInf->FreeAreaTbl[i].Size -= _Size;//空闲表的Size--
				}
				return 0;
			}
			else
			{//当前无合适分区
				printf("\n分配失败！ - 无可满足资源大小的分区\n");
				return 1;
			}
		}
		else
		{
			printf("\n分配失败！ - 用户区已全部被分配\n");
			return 1;
		}
	}
	return 0;
}
int MemFFFree(int ID)
{//适用于首次适应的内存释放算法
	int i,j;
	int fore = 0,next = 0;//标志上下邻接
	int nexti;//记录下一个非邻接分区号
	long tIB,tSize;//临时保存预释放的内存分区的起始地址和大小
	for (i=0;i<MemInf->AllocNum;i++)
	{
		if (MemInf->AllocAreaTbl[i].Status == 1 && MemInf->AllocAreaTbl[i].ID == ID)
		{//分区使用表中找到
			MemInf->AllocAreaTbl[i].Status = 0;//设置此条分配为无效
			tIB = MemInf->AllocAreaTbl[i].ImageBase;
			tSize = MemInf->AllocAreaTbl[i].Size;
			break;
		}
	}
	for (i=0;i<MemInf->FreeNum;i++)
	{//FF从开头开始
		if (MemInf->FreeAreaTbl[i].Status)
		{
			if (MemInf->FreeAreaTbl[i].ImageBase + MemInf->FreeAreaTbl[i].Size == tIB)
			{//若空闲表中只有上邻接，则行上合并
				MemInf->FreeAreaTbl[i].Size += tSize;
				tSize = MemInf->FreeAreaTbl[i].Size;
				tIB = MemInf->FreeAreaTbl[i].ImageBase;
				fore = 1;
			}
			else if (MemInf->FreeAreaTbl[i].ImageBase == tIB + tSize)
			{//若空闲表中有下邻接，则行下合并
				MemInf->FreeAreaTbl[i].Size += tSize;
				MemInf->FreeAreaTbl[i].ImageBase -= tSize;
				tSize = MemInf->FreeAreaTbl[i].Size;
				next = 1;
			}
		}
	}
	if (fore == 0 && next == 0)
	{//上下都不邻接
		for (i=0;i<MemInf->FreeNum;i++)
		{
			if (MemInf->FreeAreaTbl[i].Status && MemInf->FreeAreaTbl[i].ImageBase > tIB + tSize)
				break;//检索到第一个不邻接的后续分区则退出循环
		}
		if (i == MemInf->FreeNum)
		{//无后续分区
			MemInf->FreeNum += 10;
			MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//扩充空闲分区表容量
			MemInf->FreeAreaTbl[i].ImageBase = tIB;
			MemInf->FreeAreaTbl[i].Size = tSize;
			MemInf->FreeAreaTbl[i].Status = 1;//直接加入
			return 0;
		}
		else
		{
			nexti = i;
			/*i--;
			if (i <= 0)
			{//若需要插在第一个空闲分区前
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//扩充空闲分区表容量
				for (j=MemInf->FreeNum-1;j>=1;j--)
				{//空闲分区表后移
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[i].ImageBase = tIB;
				MemInf->FreeAreaTbl[i].Size = tSize;
				MemInf->FreeAreaTbl[i].Status = 1;//直接加入
				return 0;
			}*/
			while (i > 0 && (!MemInf->FreeAreaTbl[i].Status) || MemInf->FreeAreaTbl[i].ImageBase + MemInf->FreeAreaTbl[i].Size < tIB)
			{
				i--;
			}
			if (i == nexti)
			{//就在next前新增分区插入
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//扩充空闲分区表容量
				for (j=MemInf->FreeNum-1;j>nexti;j--)
				{//空闲分区表后移
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[nexti].ImageBase = tIB;
				MemInf->FreeAreaTbl[nexti].Size = tSize;
				MemInf->FreeAreaTbl[nexti].Status = 1;//直接加入
				return 0;
			}
			else if (i + 1 != nexti)
			{//上一个空闲分区不邻接
				MemInf->FreeAreaTbl[i + 1].ImageBase = tIB;
				MemInf->FreeAreaTbl[i + 1].Size = tSize;
				MemInf->FreeAreaTbl[i + 1].Status = 1;//直接加入
				return 0;
			}
			else if (i + 1 == nexti)
			{//上一个空闲分区邻接下一个空闲分区
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//扩充空闲分区表容量
				for (j=MemInf->FreeNum-1;j>next;j--)
				{//空闲分区表后移
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[nexti].ImageBase = tIB;
				MemInf->FreeAreaTbl[nexti].Size = tSize;
				MemInf->FreeAreaTbl[nexti].Status = 1;//直接加入
				return 0;
			}
			/*
			if (i == 0)
			{//若需要插在第一个空闲分区前
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//扩充空闲分区表容量
				for (j=MemInf->FreeNum-1;j>=1;j--)
				{//空闲分区表后移
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[i].ImageBase = tIB;
				MemInf->FreeAreaTbl[i].Size = tSize;
				MemInf->FreeAreaTbl[i].Status = 1;//直接加入
				return 0;
			}
			else
			{
				
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//扩充空闲分区表容量
				for (j=MemInf->FreeNum-1;j>=1;j--)
				{//空闲分区表后移
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[i].ImageBase = tIB;
				MemInf->FreeAreaTbl[i].Size = tSize;
				MemInf->FreeAreaTbl[i].Status = 1;//直接加入
				return 0;
			}
			*/
		}

	}
	/*

	if (i == MemInf->AllocNum)
	{//空闲分区表无状态为0
		MemInf->AllocNum += 10;
		MemInf->AllocAreaTbl = (AllocArea*)realloc(MemInf->AllocAreaTbl,MemInf->AllocNum*sizeof(AllocArea));//扩充分区使用表容量
		for (i=MemInf->AllocNum-1;i>=MemInf->AllocNum-9;i--)
			MemInf->AllocAreaTbl[i].Status = 0;
		MovetoAllocTbl(ID,_Size,ImageBase);//再次查找
	}*/
	return 0;
}
int ShowMemInfo(void)
{//显示用户区内存总览
	int i;
	system("cls");
	printf("FreeArea:\n");
	for (i=0;i<MemInf->FreeNum;i++)
	{
		if (MemInf->FreeAreaTbl[i].Status)
			printf("%ld ~ %ld\n",MemInf->FreeAreaTbl[i].ImageBase,MemInf->FreeAreaTbl[i].ImageBase + MemInf->FreeAreaTbl[i].Size - 1);
	}
	printf("///////////////////////////////\nAllocArea:\n");
	for (i=0;i<MemInf->AllocNum;i++)
	{
		if (MemInf->AllocAreaTbl[i].Status)
			printf("Process/Job %d :%ld ~ %ld\n",MemInf->AllocAreaTbl[i].ID,MemInf->AllocAreaTbl[i].ImageBase,MemInf->AllocAreaTbl[i].ImageBase + MemInf->AllocAreaTbl[i].Size - 1);
	}
	return 0;
}

int main(void)
{
	char ch;
	int Q = 0;
	int ID;
	long Size;
	long ImageBase;
	InitMemMap();//初始化用户区内存
	do
	{
		system("cls");
		printf("1.Allocate\n");
		printf("2.Free\n");
		printf("3.ShowMem\n");
		printf("0.Quit\n");
		ch = getch();
		switch (ch)
		{
		case '0':
			Q = 1;
			printf("Press any key to quit\n");
			break;
		case '1':
			ShowMemInfo();
			printf("MemAllocate:(ID Size)\n");
			scanf("%d %ld",&ID,&Size);
			getchar();
			if (MemFFAllocate(ID,Size,&ImageBase) == 0)
			{
				printf("分区分配成功!\n");
				printf("ImageBase: %ld",ImageBase);
			}
			break;
		case '2':
			ShowMemInfo();
			printf("Proc&Job Free:(ID)\n");
			scanf("%d",&ID);
			getchar();
			MemFFFree(ID);
			printf("分区释放完成!\n");
			break;
		case '3':
			ShowMemInfo();
			break;
		default:
			printf("Error Input!\n");
			break;
		}
		getch();
	}while(Q != 1);

	return 0;
}
