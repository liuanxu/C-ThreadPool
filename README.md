# C-ThreadPool
C++11实现线程池功能

1.头文件中包含四个类。

2.Task抽象类，用于实现客户要求的任务，可自定义其功能，只需继承该Task类并重写其虚函数run()即可。

3.LeisureThreadList类，用于维护固定数量的工作线程，提供外部操作接口push(),pop(),并用锁保证操作的线程安全。

4.WorkThread类，工作线程类，用于执行管理线程分配下来的任务，无任务时处于阻塞状态。消费者角色。

5.ThreadPool类，线程池主类，其中包含管理线程，用于任务队列的管理、工作线程队列的管理和工作线程的任务分配。生产者不断产生任务，并通过条件变量同步与消费者线程的通信。

