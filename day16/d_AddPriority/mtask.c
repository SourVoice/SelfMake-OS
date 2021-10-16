/* 多任务管理 */

#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMEMT_DESCRIPTOR *)ADR_GDT;
	taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) /*所有程序都记录在GDT当中*/
	{
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8; /*sel for selector,在GDT中选择TASK_GDT0+i段*/
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2;	/*任务活动标志*/
	task->priority = 2; /*0.02s间隔*/
	taskctl->running = 1;
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);

	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	return task;
}
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++)
	{
		if (taskctl->tasks0[i].flags == 0)
		{
			task = &taskctl->tasks0[i];
			/*(任务被分配到GDT段中时)寄存器初始值*/
			task->flags = 1;			   /*正在使用的标志*/
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0;			   /*这里先置为0*/
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /*任务均在使用(超过MAX_TASKS)*/
}
void task_run(struct TASK *task, int priority) /*task添加到taskctl表中*/
{
	if (priority > 0) /*优先级0时不改变以经设定的优先级*/
	{
		task->priority = priority;
	}
	if (task->flags != 2)
	{
		task->flags = 2;
		taskctl->tasks[taskctl->running] = task;
		++taskctl->running;
	}
	return;
}
void task_switch(void)
{
	struct TASK *task;
	taskctl->now++;
	if (taskctl->now == taskctl->running) /*当now走完一遍则前置*/
	{
		taskctl->now = 0;
	}
	task = taskctl->tasks[taskctl->now];
	timer_settime(task_timer, task->priority);
	if (taskctl->running >= 2) /*任务进程大于1个*/
	{
		farjmp(0, task->sel);
	}
	return;
}
void task_sleep(struct TASK *task)
{
	int i;
	char ts = 0;
	if (task->flags == 2) /*任务处于唤醒状态*/
	{
		if (task == taskctl->tasks[taskctl->now]) /*任务自己使自己休眠*/
		{
			ts = 1;
		}
		for (i = 0; i < taskctl->running; i++)
		{
			if (taskctl->tasks[i] == task)
			{
				break;
			}
		}
		taskctl->running--;
		if (i < taskctl->now)
		{
			taskctl->now--;
		}
		/*移动成员*/
		for (; i < taskctl->running; i++)
		{
			taskctl->tasks[i] = taskctl->tasks[i + 1];
		}
		task->flags = 1; /*1 for not working*/
		if (ts != 0)
		{
			/*任务切换*/
			if (taskctl->now >= taskctl->running) /*前置now值*/
			{
				taskctl->now = 0;
			}
			farjmp(0, taskctl->tasks[taskctl->now]->sel);
		}
	}
	return;
}
