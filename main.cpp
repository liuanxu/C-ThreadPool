#include"ThreadPool.h"
#include<iostream>
#include<ctime>
#include<fstream>
#include<chrono>

using namespace std;

mutex mtx;
//�Զ����������̳��ڣ�Task������
class myTask :public Task {
	int m_data;
public:
	myTask(int data) {
		m_data = data;
	}
	void run() {
		mtx.lock();
		cout << "�̣߳� " << this_thread::get_id() << "����� " << m_data << endl;
		mtx.unlock();
	}
};

int main() {
	ThreadPool pool(5);
	for (int i = 0; i < 10000; i++) {
		pool.addTask(new myTask(i));
	}
	pool.exit();
	return 0;
}