#include"ThreadPool.h"
#include<iostream>

//�����߳�
WorkThread::WorkThread() :m_task{ nullptr }, m_bStop{ false } {
	m_bRunning.store(true);			//˵���ö��󴴽���Ϳ�ʼ����������
	//��ʼ��ִ���߳�
	m_thread = thread(&WorkThread::run, this);
}

WorkThread::~WorkThread() {
	if (!m_bStop) {				//ֹͣ�߳����У�����stop()����
		stop();
	}
	if (m_thread.joinable()) {	//�ȴ�ִ��������߳�m_thread = thread(&WorkThread::run, this);���������ٸ���ʵ�����������Դ
		m_thread.join();
	}
}

//�����̷߳�������
bool WorkThread::assign(Task * task) {
	m_mutexTask.lock();
	if (m_task != nullptr) {	//����Ϊ�գ�����ʧ��
		m_mutexTask.unlock();
		return false;
	}
	m_task = task;
	m_mutexTask.unlock();
	m_condition.notify_one();	//֪ͨ�̣߳����Ѵ����Ĺ����߳�m_thread����ں���run()������ʹ����������m_condition�����ȴ���
	return true;
}

//�����̵߳���ں���
void WorkThread::run() {
	while (true) {
		if (!m_bRunning) {			//δ����,�˳�
			m_mutexTask.lock();
			if (m_task == nullptr) {	//����δ��������,����whileѭ���������߳�
				m_mutexTask.unlock();
				break;	//break���ĵ��ã�������ѭ��(while��for)���߷�֧���(switch)���á�
			}
			m_mutexTask.unlock();
		}
		Task * task = nullptr;
		//�ȴ���������߳�δ�˳�����û���������߳�����
		{
			unique_lock<mutex> lock(m_mutexTask);	//��Ҫ�ж�m_task,����
			//�ȴ��ź�,û�������Ҵ��߳��������У����������߳�
			/*
			wait()�����������ĳ�Ա������������һ������������ڶ�������lambda���ʽ����ֵ��false����ôwait��������һ�����������������������������С�
			������ʲôʱ���أ�����������ĳ���̵߳���notify_one������Ա����Ϊֹ���������true����ôwait()ֱ�ӷ��ء�
			���û�еڶ����������͸�Ĭ�ϵڶ�����������falseЧ��һ����
			*/
			m_condition.wait(lock,
				[this]() {
				return !((m_task == nullptr) && this->m_bRunning.load()); });
			task = m_task;
			m_task = nullptr;
			if (task == nullptr) {	//����m_bRunningΪfalseʱ��˵���߳���ͣ���˳�����ʱ��m_taskΪ�գ����������˳���������Ҫִ�����ѷ�����������˳�
				continue;		//����Ϊ�գ�������������д���
			}
		}
		task->run();	//ִ������
		delete task;	//ִ�����ͷ���Դ
		task = nullptr;
	}
}

thread::id WorkThread::getThreadID() {
	return this_thread::get_id();
}

void WorkThread::stop() {
	m_bRunning.store(false);	//���ö����ֹͣ��־λ���߳�ֹͣ����
	m_mutexThread.lock();
	if (m_thread.joinable()) {
		/*
		һ��Ҫ�����ñ�־λ��Ȼ��֪ͨ���Ѹö�����̣߳�run()������m_condition���ܴ��ڵȴ�״̬��
		*/
		m_condition.notify_all();	//����run()����������m_bRunning��Ϊfasle,��run�����ж��ܷ��˳�
		m_thread.join();
	}
	m_mutexThread.unlock();
	m_bStop = true;
}

//�˴����ã�������ֻ����һ���̣߳��ɲ�����
void WorkThread::notify() {
	m_mutexCondition.lock();
	m_condition.notify_one();
	m_mutexCondition.unlock();
}

