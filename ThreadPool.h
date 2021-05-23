#pragma once
#include<list>
#include<queue>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<future>

using namespace std;

//��������࣬ʹ��ʱ��Ҫ�̳У�Ȼ����дrun����
class Task {
private:
	int m_priority;		//��������ȼ�
public:
	enum PRIORITY {
		MIN = 1,
		NORMAL = 15,
		MAX = 25
	};
	Task() {};
	Task(PRIORITY priority) :m_priority(priority) {}
	~Task() {};
	void setPriority(PRIORITY priority) { m_priority = priority; }

	virtual void run() = 0;			//������
};

//�����̡߳�1.�ɵ���ʹ�ã��̳и��࣬��дrun()������2.����һ�����а������û�ָ�룬��������
class WorkThread {
private:
	thread m_thread;		//�����߳�
	Task* m_task;			//����ָ��
	mutex m_mutexThread;	//���������Ƿ�ִ�еĻ�����
	mutex m_mutexCondition;	//����������
	mutex m_mutexTask;		//���ڷ�������Ļ�����
	condition_variable m_condition;		//������������
	atomic<bool> m_bRunning;			//�ж��Ƿ����е�ԭ�ӱ���
	bool m_bStop;

protected:
	virtual void run();		//�߳���ں���
public:
	WorkThread();
	~WorkThread();
	//��ʾɾ��Ĭ�Ͽ������캯���������ܽ���Ĭ�Ͽ���
	WorkThread(const WorkThread &thread) = delete; 
	WorkThread &operator=(const WorkThread &thread) = delete;
	//ɾ����ֵ���ú�����������move����,��ֹ�����߳�����Ȩת�ơ�
	WorkThread(const WorkThread &&thread) = delete;
	WorkThread &operator=(const WorkThread &&thread) = delete;

	bool assign(Task *task);	//��������
	thread::id getThreadID();	//��ȡ�߳�id
	void stop();		//ֹͣ�߳����У������߳�����
	void notify();		//���������߳�
	void notify_all();	//�������������߳�
	bool isExecuting();	//�ж��߳��Ƿ�������

};

//�����߳��б��ڲ���֤�̰߳�ȫ���ⲿʹ�ò��ü���
class LeisureThreadList {
	list<WorkThread *> m_threadList;	//�߳��б�
	mutex m_mutexThread;		//�߳��б���ʻ�����
	void assign(const size_t counts);	//�����߳�

public:
	LeisureThreadList(const size_t counts);
	~LeisureThreadList();
	void push(WorkThread *thread);		//����߳�
	WorkThread *top();		//���ص�һ���߳�ָ��
	void pop();		//ɾ����һ���߳�
	size_t size();	//�����̸߳���
	void stop();	//ֹͣ����
};

//�̳߳��࣬�ڲ�����Ϊ�̰߳�ȫ��
/*
��������ʱֻҪ����addTask����������񼴿ɣ�������Զ����С�
����̳߳��˳����������exit()������
�̳߳ؿ�����ִͣ�У�����stop()���ɣ����¿�ʼ������Ҫ����start()��
*/
class ThreadPool {
	thread m_thread;		//�̳߳ص���������̣߳��������߳�
	LeisureThreadList m_leisureThreadList;	//�߳��б�
	queue<Task *> m_taskList;	//�������
	atomic<bool> m_bRunning;	//���������̳߳���ͣ�����ı�־λ
	atomic<bool> m_bEnd;		//��������ֹ�̳߳����еı�־λ
	atomic<size_t> m_threadCounts;	//�߳�����
	condition_variable m_condition_task;	//������������
	condition_variable m_condition_thread;	//�߳��б���������
	condition_variable m_condition_running;	//������������
	mutex m_runningMutex;		//���п��Ʊ�����
	mutex m_mutexThread;		//���л�����
	mutex m_taskMutex;			//���������б�Ļ�����
	bool m_bExit;		//�Ƿ��˳���־λ

	void run();			//�̳߳����̺߳���

public:
	ThreadPool(const size_t counts);
	~ThreadPool();
	size_t threadCounts();
	bool isRunning();	//�߳��Ƿ�������������
	void addTask(Task *task);	//�������

	//�̳߳ؿ�ʼ���������̳߳ش������õ��øú������ú�����stop()�������ʹ��
	void start();	//�̳߳ؿ�ʼִ��

	//�̳߳���ͣ������ȣ�����Ӱ��������ӡ���Ҫ��ʼ������ȣ���Ҫ����start()������
	void stop();	//�̳߳���ͣ����

	//�ú��������̳߳��е��������񶼷����ȥ�Ժ󣬽����ֳɳԵ����С�
	//ͬ�����߳��б��е��̻߳����Լ�������ִ����Ϻ����Ƴ���
	void exit();	//�̳߳��˳�

};
