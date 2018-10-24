#include<stdio.h>
#include<conio.h>
#include<windows.h>
typedef struct PCB
{
	char ProcessName[50];//������
	DWORD PID;//ģ����̵�ID
	int Priority;//�������ȼ�(H0~L5)
	long Totaltime;//������ʱ��
	int Status;//����״̬(0:ִ�� 1:���� 2:����)
	HANDLE hThread;//����+ָ��
	HANDLE hEvt;//����������ת�������¼�
	struct PCB *next;
}PCB,*PCBList;
typedef struct InInfo
{//�����߳�ʱ��������
	char ProcessName[50];//ָ��������
	HANDLE hEvt;
}InInfo;
typedef struct ProcessMap
{
	PCB *Running;//�������еĽ���
	PCBList Blocked;//��������
	PCBList Ready;//��������
}ProcessMap,*ProcessMap_ptr;

HWND MainHwnd = NULL;//����崰����
ProcessMap_ptr ProcMap = NULL;//ָ����̱�
HANDLE hTH = NULL;//���̵����߳�
int method = 0;//������̵��ȷ�ʽ

int InitProcessMap(ProcessMap_ptr *ProcessMap_ptr)
{//��ʼ�����̱�
	*ProcessMap_ptr = (ProcessMap*)malloc(sizeof(ProcessMap));
	(*ProcessMap_ptr)->Blocked = (PCBList)malloc(sizeof(PCB));
	(*ProcessMap_ptr)->Ready = (PCBList)malloc(sizeof(PCB));
	(*ProcessMap_ptr)->Blocked->next = NULL;
	(*ProcessMap_ptr)->Ready->next = NULL;
	(*ProcessMap_ptr)->Running = NULL;
	return 0;
}
DWORD WINAPI Thread(LPVOID lp)
{//���߳�ģ�����
	MSG msg;//������Ϣѭ��
	HWND newWnd = NULL;//ģ����̵�С����
	char title[30] = "";//������⣬����"<str><TID> - "
	char tTitle[30] = "";//��ʱ�ַ����飬����"<str><TID> - <Num>"
	char tNum[5] = "";//��ʱ�ַ����飬�����ַ�����ʽ�ĵ�������
	int ClickNum = 0;//��ť��������
	DWORD TID = GetCurrentThreadId();//��ģ����̣��̣߳���ID

	strcat(title,((InInfo*)lp)->ProcessName);
	itoa(TID,tTitle,10);//TIDת�ַ���
	strcat(title,tTitle);//title:"<str><TID>"
	strcat(title," - ");//title:"<str><TID> - "
	ZeroMemory(tTitle,30);//��ʱ�ַ��������
	sprintf(tTitle,"%s%s",title,"0");//tTitle:"<str><TID> - 0"
	newWnd = CreateWindow("BUTTON",tTitle,WS_SIZEBOX | WS_CAPTION | WS_VISIBLE ,300,300,150,100,NULL,NULL,(HINSTANCE)GetWindowLong(MainHwnd, GWL_HINSTANCE),NULL);
	while(GetMessage(&msg,NULL,NULL,NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_LBUTTONUP)
		{
			ClickNum++;//������������
			ZeroMemory(tTitle,30);//��ʱ�ַ��������
			itoa(ClickNum,tNum,10);
			sprintf(tTitle,"%s%s",title,tNum);
			SetWindowText(newWnd,tTitle);//������ʾ��������
		}
		WaitForSingleObject(((InInfo*)lp)->hEvt,INFINITE);//����������������/����
	}
	return 0;
}
DWORD WINAPI Run(LPVOID lp)
{//���Ƚ���
	long t1,t2;
	PCB *PCB_ptr = NULL,*PCB_ptr_HPrio = NULL,*PCB_ptr_preHPrio = NULL;//PCB��ʱָ�룬ָ��ǰ�������������ȼ���߽��̡����ȼ���߽���ǰһ������
	InInfo *tInf = NULL;
	while (1)
	{
		switch (method)
		{
			case 1://FCFS
				PCB_ptr = ProcMap->Ready->next;
				if (!ProcMap->Running && PCB_ptr)//û�н�����ִ��״̬�����Ҿ��������н���
				{
					ProcMap->Running = PCB_ptr;
					ProcMap->Ready->next = PCB_ptr->next;
					if (ProcMap->Running->PID == 0)
					{
						tInf = (InInfo*)malloc(sizeof(InInfo));
						tInf->hEvt = PCB_ptr->hEvt;
						strcpy(tInf->ProcessName,PCB_ptr->ProcessName);
						ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(PCB_ptr->PID));//�����߳�
					}
					SetEvent(PCB_ptr->hEvt);//��������
				}
				if (ProcMap->Running)//�н�����ִ��
				{
					ProcMap->Running->Totaltime += 50;//������ʱ�����
				}
				break;
			case 2://����ռ���ȼ�����
				PCB_ptr = ProcMap->Ready->next;
				if (!ProcMap->Running && PCB_ptr)//û�н�����ִ��״̬�����Ҿ��������н���
				{
					PCB_ptr_preHPrio = ProcMap->Ready;//�����Ƚ��̵�ǰ�����
					PCB_ptr_HPrio = PCB_ptr;//�ȴ����ȵĵ�ǰ�������������ȼ���ߵĽ���
					while (PCB_ptr->next)
					{//���������������У��ҵ���ǰ�������������ȼ���ߵĽ���
						if (PCB_ptr->next->Priority < PCB_ptr_HPrio->Priority)//�������ľ������н��̵����ȼ�����
						{
							PCB_ptr_HPrio = PCB_ptr->next;
							PCB_ptr_preHPrio = PCB_ptr;
						}
						PCB_ptr = PCB_ptr->next;
					}
					ProcMap->Running = PCB_ptr_HPrio;//�����ȼ���߽��̵��ȵ������
					PCB_ptr_preHPrio->next = PCB_ptr_HPrio->next;
					PCB_ptr_HPrio->next = NULL;//�����ȵĽ��������������
					if (ProcMap->Running->PID == 0)
					{
						tInf = (InInfo*)malloc(sizeof(InInfo));
						tInf->hEvt = ProcMap->Running->hEvt;
						strcpy(tInf->ProcessName,ProcMap->Running->ProcessName);
						ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(ProcMap->Running->PID));//�����߳�
					}
					SetEvent(ProcMap->Running->hEvt);//��������
				}
				if (ProcMap->Running)//�н�����ִ��
				{
					ProcMap->Running->Totaltime += 50;//������ʱ�����
				}
				break;
			case 3://��ռ���ȼ�����
				PCB_ptr = ProcMap->Ready->next;
				if (PCB_ptr)//���������н���
				{
					PCB_ptr_preHPrio = ProcMap->Ready;//�����Ƚ��̵�ǰ�����
					PCB_ptr_HPrio = PCB_ptr;//�ȴ����ȵĵ�ǰ�������������ȼ���ߵĽ���
					while (PCB_ptr->next)
					{//���������������У��ҵ���ǰ�������������ȼ���ߵĽ���
						if (PCB_ptr->next->Priority < PCB_ptr_HPrio->Priority)//�������ľ������н��̵����ȼ�����
						{
							PCB_ptr_HPrio = PCB_ptr->next;
							PCB_ptr_preHPrio = PCB_ptr;
						}
						PCB_ptr = PCB_ptr->next;
					}
					if ((ProcMap->Running && PCB_ptr_HPrio->Priority < ProcMap->Running->Priority) ||
						(!ProcMap->Running))
					{//�������������ִ�еĽ��̣������������еĽ������ȼ�С����ռ�Ľ��̣���������������еĽ���
						if (ProcMap->Running)
						{//�������������ִ�еĽ���
							ResetEvent(ProcMap->Running->hEvt);//��ͣģ���߳�
							PCB_ptr = ProcMap->Ready;
							while (PCB_ptr->next)
							{
								PCB_ptr = PCB_ptr->next;
							}
							ProcMap->Running->next = PCB_ptr->next;
							PCB_ptr->next = ProcMap->Running;//����ռ�Ľ��̲��뵽��β
							ProcMap->Running = NULL;
						}
						ProcMap->Running = PCB_ptr_HPrio;//�����ȼ���߽��̵��ȵ������
						PCB_ptr_preHPrio->next = PCB_ptr_HPrio->next;
						PCB_ptr_HPrio->next = NULL;//�����ȵĽ��������������
						if (ProcMap->Running->PID == 0)
						{
							tInf = (InInfo*)malloc(sizeof(InInfo));
							tInf->hEvt = ProcMap->Running->hEvt;
							strcpy(tInf->ProcessName,ProcMap->Running->ProcessName);
							ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(ProcMap->Running->PID));//�����߳�
						}
						SetEvent(ProcMap->Running->hEvt);//��������
					}
				}
				if (ProcMap->Running)//�н�����ִ��
				{
					ProcMap->Running->Totaltime += 50;//������ʱ�����
				}
				break;
			case 4://RR����
				PCB_ptr = ProcMap->Ready->next;
				if (PCB_ptr)
				{//���������н���
					if (ProcMap->Running)
					{//�н�������ʹ�ô��������ͣ�������������β
						ProcMap->Running->Totaltime += 50;//������ʱ�����
						ResetEvent(ProcMap->Running->hEvt);//��ͣģ���߳�
						PCB_ptr = ProcMap->Ready;
						while (PCB_ptr->next)
						{
							PCB_ptr = PCB_ptr->next;
						}
						ProcMap->Running->next = PCB_ptr->next;
						PCB_ptr->next = ProcMap->Running;//����ռ�Ľ��̲��뵽��β
						ProcMap->Running = NULL;
					}
					ProcMap->Running = ProcMap->Ready->next;//�������ж�ͷ���̷��䵽�����
					ProcMap->Ready->next = ProcMap->Running->next;
					ProcMap->Running->next = NULL;//��ͷ���������������
					if (ProcMap->Running->PID == 0)
					{
						tInf = (InInfo*)malloc(sizeof(InInfo));
						tInf->hEvt = ProcMap->Running->hEvt;
						strcpy(tInf->ProcessName,ProcMap->Running->ProcessName);
						ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(ProcMap->Running->PID));//�����߳�
					}
					SetEvent(ProcMap->Running->hEvt);//��������
				}
				break;
		}
		Sleep(50);//50ms��ѯ/ʱ��Ƭ
	}
}
int Create(void)
{//ģ�ⴴ������
	HANDLE hEvt;//�¼����
	PCB *PCB_ptr = NULL;//ָ��PCB��ָ��
	PCB *tPCB = NULL;

	tPCB = (PCB*)malloc(sizeof(PCB));
	tPCB->hEvt = CreateEvent(NULL,TRUE,FALSE,NULL);//�ֶ����ã���ʼ״̬δ����

	system("cls");
	printf("Input Process Name:");
	scanf("%s",tPCB->ProcessName);//���������
	printf("Input Process Priority(H0~L5):");
	scanf("%d",&tPCB->Priority);//����������ȼ�
	tPCB->Totaltime = 0;//������̳�ʼʱ��
	tPCB->Status = 1;//������̳�ʼ״̬(����)
	tPCB->hThread = NULL;
	tPCB->PID  = 0;
	tPCB->next = NULL;

	SuspendThread(hTH);//����ʱ��ͣ����
	PCB_ptr = ProcMap->Ready;
	while (PCB_ptr->next && PCB_ptr->next->next)
	{
		PCB_ptr = PCB_ptr->next;
	}
	tPCB->next = PCB_ptr->next;
	PCB_ptr->next = tPCB;//PCB���뵽��β
	ResumeThread(hTH);//��������ָ�����
	return 0;
}
int Block(void)
{//����ģ�����
	DWORD tPID;//���汻������ģ�����PID
	system("cls");
	if (!ProcMap->Running)//�޽�����������
		printf("Block failed! - No running process\n");
	else
	{
		SuspendThread(hTH);
		ResetEvent(ProcMap->Running->hEvt);//�������̵�����
		ProcMap->Running->next = ProcMap->Blocked->next;
		ProcMap->Blocked->next = ProcMap->Running;//PCB������������
		tPID = ProcMap->Running->PID;
		ProcMap->Running = NULL;
		ResumeThread(hTH);
		printf("Process %ld has been blocked!\n",tPID);
	}
	getch();
	return 0;
}
int WakeUp(void)
{//����ģ�����
	DWORD tPID;
	PCB *PCB_ptr = ProcMap->Blocked;//ָ��Ԥ�ָ��Ľ���PCB��ǰһ���ڵ�
	PCB *PCB_ins = NULL;//ָ��Ԥ�ָ��Ľ���PCB
	char t;//������ջ�����
	system("cls");
	printf("Input PID in blocked list to wakeup:");
	scanf("%ld",&tPID);
	while (PCB_ptr->next)
	{
		if (PCB_ptr->next->PID == tPID)
			break;//�ҵ������������д˽���
		PCB_ptr = PCB_ptr->next;
	}
	if (!PCB_ptr->next)
	{//����������û���ҵ���Ӧ����
		printf("%ld is no in the blocked list!\n",tPID);
	}
	else
	{//�����������ҵ���Ӧ����
		PCB_ins = PCB_ptr->next;
		PCB_ptr->next = PCB_ins->next;//����������ժ������
		PCB_ptr = ProcMap->Ready;
		while (PCB_ptr->next)
		{
			PCB_ptr = PCB_ptr->next;
		}
		PCB_ins->next = PCB_ptr->next;
		PCB_ptr->next = PCB_ins;//��������β���������
		printf("Process %ld has been waken up to the Ready list!\n",tPID);
	}
	while ((t = getchar()) != '\n' && t != EOF)	;//��ջ�����
	getch();
	return 0;
}
int Terminate(void)
{//����ģ�����
	DWORD tPID;//Ԥ�������̵�PID
	PCB *PCB_ptr = NULL,*PCB_del = NULL;
	system("cls");
	printf("Input Process ID to terminate process:");
	scanf("%ld",&tPID);
	if (ProcMap->Running && ProcMap->Running->PID == tPID)
	{//��鵱ǰ�������еĽ���
		PCB_del = ProcMap->Running;
		TerminateThread(PCB_del->hThread,0);
		ProcMap->Running = NULL;
		free(PCB_del);
		printf("Process has been terminated!\n");
	}
	else
	{//������������
		PCB_ptr = ProcMap->Ready;
		while (PCB_ptr->next)
		{
			if (PCB_ptr->next->PID == tPID)
				break;
			PCB_ptr = PCB_ptr->next;
		}
		if (!PCB_ptr->next)
		{//��������û�ҵ���������������
			PCB_ptr = ProcMap->Blocked;
			while (PCB_ptr->next)
			{
				if (PCB_ptr->next->PID == tPID)
					break;
				PCB_ptr = PCB_ptr->next;
			}
			if (!PCB_ptr->next)
			{//��������û�ҵ����޴˽���
				printf("Process not found!\n");
			}
			else
			{//�����������ҵ�
				PCB_del = PCB_ptr->next;
				PCB_ptr->next = PCB_del->next;//����������ժ��PCB
				TerminateThread(PCB_del->hThread,0);//�����߳�ʵ��
				free(PCB_del);//�ͷ�PCB
				printf("Process has been terminated!\n");
			}
		}
		else
		{//�����������ҵ�
			PCB_del = PCB_ptr->next;
			PCB_ptr->next = PCB_del->next;//����������ժ��PCB
			SuspendThread(hTH);
			TerminateThread(PCB_del->hThread,0);//�����߳�ʵ��
			free(PCB_del);//�ͷ�PCB
			ResumeThread(hTH);
			printf("Process has been terminated!\n");
		}
	}
	getch();
	return 0;
}
int ShowProcMap(void)
{//��ʾ���н���
	PCB *PCB_ptr = NULL;
	system("cls");
	printf("Running Process:\n");
	if (ProcMap->Running)
		printf("ProcessName: %s\nPID: %ld\nRunTime: %ld ms\nPriority: %d\n",ProcMap->Running->ProcessName,ProcMap->Running->PID,ProcMap->Running->Totaltime,ProcMap->Running->Priority);
	printf("-----------------------------------------\nReady Processes:\n");
	PCB_ptr = ProcMap->Ready;
	while (PCB_ptr->next)
	{
		PCB_ptr = PCB_ptr->next;
		printf("ProcessName: %s\nPID: %ld\nRunTime: %ld ms\nPriority: %d\n\n",PCB_ptr->ProcessName,PCB_ptr->PID,PCB_ptr->Totaltime,PCB_ptr->Priority);
	}
	printf("-----------------------------------------\nBlocked Processes:\n");
	PCB_ptr = ProcMap->Blocked;
	while (PCB_ptr->next)
	{
		PCB_ptr = PCB_ptr->next;
		printf("ProcessName: %s\nPID: %ld\nRunTime: %ld ms\nPriority: %d\n\n",PCB_ptr->ProcessName,PCB_ptr->PID,PCB_ptr->Totaltime,PCB_ptr->Priority);
	}
	printf("-----------------------------------------\nPress any key to continue..\n");
	getch();
	return 0;
}
int ClearUp(void)
{//�������ģ����̣��������PCB
	PCB *PCB_ptr = NULL,*PCB_del = NULL;
	TerminateThread(hTH,0);//ֹͣ�����߳�
	if (ProcMap->Running)
	{//��ֹ�������еĽ���
		TerminateThread(ProcMap->Running->hThread,0);
		free(ProcMap->Running);
		ProcMap->Running = NULL;
	}
	//�����������
	PCB_ptr = ProcMap->Blocked;
	while (PCB_ptr->next)
	{
		PCB_del = PCB_ptr->next;
		TerminateThread(PCB_del->hThread,0);
		PCB_ptr->next = PCB_del->next;
		free(PCB_del);
	}
	free(PCB_ptr);
	//��վ�������
	PCB_ptr = ProcMap->Ready;
	while (PCB_ptr->next)
	{
		PCB_del = PCB_ptr->next;
		TerminateThread(PCB_del->hThread,0);
		PCB_ptr->next = PCB_del->next;
		free(PCB_del);
	}
	free(PCB_ptr);
	return 0;
}

