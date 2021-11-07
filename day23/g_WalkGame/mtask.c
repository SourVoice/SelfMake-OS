/* 多任务管理 */

#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
void task_idle(void) /*闲置任务(放于level最下层(卫兵思想)) idle for "闲置"*/
{
	for (;;)
	{
		io_hlt();
	}
}

struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv]; /*当前level*/
	return tl->tasks[tl->now];								 /*返回当前任务*/
}
void task_add(struct TASK *task) /*level中添加一个任务*/
{
	struct TASKLEVEL *tl = &taskctl->level[task->level]; /*任务所在LEVEL*/
	if (tl->running < MAX_TASKS_LV)						 /*判断在一个level中是否添加了100个以上的level*/
	{
		tl->tasks[tl->running] = task;
		tl->running++; /*当前所在LEVEL任务++*/
		task->flags = 2;
	}
	return;
}
void task_remove(struct TASK *task) /*level中删除一个任务*/
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	for (i = 0; i < tl->running; i++)
	{
		if (tl->tasks[i] == task) /*找到当前任务所在位置*/
		{
			break;
		}
	}
	tl->running--;
	if (i < tl->now)
	{
		tl->now--;
	}
	if (tl->now >= tl->running)
	{
		tl->now = 0;
	}
	task->flags = 1; /*当前任务休眠*/
	/*移动任务*/
	for (; i < tl->running; i++)
	{
		tl->tasks[i] = tl->tasks[i + 1];
	}
	return;
}
void task_switchsub(void) /*判断任务切换时切换到的LEVEL*/
{
	int i;
	for (i = 0; i < MAX_TASKLEVELS; i++)
	{
		if (taskctl->level[i].running > 0)
		{
			break;
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}

struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++)
	{
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) /*所有程序都记录在GDT当中*/
	{
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0; /*sel for selector,在GDT中选择TASK_GDT0+i段*/
	}

	task = task_alloc();
	task->flags = 2;	/*任务活动标志*/
	task->priority = 2; /*0.02s间隔*/
	task->level = 0;	/*最高level*/
	task_add(task);
	task_switchsub(); /*设置level*/
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);

	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int)&task_idle;
	idle->tss.es = 2 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1); /*放在最下层*/

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
			task->tss.ss0 = 0;
			return task;
		}
	}
	return 0; /*任务均在使用(超过MAX_TASKS)*/
}
void task_run(struct TASK *task, int level, int priority) /*task添加到taskctl表中*/
{
	if (level < 0)
	{
		level = task->level; /*不改变level*/
	}
	if (priority > 0) /*优先级0时不改变以经设定的优先级*/
	{
		task->priority = priority;
	}
	if (task->flags == 2 && task->level != level) /*改变活动中的level*/
	{
		task_remove(task); /*该处执行后flag值变为1,下面的if语句也会执行*/
	}
	if (task->flags != 2)
	{
		/*从休眠状态唤醒*/
		task->level = level;
		task_add(task);
	}
	taskctl->lv_change = 1; /*下次任务切换时检测level*/
	return;
}
void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running) /*当now走完一遍则前置*/
	{
		tl->now = 0;
	}
	if (taskctl->lv_change != 0)
	{
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task)
	{
		farjmp(0, new_task->sel);
	}
	return;
}
void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == 2)
	{
		now_task = task_now();
		task_remove(task);	  /*函数内flags变为1*/
		if (task == now_task) /*自己休眠则需要任务切换*/
		{
			task_switchsub();
			now_task = task_now(); /*在设定后获取当前任务的值*/
			farjmp(0, now_task->sel);
		}
	}
	return;
}
