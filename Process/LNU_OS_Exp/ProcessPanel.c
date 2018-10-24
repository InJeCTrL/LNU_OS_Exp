#include<stdio.h>
#include<conio.h>
#include<windows.h>
typedef struct PCB
{
	char ProcessName[50];//进程名
	DWORD PID;//模拟进程的ID
	int Priority;//进程优先级(H0~L5)
	long Totaltime;//总运行时间
	int Status;//进程状态(0:执行 1:就绪 2:阻塞)
	HANDLE hThread;//数据+指令
	HANDLE hEvt;//用于阻塞、转就绪的事件
	struct PCB *next;
}PCB,*PCBList;
typedef struct InInfo
{//生成线程时导入数据
	char ProcessName[50];//指定进程名
	HANDLE hEvt;
}InInfo;
typedef struct ProcessMap
{
	PCB *Running;//正在运行的进程
	PCBList Blocked;//阻塞队列
	PCBList Ready;//就绪队列
}ProcessMap,*ProcessMap_ptr;

HWND MainHwnd = NULL;//主面板窗体句柄
ProcessMap_ptr ProcMap = NULL;//指向进程表
HANDLE hTH = NULL;//进程调度线程
int method = 0;//保存进程调度方式

int InitProcessMap(ProcessMap_ptr *ProcessMap_ptr)
{//初始化进程表
	*ProcessMap_ptr = (ProcessMap*)malloc(sizeof(ProcessMap));
	(*ProcessMap_ptr)->Blocked = (PCBList)malloc(sizeof(PCB));
	(*ProcessMap_ptr)->Ready = (PCBList)malloc(sizeof(PCB));
	(*ProcessMap_ptr)->Blocked->next = NULL;
	(*ProcessMap_ptr)->Ready->next = NULL;
	(*ProcessMap_ptr)->Running = NULL;
	return 0;
}
DWORD WINAPI Thread(LPVOID lp)
{//用线程模拟进程
	MSG msg;//用于消息循环
	HWND newWnd = NULL;//模拟进程的小窗体
	char title[30] = "";//保存标题，保存"<str><TID> - "
	char tTitle[30] = "";//临时字符数组，保存"<str><TID> - <Num>"
	char tNum[5] = "";//临时字符数组，保存字符串形式的单击次数
	int ClickNum = 0;//按钮单击次数
	DWORD TID = GetCurrentThreadId();//本模拟进程（线程）的ID

	strcat(title,((InInfo*)lp)->ProcessName);
	itoa(TID,tTitle,10);//TID转字符串
	strcat(title,tTitle);//title:"<str><TID>"
	strcat(title," - ");//title:"<str><TID> - "
	ZeroMemory(tTitle,30);//临时字符数组清空
	sprintf(tTitle,"%s%s",title,"0");//tTitle:"<str><TID> - 0"
	newWnd = CreateWindow("BUTTON",tTitle,WS_SIZEBOX | WS_CAPTION | WS_VISIBLE ,300,300,150,100,NULL,NULL,(HINSTANCE)GetWindowLong(MainHwnd, GWL_HINSTANCE),NULL);
	while(GetMessage(&msg,NULL,NULL,NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_LBUTTONUP)
		{
			ClickNum++;//单击次数递增
			ZeroMemory(tTitle,30);//临时字符数组清空
			itoa(ClickNum,tNum,10);
			sprintf(tTitle,"%s%s",title,tNum);
			SetWindowText(newWnd,tTitle);//更新显示单击次数
		}
		WaitForSingleObject(((InInfo*)lp)->hEvt,INFINITE);//主面板进程设置阻塞/激活
	}
	return 0;
}
DWORD WINAPI Run(LPVOID lp)
{//调度进程
	long t1,t2;
	PCB *PCB_ptr = NULL,*PCB_ptr_HPrio = NULL,*PCB_ptr_preHPrio = NULL;//PCB临时指针，指向当前就绪队列中优先级最高进程、优先级最高进程前一个进程
	InInfo *tInf = NULL;
	while (1)
	{
		switch (method)
		{
			case 1://FCFS
				PCB_ptr = ProcMap->Ready->next;
				if (!ProcMap->Running && PCB_ptr)//没有进程在执行状态，并且就绪队列有进程
				{
					ProcMap->Running = PCB_ptr;
					ProcMap->Ready->next = PCB_ptr->next;
					if (ProcMap->Running->PID == 0)
					{
						tInf = (InInfo*)malloc(sizeof(InInfo));
						tInf->hEvt = PCB_ptr->hEvt;
						strcpy(tInf->ProcessName,PCB_ptr->ProcessName);
						ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(PCB_ptr->PID));//创建线程
					}
					SetEvent(PCB_ptr->hEvt);//激活启动
				}
				if (ProcMap->Running)//有进程在执行
				{
					ProcMap->Running->Totaltime += 50;//总运行时间递增
				}
				break;
			case 2://非抢占优先级调度
				PCB_ptr = ProcMap->Ready->next;
				if (!ProcMap->Running && PCB_ptr)//没有进程在执行状态，并且就绪队列有进程
				{
					PCB_ptr_preHPrio = ProcMap->Ready;//待调度进程的前驱结点
					PCB_ptr_HPrio = PCB_ptr;//等待调度的当前就绪队列中优先级最高的进程
					while (PCB_ptr->next)
					{//遍历整个就绪队列，找到当前就绪队列中优先级最高的进程
						if (PCB_ptr->next->Priority < PCB_ptr_HPrio->Priority)//遍历到的就绪队列进程的优先级更高
						{
							PCB_ptr_HPrio = PCB_ptr->next;
							PCB_ptr_preHPrio = PCB_ptr;
						}
						PCB_ptr = PCB_ptr->next;
					}
					ProcMap->Running = PCB_ptr_HPrio;//将优先级最高进程调度到处理机
					PCB_ptr_preHPrio->next = PCB_ptr_HPrio->next;
					PCB_ptr_HPrio->next = NULL;//被调度的进程脱离就绪队列
					if (ProcMap->Running->PID == 0)
					{
						tInf = (InInfo*)malloc(sizeof(InInfo));
						tInf->hEvt = ProcMap->Running->hEvt;
						strcpy(tInf->ProcessName,ProcMap->Running->ProcessName);
						ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(ProcMap->Running->PID));//创建线程
					}
					SetEvent(ProcMap->Running->hEvt);//激活启动
				}
				if (ProcMap->Running)//有进程在执行
				{
					ProcMap->Running->Totaltime += 50;//总运行时间递增
				}
				break;
			case 3://抢占优先级调度
				PCB_ptr = ProcMap->Ready->next;
				if (PCB_ptr)//就绪队列有进程
				{
					PCB_ptr_preHPrio = ProcMap->Ready;//待调度进程的前驱结点
					PCB_ptr_HPrio = PCB_ptr;//等待调度的当前就绪队列中优先级最高的进程
					while (PCB_ptr->next)
					{//遍历整个就绪队列，找到当前就绪队列中优先级最高的进程
						if (PCB_ptr->next->Priority < PCB_ptr_HPrio->Priority)//遍历到的就绪队列进程的优先级更高
						{
							PCB_ptr_HPrio = PCB_ptr->next;
							PCB_ptr_preHPrio = PCB_ptr;
						}
						PCB_ptr = PCB_ptr->next;
					}
					if ((ProcMap->Running && PCB_ptr_HPrio->Priority < ProcMap->Running->Priority) ||
						(!ProcMap->Running))
					{//处理机中有正在执行的进程，并且正在运行的进程优先级小于抢占的进程；或处理机无正在运行的进程
						if (ProcMap->Running)
						{//处理机中有正在执行的进程
							ResetEvent(ProcMap->Running->hEvt);//暂停模拟线程
							PCB_ptr = ProcMap->Ready;
							while (PCB_ptr->next)
							{
								PCB_ptr = PCB_ptr->next;
							}
							ProcMap->Running->next = PCB_ptr->next;
							PCB_ptr->next = ProcMap->Running;//被抢占的进程插入到队尾
							ProcMap->Running = NULL;
						}
						ProcMap->Running = PCB_ptr_HPrio;//将优先级最高进程调度到处理机
						PCB_ptr_preHPrio->next = PCB_ptr_HPrio->next;
						PCB_ptr_HPrio->next = NULL;//被调度的进程脱离就绪队列
						if (ProcMap->Running->PID == 0)
						{
							tInf = (InInfo*)malloc(sizeof(InInfo));
							tInf->hEvt = ProcMap->Running->hEvt;
							strcpy(tInf->ProcessName,ProcMap->Running->ProcessName);
							ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(ProcMap->Running->PID));//创建线程
						}
						SetEvent(ProcMap->Running->hEvt);//激活启动
					}
				}
				if (ProcMap->Running)//有进程在执行
				{
					ProcMap->Running->Totaltime += 50;//总运行时间递增
				}
				break;
			case 4://RR调度
				PCB_ptr = ProcMap->Ready->next;
				if (PCB_ptr)
				{//就绪队列有进程
					if (ProcMap->Running)
					{//有进程正在使用处理机，暂停并插入就绪队列尾
						ProcMap->Running->Totaltime += 50;//总运行时间递增
						ResetEvent(ProcMap->Running->hEvt);//暂停模拟线程
						PCB_ptr = ProcMap->Ready;
						while (PCB_ptr->next)
						{
							PCB_ptr = PCB_ptr->next;
						}
						ProcMap->Running->next = PCB_ptr->next;
						PCB_ptr->next = ProcMap->Running;//被抢占的进程插入到队尾
						ProcMap->Running = NULL;
					}
					ProcMap->Running = ProcMap->Ready->next;//就绪队列队头进程分配到处理机
					ProcMap->Ready->next = ProcMap->Running->next;
					ProcMap->Running->next = NULL;//队头进程脱离就绪队列
					if (ProcMap->Running->PID == 0)
					{
						tInf = (InInfo*)malloc(sizeof(InInfo));
						tInf->hEvt = ProcMap->Running->hEvt;
						strcpy(tInf->ProcessName,ProcMap->Running->ProcessName);
						ProcMap->Running->hThread = CreateThread(NULL,0,Thread,tInf,NULL,&(ProcMap->Running->PID));//创建线程
					}
					SetEvent(ProcMap->Running->hEvt);//激活启动
				}
				break;
		}
		Sleep(50);//50ms轮询/时间片
	}
}
int Create(void)
{//模拟创建进程
	HANDLE hEvt;//事件句柄
	PCB *PCB_ptr = NULL;//指向PCB的指针
	PCB *tPCB = NULL;

	tPCB = (PCB*)malloc(sizeof(PCB));
	tPCB->hEvt = CreateEvent(NULL,TRUE,FALSE,NULL);//手动设置，初始状态未触发

	system("cls");
	printf("Input Process Name:");
	scanf("%s",tPCB->ProcessName);//导入进程名
	printf("Input Process Priority(H0~L5):");
	scanf("%d",&tPCB->Priority);//导入进程优先级
	tPCB->Totaltime = 0;//导入进程初始时间
	tPCB->Status = 1;//导入进程初始状态(就绪)
	tPCB->hThread = NULL;
	tPCB->PID  = 0;
	tPCB->next = NULL;

	SuspendThread(hTH);//插入时暂停调度
	PCB_ptr = ProcMap->Ready;
	while (PCB_ptr->next && PCB_ptr->next->next)
	{
		PCB_ptr = PCB_ptr->next;
	}
	tPCB->next = PCB_ptr->next;
	PCB_ptr->next = tPCB;//PCB插入到队尾
	ResumeThread(hTH);//插入结束恢复调度
	return 0;
}
int Block(void)
{//阻塞模拟进程
	DWORD tPID;//保存被阻塞的模拟进程PID
	system("cls");
	if (!ProcMap->Running)//无进程正在运行
		printf("Block failed! - No running process\n");
	else
	{
		SuspendThread(hTH);
		ResetEvent(ProcMap->Running->hEvt);//阻塞进程的运行
		ProcMap->Running->next = ProcMap->Blocked->next;
		ProcMap->Blocked->next = ProcMap->Running;//PCB插入阻塞队列
		tPID = ProcMap->Running->PID;
		ProcMap->Running = NULL;
		ResumeThread(hTH);
		printf("Process %ld has been blocked!\n",tPID);
	}
	getch();
	return 0;
}
int WakeUp(void)
{//唤醒模拟进程
	DWORD tPID;
	PCB *PCB_ptr = ProcMap->Blocked;//指向预恢复的进程PCB的前一个节点
	PCB *PCB_ins = NULL;//指向预恢复的进程PCB
	char t;//用于清空缓冲区
	system("cls");
	printf("Input PID in blocked list to wakeup:");
	scanf("%ld",&tPID);
	while (PCB_ptr->next)
	{
		if (PCB_ptr->next->PID == tPID)
			break;//找到阻塞队列中有此进程
		PCB_ptr = PCB_ptr->next;
	}
	if (!PCB_ptr->next)
	{//阻塞队列中没有找到对应进程
		printf("%ld is no in the blocked list!\n",tPID);
	}
	else
	{//阻塞队列中找到对应进程
		PCB_ins = PCB_ptr->next;
		PCB_ptr->next = PCB_ins->next;//阻塞队列上摘除进程
		PCB_ptr = ProcMap->Ready;
		while (PCB_ptr->next)
		{
			PCB_ptr = PCB_ptr->next;
		}
		PCB_ins->next = PCB_ptr->next;
		PCB_ptr->next = PCB_ins;//就绪队列尾部插入进程
		printf("Process %ld has been waken up to the Ready list!\n",tPID);
	}
	while ((t = getchar()) != '\n' && t != EOF)	;//清空缓冲区
	getch();
	return 0;
}
int Terminate(void)
{//结束模拟进程
	DWORD tPID;//预结束进程的PID
	PCB *PCB_ptr = NULL,*PCB_del = NULL;
	system("cls");
	printf("Input Process ID to terminate process:");
	scanf("%ld",&tPID);
	if (ProcMap->Running && ProcMap->Running->PID == tPID)
	{//检查当前正在运行的进程
		PCB_del = ProcMap->Running;
		TerminateThread(PCB_del->hThread,0);
		ProcMap->Running = NULL;
		free(PCB_del);
		printf("Process has been terminated!\n");
	}
	else
	{//搜索就绪队列
		PCB_ptr = ProcMap->Ready;
		while (PCB_ptr->next)
		{
			if (PCB_ptr->next->PID == tPID)
				break;
			PCB_ptr = PCB_ptr->next;
		}
		if (!PCB_ptr->next)
		{//就绪队列没找到，搜索阻塞队列
			PCB_ptr = ProcMap->Blocked;
			while (PCB_ptr->next)
			{
				if (PCB_ptr->next->PID == tPID)
					break;
				PCB_ptr = PCB_ptr->next;
			}
			if (!PCB_ptr->next)
			{//阻塞队列没找到，无此进程
				printf("Process not found!\n");
			}
			else
			{//阻塞队列中找到
				PCB_del = PCB_ptr->next;
				PCB_ptr->next = PCB_del->next;//阻塞队列中摘除PCB
				TerminateThread(PCB_del->hThread,0);//结束线程实体
				free(PCB_del);//释放PCB
				printf("Process has been terminated!\n");
			}
		}
		else
		{//就绪队列中找到
			PCB_del = PCB_ptr->next;
			PCB_ptr->next = PCB_del->next;//就绪队列中摘除PCB
			SuspendThread(hTH);
			TerminateThread(PCB_del->hThread,0);//结束线程实体
			free(PCB_del);//释放PCB
			ResumeThread(hTH);
			printf("Process has been terminated!\n");
		}
	}
	getch();
	return 0;
}
int ShowProcMap(void)
{//显示所有进程
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
{//清除所有模拟进程，清空所有PCB
	PCB *PCB_ptr = NULL,*PCB_del = NULL;
	TerminateThread(hTH,0);//停止调度线程
	if (ProcMap->Running)
	{//终止正在运行的进程
		TerminateThread(ProcMap->Running->hThread,0);
		free(ProcMap->Running);
		ProcMap->Running = NULL;
	}
	//清空阻塞进程
	PCB_ptr = ProcMap->Blocked;
	while (PCB_ptr->next)
	{
		PCB_del = PCB_ptr->next;
		TerminateThread(PCB_del->hThread,0);
		PCB_ptr->next = PCB_del->next;
		free(PCB_del);
	}
	free(PCB_ptr);
	//清空就绪进程
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
{//选择调度方式
	char ch;//选项
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
				method = 1;//调度方式为1
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//FCFS调度
				break;
			case '2':
				method = 2;//调度方式为2
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//非抢占优先级调度
				break;
			case '3':
				method = 3;//调度方式为3
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//抢占优先级调度
				break;
			case '4':
				method = 4;//调度方式为4
				hTH = CreateThread(NULL,0,Run,NULL,0,NULL);//时间片轮询调度
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
	char ch;//选项
	
	InitProcessMap(&ProcMap);//创建进程表
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
			case '1'://创建进程
				Create();
				break;
			case '2'://阻塞进程
				Block();
				break;
			case '3'://唤醒进程
				WakeUp();
				break;
			case '4':
				Terminate();//结束进程
				break;
			case '5':
				ShowProcMap();//显示所有进程
				break;
			case '6':
				ClearUp();//全清进程
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