//�˴����ã�������ֻ����һ���߳�
void WorkThread::notify_all() {
	m_mutexCondition.lock();
	m_condition.notify_all();
	m_mutexCondition.unlock();
}
bool WorkThread::isExecuting() {
	//��ʱ���������������̺߳�ʵ���������е�m_task��Ϊnullptr,����Լ�����ö��������������m_task�����Ŷӣ����m_task�Ѿ����Ŷӵ������ʧ��
	bool ret;
	m_mutexTask.lock();		//����m_taskʱ��Ҫ����
	ret = (m_task == nullptr);	//��Ϊ������������
	m_mutexTask.unlock();
	return !ret;
}

//�����߳��б�
LeisureThreadList::LeisureThreadList(const size_t counts) {
	assign(counts);		//������������߳�
}

LeisureThreadList::~LeisureThreadList() {
//ɾ���߳��б��е������̡߳������������ü�������ΪThreadPool��������ǰ���ȵ���exit()��������ֹ�����̵߳����У�����ô���ʵ���������stop()����
	while (!m_threadList.empty()) {		//ɾ��m_threadList�еĹ����߳���ʵ��������
		WorkThread * temp = m_threadList.front();
		m_threadList.pop_front();
		delete temp;
	}
}

//������ʼ���������߳�
void LeisureThreadList::assign(const size_t counts) {
	for (size_t i = 0u; i < counts; i++) {
		m_threadList.push_back(new WorkThread);		//�����߳�
	}
}

//����߳�,���߳��б�������̣߳������
void LeisureThreadList::push(WorkThread *thread) {
	if (thread == nullptr) {
		return;
	}
	m_mutexThread.lock();
	m_threadList.push_back(thread);
	m_mutexThread.unlock();
}

//���ص�һ���߳�ָ��,�漰���߳��б�����������
WorkThread * LeisureThreadList::top() {
	WorkThread * thread;	//��Ҫ����һ��ָ���ȡ���ڽ���֮��return
	m_mutexThread.lock();
	if (m_threadList.empty()) {
		thread = nullptr;
	}
	else {
		thread = m_threadList.front();
	}
	m_mutexThread.unlock();
	return thread;
}

void LeisureThreadList::pop() {
	m_mutexThread.lock();
	if (!m_threadList.empty()) { m_threadList.pop_front(); }
	m_mutexThread.unlock();
}

size_t LeisureThreadList::size() {
	size_t counts = 0u;
	m_mutexThread.lock();
	counts = m_threadList.size();
	m_mutexThread.unlock();
	return counts;
}

//ֹͣ����
void LeisureThreadList::stop() {
	m_mutexThread.lock();
	for (auto thread : m_threadList) {
		thread->stop();				//���ù����߳��������ڲ���ֹ�������߳��˳����������ڣ�Ҳ����LeisureList��������ʱ��Ҫ����WorkThread������������delete����
	}
	m_mutexThread.unlock();
}


//�̳߳���ľ��嶨��,�ڴ����ʼ����ʱ����Ҫͬʱ�Գ�Ա����m_leisureThreadList�����乹�캯�����г�ʼ����
/*���������һ�����Ա����������һ���������һ���ṹ�����������Ա��ֻ��һ���������Ĺ��캯����
��û��Ĭ�Ϲ��캯������ʱҪ��������Ա���г�ʼ�����ͱ������������Ա�Ĵ������Ĺ��캯����
���û�г�ʼ���б���ô�����޷���ɵ�һ�����ͻᱨ��*/
ThreadPool::ThreadPool(const size_t counts):m_leisureThreadList(counts),m_bExit(false) {
	m_threadCounts = counts;
	m_bRunning.store(true);		//������ͣ�����߳�����
	m_bEnd.store(true);			//������ֹ�����߳�����
	m_thread = thread(&ThreadPool::run, this);		//�̳߳ض������������̣߳��������߳�
}

ThreadPool::~ThreadPool() {
	if (!m_bExit) {
		exit();		//����ʱ���ô˺���������ֹ�����̣߳���ں���run()�����У�������leisureList��stop()���������е���WorkThread��stop����������
	}
}

