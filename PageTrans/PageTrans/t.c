#include<windows.h>
#include<conio.h>
#include<stdio.h>
#define MaxPageNum 3//ÿ����ҵֻ����3�������
#define BlockSize 1024//������С
typedef struct PageTableItem
{
	int Page;//ҳ��
	int Exists;//���ڱ�־
	int MemBlock;//�����еĿ��
	int Edit;//�޸ı�־
	int RAWBlock;//�����еĿ��
	int Time;//ʱ����FIFOʱ��ҳ�汾�����ڴ����ʱ����LRUʱ��ҳ�����ϴ�ʹ�ú����ʱ��
}PageTableItem;//ҳ����
int Method = -1;//��ҳ��ʽ0:FIFO 1:LRU
int InitPageTable(PageTableItem **PageTbl,int PageItemNum)
{//��ʼ��ҳ��
	int i;
	(*PageTbl) = (PageTableItem*)malloc(PageItemNum*sizeof(PageTableItem));
	for (i=0;i<PageItemNum;i++)
	{//ҳ����и��г�ʼ��
		(*PageTbl)[i].Edit = 0;
		(*PageTbl)[i].Exists = 0;
		(*PageTbl)[i].MemBlock = 0;
		(*PageTbl)[i].Page = i;
		(*PageTbl)[i].RAWBlock = i + 2;
		(*PageTbl)[i].Time = 0;
	}
	for (i=0;i<3;i++)
	{//ǰ����ҳ��Ԥװ���ڴ�
		(*PageTbl)[i].Exists = 1;
		(*PageTbl)[i].MemBlock = i * 10;
		(*PageTbl)[i].Time = i;
	}

	return 0;
}
int SelectMethod(void)
{//ѡ���ҳ��ʽ
	char ch;
	do
	{
		system("cls");
		printf("��ҳ��ʽ��\n");
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
	}while (Method == -1);//����ҳ��ʽδ��ȷѡ��
	return 0;
}
int GetMaxP(PageTableItem *PageTbl,int PageItemNum)
{//�õ��ڴ���Time����ҳ��
	int maxT = 0;//���Time
	int maxP = 0;//���Time��Ӧ��ҳ��
	int i;
	for (i=0;i<PageItemNum;i++)
	{//�ҳ�Time����ҳ���
		if (PageTbl[i].Exists && maxT < PageTbl[i].Time)
		{
			maxP = i;
			maxT = PageTbl[i].Time;
		}
	}
	return maxP;
}
int IntTrans(PageTableItem *PageTbl,int Page,int PageItemNum)
{//ģ��ȱҳ�ж�
	int maxP = 0;//���Time��Ӧ��ҳ��
	int i;
	maxP = GetMaxP(PageTbl,PageItemNum);//�õ��ڴ���Time����ҳ��
	PageTbl[maxP].Exists = 0;//ҳ�滻��
	PageTbl[maxP].Time = 0;//Time����
	PageTbl[Page].Exists = 1;//��ҳ�滻��
	PageTbl[Page].MemBlock = Page * 10;//ҳ�����ڴ�Ŀ�Ÿ�ֵ
	return PageTbl[Page].MemBlock * BlockSize;//���ؿ��
}
int TransVA(PageTableItem *PageTbl,int VA,int PageItemNum)
{//�߼���ַת�����ַ
	int Page,RA;//ҳ�ţ�ҳ��ƫ��
	int tBlock;//���
	int i;
	Page = VA / BlockSize;//����ҳ��
	RA = VA % BlockSize;//����ҳ��ƫ��
	if (Page >= PageItemNum)
		return -1;//�߼���ַԽ��
	for (i=0;i<PageItemNum;i++)
		if (PageTbl[i].Exists == 1)
			PageTbl[i].Time++;//�����ڴ��е�ҳ����ʱ��+1
	if (PageTbl[Page].Exists == 1)
	{//ҳ���ڴ���
		if (Method == 1)
			PageTbl[Page].Time = 0;//LRU����ʱ������
		return PageTbl[Page].MemBlock * BlockSize + RA;//���*�鳤+ҳ��ƫ��
	}
	else
	{//ҳ�����ڴ���
		tBlock = IntTrans(PageTbl,Page,PageItemNum);//����ȱҳ�ж�
		return tBlock * BlockSize + RA;//���*�鳤+ҳ��ƫ��
	}
}
int ShowPageInfo(PageTableItem *PageTbl,int PageItemNum)
{//��ʾҳ�����
	int i;
	printf("ҳ�� ���ڱ�־ ������ �޸ı�־ ������\n");
	for (i=0;i<PageItemNum;i++)
	{
		printf("%d%8d%10d%10d%8d\n",i,PageTbl[i].Exists,PageTbl[i].MemBlock,PageTbl[i].Edit,PageTbl[i].RAWBlock);
	}
	printf("----------------------------------------------\n");
	return 0;
}

int main(void)
{
	PageTableItem *PageTable = NULL;//ҳ��
	int PageNum;//ҳ��(ҳ������)
	int VirtualAddr;//�߼���ַ
	int retPA;//���ص������ַ
	char Continue;//������־

	SelectMethod();//ѡ���ҳ��ʽ
	system("cls");

	printf("��ҵҳ����");
	scanf("%d",&PageNum);
	InitPageTable(&PageTable,PageNum);//��ʼ��ҳ��Ԥװ��ǰ����ҳ��
	
	do
	{
		system("cls");
		ShowPageInfo(PageTable,PageNum);//��ʾҳ��
		printf("�߼���ַ(�鳤 %d B)��",BlockSize);
		scanf("%d",&VirtualAddr);
		retPA = TransVA(PageTable,VirtualAddr,PageNum);//���Խ��߼���ַת��Ϊ�����ַ
		if (retPA == -1)
			printf("�߼���ַԽ�磡\n");
		else
			printf("�����ַ��%d\n",retPA);
		printf("����(1)�����(0)�� ");
		Continue = getch();
	}while(Continue == '1');

	free(PageTable);
	system("cls");
	printf("Press any key to quit.\n");
	getch();
	return 0;
}
