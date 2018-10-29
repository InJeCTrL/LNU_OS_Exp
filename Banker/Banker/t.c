#include<stdio.h>
#include<conio.h>
#include<windows.h>
#define ProcNum 5//������5
#define ResNum 3//��Դ����3
typedef struct ProcInfo
{
	int Max[ProcNum][ResNum];//���̵������Դ����
	int Allocation[ProcNum][ResNum];//�����ѷ������Դ
	int Need[ProcNum][ResNum];//�����������Դ
}ProcInfo;//����-��Դ��Ϣ��
int Work[ResNum];//��ǰ������Դ+ĳ�����ͷų�����Դ
int Available[ResNum];//��ǰ������Դ��
ProcInfo PStbl;//����-��Դ��Ϣ��
int Finish[ProcNum] = {0};//�����Ƿ����
int AllOver[ProcNum] = {0};//��־�����Ƿ���ȫ����
int InitPSA(void)
{//��ʼ��������Ϣ����ǰ������Դ��
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
{//��ʾ��Դ�����
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
{//�����Ƿ�ʵ����ȫ����
	int i;
	for (i=0;i<ResNum;i++)
		if (PStbl.Need[ProcIndex][i] != 0)
			return 0;
	return 1;
}
int BankerCheck(int ProcIndex,int *Req,int *RetPL)
{//���мҼ���㷨������0����ȫ��1������ȫ��2�����Ϸ�
	int i,j,n,k = 0;
	int FinishNum = 0;//�����Ľ�����
	int AllOverNum = 0;//�Ѿ���ȫ�����Ľ�����
	int EnableNum;//���μ����õĽ�����
	int Sec = 1;//��ʾ�Ƿ�ȫ��1������ȫ��0����ȫ
	///////////////////////////////
	for (i=0;i<ResNum;i++)
	{//�����Դ�����Ƿ�Ϸ�
		if (PStbl.Need[ProcIndex][i] < Req[i] || Available[i] < Req[i])
		{
			break;//����Դ���зǷ�����Դ�޷�����������
		}
	}
	if (i < ResNum)
	{//��Դ�Ϸ����ʧ��
		return 2;
	}
	///////////////////////////////
	for (i=0;i<ResNum;i++)
	{//��Դ�����Ԥ�޸�
		PStbl.Allocation[ProcIndex][i] += Req[i];//Allocation++
		PStbl.Need[ProcIndex][i] -= Req[i];//Need--
		Available[i] -= Req[i];//Available--
		Work[i] = Available[i];//Work����ֵ
	}
	///////////////////////////////
	for (i=0;i<ProcNum;i++)
	{//Finish����
		Finish[i] = 0;
		AllOverNum += AllOver[i];
	}
	EnableNum = ProcNum - AllOverNum;//��ǰ���ý�����
	///////////////////////////////
	for (i=0;i<ProcNum;i++)
	{//��ȫ���й�-1
		RetPL[i] = -1;
	}
	///////////////////////////////
	for (n=0;n<ProcNum;n++)
	{//ѭ��(������)��
		for (i=0;i<ProcNum;i++)
		{//ÿ�����̼��
			if (Finish[i] == 1 || AllOver[i] == 1)
				continue;//�ý����ѽ������ж���һ������
			for (j=0;j<ResNum;j++)
			{//���Need�Ƿ�С����Work
				if (PStbl.Need[i][j] > Work[j])
				{//��������Need����Work�������˽���
					break;
				}
			}
			if (j == ResNum)
			{//��NeedС����Work��Work++��Finish=1
				for (j=0;j<ResNum;j++)
					Work[j] += PStbl.Allocation[i][j];
				Finish[i] = 1;
				FinishNum++;
				RetPL[k++] = i;//���°�ȫ����
			}
		}
		if (FinishNum == EnableNum)
		{//�������������ڵ�ǰ���ý��������˳�ѭ��
			Sec = 0;
			break;
		}
	}
	///////////////////////////////
	for (i=0;i<ResNum;i++)
	{//������Դ�����Ԥ�޸�
		PStbl.Allocation[ProcIndex][i] -= Req[i];//Allocation--
		PStbl.Need[ProcIndex][i] += Req[i];//Need++
		Available[i] += Req[i];//Available++
	}
	return Sec;
}
int Request(void)
{//��Դ����
	int R[ResNum];//��Դ��������
	int ProcIndex;//Ҫ������Դ�Ľ������
	int RetPL[ProcNum] = {-1};//���صİ�ȫ����
	int ret;
	int i;
	printf("\n������Դ�Ľ�����ţ�");
	scanf("%d",&ProcIndex);
	printf("\n��Դ��������(%d)��",ResNum);
	for (i=0;i<ResNum;i++)
		scanf("%d",&R[i]);
	ret = BankerCheck(ProcIndex,R,RetPL);
	if (ret == 1)
	{
		printf("\n�˴���Դ����ܾ����� -- ����ȫ\n");
	}
	else if (ret == 2)
	{
		printf("\n�˴���Դ����ܾ����� -- ���Ϸ�\n");
	}
	else
	{
		printf("\n��ȫ���У�");
		for (i=0;i<ProcNum && RetPL[i] != -1;i++)
			printf(" %d",RetPL[i]);//�����ȫ����
		printf("\n�˴���Դ����ɹ�\n");
		for (i=0;i<ResNum;i++)
		{//�޸���Դ�����
			PStbl.Allocation[ProcIndex][i] += R[i];//Allocation++
			PStbl.Need[ProcIndex][i] -= R[i];//Need--
			Available[i] -= R[i];
		}
	}
	for (i=0;i<ProcNum;i++)
	{
		if (IsProcAllOver(i))
			AllOver[i] = 1;//������NeedΪ0������Ϊ��������ȫ����
	}
	
	return 0;
}

int main(void)
{
	char ch;
	int Q = 0;//�˳���־
	srand(GetTickCount());
	InitPSA();//��ʼ��������Դ��
	do
	{
		system("cls");
		printf("1.��Դ����\n");
		printf("2.��ʾ��Դ�����\n");
		printf("0.�˳�\n");
		printf("\n����ѡ��\n");
		ch = getch();
		switch (ch)
		{
			case '1'://��ʾ����������
				ShowPStbl();
				Request();
				break;
			case '2'://��ʾ�����
				ShowPStbl();
				break;
			case '0':
				Q = 1;//��־�����˳�
				printf("\n������˳�\n");
				break;
			default:
				printf("\n�������������룬���������\n");
				break;
		}
		getch();
	}while (Q != 1);
	return 0;
}
