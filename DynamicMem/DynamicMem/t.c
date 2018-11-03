#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#define VeryLittle 10//��Сʣ����Ƭ��С
typedef struct FreeArea
{
	long ImageBase;//������ʼ��ַ
	long Size;//��������
	int Status;//����δ�����¼�Ƿ���Ч
}FreeArea;//���з���˵������
typedef struct AllocArea
{
	long ImageBase;//������ʼ��ַ
	long Size;//��������
	int Status;//��������ʹ�ü�¼�Ƿ���Ч
	int ID;//�������ҵ(����)ID
}AllocArea;//����ʹ��˵������
typedef struct MemInfo
{
	FreeArea *FreeAreaTbl;//���з���˵�����׵�ַ
	AllocArea *AllocAreaTbl;//����ʹ��˵�����׵�ַ
	long FreeNum;//���з���˵���������
	long AllocNum;//����ʹ��˵���������
}MemInfo;//�ڴ涯̬�������
MemInfo *MemInf = NULL;//�û����ڴ�����
int InitMemMap(void)
{//��ʼ�����з���˵���������ʹ��˵����
	int i;
	if (MemInf = (MemInfo*)malloc(sizeof(MemInfo)))
	{
		MemInf->AllocNum = 20;//����ʹ��˵�����������ʼ20
		MemInf->FreeNum = 20;//���з���˵�����������ʼ20
		if (MemInf->AllocAreaTbl = (AllocArea*)malloc(MemInf->AllocNum * sizeof(AllocArea)))
		{
			if (MemInf->FreeAreaTbl = (FreeArea*)malloc(MemInf->FreeNum * sizeof(FreeArea)))
			{
				for (i=0;i<MemInf->AllocNum;i++)
				{//����ʹ�ñ����ݳ�ʼ��
					MemInf->AllocAreaTbl[i].Status = 0;
				}
				for (i=0;i<MemInf->FreeNum;i++)
				{//���з��������ݳ�ʼ��
					MemInf->FreeAreaTbl[i].Status = 0;
				}
				MemInf->FreeAreaTbl[0].ImageBase = 64;//����ϵͳ��ռ�õ�64B���û�����ʼΪ64B
				MemInf->FreeAreaTbl[0].Size = 2048 - 64;//�����ڴ��ܴ�СΪ2K
				MemInf->FreeAreaTbl[0].Status = 1;//�������з�����¼��Ч�������ڶ�̬����
				return 0;
			}
			else
			{//�ڴ����ʧ��
				free(MemInf->AllocAreaTbl);
				free(MemInf);
				return 1;
			}
		}
		else
		{//�ڴ����ʧ��
			free(MemInf);
			return 1;
		}
	}
	else
		return 1;//�û����ڴ�������ʼ��ʧ��
}
int MovetoAllocTbl(int ID,long _Size,long ImageBase)
{//��̬�����ڴ�ʱ�ӿ��б���Ƶ�ʹ�ñ�
	int i;
	for (i=0;i<MemInf->AllocNum;i++)
	{
		if (MemInf->AllocAreaTbl[i].Status == 0)
		{//ĳ������ʹ�ü�¼��Ч״̬
			MemInf->AllocAreaTbl[i].ID = ID;//������ҵ(����)ID
			MemInf->AllocAreaTbl[i].ImageBase = ImageBase;//���������ڴ��ַ
			MemInf->AllocAreaTbl[i].Size = _Size;//���������ڴ���С
			MemInf->AllocAreaTbl[i].Status = 1;//��ǵ�ǰ״̬Ϊ�ѷ���
			break;
		}
	}
	if (i == MemInf->AllocNum)
	{//����ʹ�ñ���������״̬Ϊ0
		MemInf->AllocNum += 10;
		MemInf->AllocAreaTbl = (AllocArea*)realloc(MemInf->AllocAreaTbl,MemInf->AllocNum*sizeof(AllocArea));//�������ʹ�ñ�����
		for (i=MemInf->AllocNum-1;i>=MemInf->AllocNum-9;i--)
			MemInf->AllocAreaTbl[i].Status = 0;
		MovetoAllocTbl(ID,_Size,ImageBase);//�ٴβ���
	}

	return 0;
}
int MemFFAllocate(int ID,long _Size,long *ImageBase)
{//�����״���Ӧ��̬�����ڴ棬0���ɹ���1��ʧ��
	int i,j;
	for (i=0;i<MemInf->FreeNum;i++)
	{
		if (MemInf->FreeAreaTbl[i].Status == 1)
		{//ĳ�����м�¼����
			if (MemInf->FreeAreaTbl[i].Size >= _Size)
			{//�����пռ��С�㹻
				*ImageBase = MemInf->FreeAreaTbl[i].ImageBase;
				if (_Size + VeryLittle >= MemInf->FreeAreaTbl[i].Size)
				{//��Ƭ��С��ֱ�ӷ�����������
					MemInf->FreeAreaTbl[i].Status = 0;//���������ֳ�
					MovetoAllocTbl(ID,MemInf->FreeAreaTbl[i].Size,MemInf->FreeAreaTbl[i].ImageBase);//���з�����->����ʹ�ñ�
				}
				else
				{//�����ʣ�������С��Ƭ
					MovetoAllocTbl(ID,_Size,MemInf->FreeAreaTbl[i].ImageBase);//���з�����->����ʹ�ñ�
					MemInf->FreeAreaTbl[i].ImageBase += _Size;//���б��IB++
					MemInf->FreeAreaTbl[i].Size -= _Size;//���б��Size--
				}
				return 0;
			}
			else
			{//��ǰ�޺��ʷ���
				printf("\n����ʧ�ܣ� - �޿�������Դ��С�ķ���\n");
				return 1;
			}
		}
		else
		{
			printf("\n����ʧ�ܣ� - �û�����ȫ��������\n");
			return 1;
		}
	}
	return 0;
}
int MemFFFree(int ID)
{//�������״���Ӧ���ڴ��ͷ��㷨
	int i,j;
	int fore = 0,next = 0;//��־�����ڽ�
	int nexti;//��¼��һ�����ڽӷ�����
	long tIB,tSize;//��ʱ����Ԥ�ͷŵ��ڴ��������ʼ��ַ�ʹ�С
	for (i=0;i<MemInf->AllocNum;i++)
	{
		if (MemInf->AllocAreaTbl[i].Status == 1 && MemInf->AllocAreaTbl[i].ID == ID)
		{//����ʹ�ñ����ҵ�
			MemInf->AllocAreaTbl[i].Status = 0;//���ô�������Ϊ��Ч
			tIB = MemInf->AllocAreaTbl[i].ImageBase;
			tSize = MemInf->AllocAreaTbl[i].Size;
			break;
		}
	}
	for (i=0;i<MemInf->FreeNum;i++)
	{//FF�ӿ�ͷ��ʼ
		if (MemInf->FreeAreaTbl[i].Status)
		{
			if (MemInf->FreeAreaTbl[i].ImageBase + MemInf->FreeAreaTbl[i].Size == tIB)
			{//�����б���ֻ�����ڽӣ������Ϻϲ�
				MemInf->FreeAreaTbl[i].Size += tSize;
				tSize = MemInf->FreeAreaTbl[i].Size;
				tIB = MemInf->FreeAreaTbl[i].ImageBase;
				fore = 1;
			}
			else if (MemInf->FreeAreaTbl[i].ImageBase == tIB + tSize)
			{//�����б��������ڽӣ������ºϲ�
				MemInf->FreeAreaTbl[i].Size += tSize;
				MemInf->FreeAreaTbl[i].ImageBase -= tSize;
				tSize = MemInf->FreeAreaTbl[i].Size;
				next = 1;
			}
		}
	}
	if (fore == 0 && next == 0)
	{//���¶����ڽ�
		for (i=0;i<MemInf->FreeNum;i++)
		{
			if (MemInf->FreeAreaTbl[i].Status && MemInf->FreeAreaTbl[i].ImageBase > tIB + tSize)
				break;//��������һ�����ڽӵĺ����������˳�ѭ��
		}
		if (i == MemInf->FreeNum)
		{//�޺�������
			MemInf->FreeNum += 10;
			MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//������з���������
			MemInf->FreeAreaTbl[i].ImageBase = tIB;
			MemInf->FreeAreaTbl[i].Size = tSize;
			MemInf->FreeAreaTbl[i].Status = 1;//ֱ�Ӽ���
			return 0;
		}
		else
		{
			nexti = i;
			/*i--;
			if (i <= 0)
			{//����Ҫ���ڵ�һ�����з���ǰ
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//������з���������
				for (j=MemInf->FreeNum-1;j>=1;j--)
				{//���з��������
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[i].ImageBase = tIB;
				MemInf->FreeAreaTbl[i].Size = tSize;
				MemInf->FreeAreaTbl[i].Status = 1;//ֱ�Ӽ���
				return 0;
			}*/
			while (i > 0 && (!MemInf->FreeAreaTbl[i].Status) || MemInf->FreeAreaTbl[i].ImageBase + MemInf->FreeAreaTbl[i].Size < tIB)
			{
				i--;
			}
			if (i == nexti)
			{//����nextǰ������������
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//������з���������
				for (j=MemInf->FreeNum-1;j>nexti;j--)
				{//���з��������
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[nexti].ImageBase = tIB;
				MemInf->FreeAreaTbl[nexti].Size = tSize;
				MemInf->FreeAreaTbl[nexti].Status = 1;//ֱ�Ӽ���
				return 0;
			}
			else if (i + 1 != nexti)
			{//��һ�����з������ڽ�
				MemInf->FreeAreaTbl[i + 1].ImageBase = tIB;
				MemInf->FreeAreaTbl[i + 1].Size = tSize;
				MemInf->FreeAreaTbl[i + 1].Status = 1;//ֱ�Ӽ���
				return 0;
			}
			else if (i + 1 == nexti)
			{//��һ�����з����ڽ���һ�����з���
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//������з���������
				for (j=MemInf->FreeNum-1;j>next;j--)
				{//���з��������
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[nexti].ImageBase = tIB;
				MemInf->FreeAreaTbl[nexti].Size = tSize;
				MemInf->FreeAreaTbl[nexti].Status = 1;//ֱ�Ӽ���
				return 0;
			}
			/*
			if (i == 0)
			{//����Ҫ���ڵ�һ�����з���ǰ
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//������з���������
				for (j=MemInf->FreeNum-1;j>=1;j--)
				{//���з��������
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[i].ImageBase = tIB;
				MemInf->FreeAreaTbl[i].Size = tSize;
				MemInf->FreeAreaTbl[i].Status = 1;//ֱ�Ӽ���
				return 0;
			}
			else
			{
				
				MemInf->FreeNum++;
				MemInf->FreeAreaTbl = (FreeArea*)realloc(MemInf->FreeAreaTbl,MemInf->FreeNum*sizeof(FreeArea));//������з���������
				for (j=MemInf->FreeNum-1;j>=1;j--)
				{//���з��������
					MemInf->FreeAreaTbl[j].ImageBase = MemInf->FreeAreaTbl[j-1].ImageBase;
					MemInf->FreeAreaTbl[j].Size = MemInf->FreeAreaTbl[j-1].Size;
					MemInf->FreeAreaTbl[j].Status = MemInf->FreeAreaTbl[j-1].Status;
				}
				MemInf->FreeAreaTbl[i].ImageBase = tIB;
				MemInf->FreeAreaTbl[i].Size = tSize;
				MemInf->FreeAreaTbl[i].Status = 1;//ֱ�Ӽ���
				return 0;
			}
			*/
		}

	}
	/*

	if (i == MemInf->AllocNum)
	{//���з�������״̬Ϊ0
		MemInf->AllocNum += 10;
		MemInf->AllocAreaTbl = (AllocArea*)realloc(MemInf->AllocAreaTbl,MemInf->AllocNum*sizeof(AllocArea));//�������ʹ�ñ�����
		for (i=MemInf->AllocNum-1;i>=MemInf->AllocNum-9;i--)
			MemInf->AllocAreaTbl[i].Status = 0;
		MovetoAllocTbl(ID,_Size,ImageBase);//�ٴβ���
	}*/
	return 0;
}
int ShowMemInfo(void)
{//��ʾ�û����ڴ�����
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
	InitMemMap();//��ʼ���û����ڴ�
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
				printf("��������ɹ�!\n");
				printf("ImageBase: %ld",ImageBase);
			}
			break;
		case '2':
			ShowMemInfo();
			printf("Proc&Job Free:(ID)\n");
			scanf("%d",&ID);
			getchar();
			MemFFFree(ID);
			printf("�����ͷ����!\n");
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