int SelectRunMode(void)
{//ѡ����ȷ�ʽ
	char ch;//ѡ��
	do
	{
		system("cls");
		printf("1.FCFS\n");
		printf("2.NP-PSA\n");
		printf("3.P-PSA\n");
		printf("4.RR\n");
		printf("Press KeyCode(1-4):");
		ch = getch();
		printf("%c",ch);
		switch (ch)
		{
			case '1':
				method = 1;//���ȷ�ʽΪ1
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//FCFS����
				break;
			case '2':
				method = 2;//���ȷ�ʽΪ2
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//����ռ���ȼ�����
				break;
			case '3':
				method = 3;//���ȷ�ʽΪ3
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//��ռ���ȼ�����
				break;
			case '4':
				method = 4;//���ȷ�ʽΪ4
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//ʱ��Ƭ��ѯ����
				break;
			default:
				printf("\nError Input!\nPress any key to retry.\n");
				getch();
				break;
		}
	}while(method == 0);
	return 0;
}

int main(void)
{
	char ch;//ѡ��
	
	InitProcessMap(&ProcMap);//�������̱�
	MainHwnd = GetConsoleWindow();
	SelectRunMode();
	
	do
	{
		system("cls");
		printf("1.CreateProcess\n");
		printf("2.Block\n");
		printf("3.WakeUp\n");
		printf("4.Terminate\n");
		printf("5.Process List\n");
		printf("6.Exit");
		printf("\nPress KeyCode(1-6):");
		ch = getch();
		printf("%c",ch);
		switch (ch)
		{
			case '1'://��������
				Create();
				break;
			case '2'://��������
				Block();
				break;
			case '3'://���ѽ���
				WakeUp();
				break;
			case '4':
				Terminate();//��������
				break;
			case '5':
				ShowProcMap();//��ʾ���н���
				break;
			case '6':
				ClearUp();//ȫ�����
				break;
			default:
				printf("\nError Input!\n");
				getch();
				break;
		}
	}while(ch != '6');
	printf("\nPress any key to exit!\n");
	getch();
	return 0;
}