size_t ThreadPool::threadCounts() {
	return m_threadCounts.load();	//	ԭ������
}

bool ThreadPool::isRunning() {
	return m_bRunning.load();		//ԭ������
}

//�����߳���ں���
void ThreadPool::run() {
	while (true) {
		if (!m_bEnd.load()) {		//m_bEnd������ֹ�������̵߳����У���û�в����������Ϊ��������߳��˳�
			m_taskMutex.lock();
			if (m_taskList.empty()) {
				m_taskMutex.unlock();
				break;				//����m_bEndΪfalse���������Ϊ�գ���ֹѭ��
			}
			m_taskMutex.unlock();
		}

		//�������߳���ִͣ�У���������״̬
		{
			unique_lock<mutex> lockRunning(m_runningMutex);
			m_condition_running.wait(lockRunning, [this]() {	//�̳߳��������������̹߳��ã�m_condition_running�������߼��ͨ�ţ������߳���ͣ
				return this->m_bRunning.load(); });				//�̳߳��������ͣ����stop()������m_bRunning��Ϊfalse��������߳������ڴ˴�
		}
		Task * task = nullptr;
		WorkThread * thread = nullptr;
		//���û���������������У�������
		{
			unique_lock<mutex> lock(m_taskMutex);	//��Ҫ����taskList������
			m_condition_task.wait(lock, [this]() {
				return !(this->m_taskList.empty() && this->m_bEnd.load()); });
			//�������ѣ����������������б��ǳ�����ֹ
			if (!m_taskList.empty()) {
				task = m_taskList.front();		//������������������ȡ��
				m_taskList.pop();
			}
		}
		//ѡ����е��߳�ִ������,m_leisureThreadList�ڲ��Ĳ����Ѿ��Ǽ�����֤��ȫ�ģ��˴�����Ҫ����
		do {
			thread = m_leisureThreadList.top();
			m_leisureThreadList.pop();
			m_leisureThreadList.push(thread);
		} while (thread->isExecuting());	//ֱ���ҵ�һ��δ���е��߳�,m_task��Ϊ������������

		//�ҵ���֪ͨ�߳�ִ��
		thread->assign(task);			//��һ��isExecuting()�Ѿ��ж�m_taskΪnullptr��������Ϊ��ֹm_bEnd=false���룬��ô����task=nullptr����Ӱ�칤���߳�
		thread->notify();
	}
}

//���������б���������Ӻ��������Ҫ���������߹����߳�����
void ThreadPool::addTask(Task * task) {
	if (task == nullptr) { return; }
	m_taskMutex.lock();
	m_taskList.push(task);
	m_taskMutex.unlock();
	//֪ͨ���ڵȴ����߳�,Ҳ���ǻ��ѹ������߳�run�����еĵȴ���
	m_condition_task.notify_one();
}

void ThreadPool::start() {
	m_bRunning.store(true);
	m_condition_running.notify_one();	//��������ͣ������run�����е�m_condition_running
}

void ThreadPool::stop() {				//�˴�Ϊ��ִͣ����start()������Ӧ��ֻ���ù������̴߳�������״̬������Ӱ���ѹ����߳�
	m_bRunning.store(false);			//��Ϊfalse������run������ѭ���������m_condition_running������
}

void ThreadPool::exit() {	//�������˴���exit()������ӦleisureList��stop��WorkThread��stop���������̴߳������ѷ�����������ֹ
	m_bEnd.store(false);	//��Ϊfalse����Ҫ���ѿ��ܴ���m_condition_task�����Ĺ������߳�
	m_condition_task.notify_all();
	m_mutexThread.lock();
	if (m_thread.joinable()) {
		m_thread.join();
	}
	m_mutexThread.unlock();
	m_leisureThreadList.stop();		//�����߳�ֹͣ���ڲ�ʵ�ֻ���߳����Ѿ����ڵ�����ִ�������˳�
	m_bExit = true;
}