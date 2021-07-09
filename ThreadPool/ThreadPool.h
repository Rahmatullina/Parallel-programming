class ThreadPool {
public:
    explicit ThreadPool(const int num_threads ) : num_threads(num_threads),
                                                  threads(num_threads),
                                                  thread_task_queues(num_threads){
        for(int i=0; i< num_threads; i++){
            threads[i] = std::thread(&ThreadPool::do_work, this, i);
        }
    };
    ~ThreadPool(){
        Dispose();
    }
    template<class TResult>
    bool Enqueue(std::unique_ptr<MyTask<TResult>> task){
        if(should_work) {
            int i = getThreadIndex();
            thread_task_queues[i].push_back(std::move(task));
            num_of_not_completed_tasks++;
            return true;
        }
        return false;
    };
    bool all_threads_work(){
        for(int i=0; i< num_threads; i++)
            if(!threads[i].joinable())
                return false;
        return true;
    }
    void Dispose(){
        should_work = false;
        for(int i=0; i < num_threads; i++)
            if(threads[i].joinable())
                threads[i].join();
    }
private:
    bool tryStealWork(std::unique_ptr<IMyTask>& task) {
        for (int i = 0; i < num_threads; i++) {
            bool result = thread_task_queues[i].pop_front(task);
            if (result) {
                return true;
            }
        }
        return false;
     }
    int getThreadIndex() const {
        return rand() % num_threads;
    }
    void do_work(int i){

        while(this->should_work || num_of_not_completed_tasks > 0)
        {
            std::unique_ptr<IMyTask> task;
            if (thread_task_queues[i].pop_front(task))
            {
                try
                {
                    task->run();
                }
                catch(AggregateException ex)
                {
                    std::lock_guard<std::mutex> lock(g_display_mutex);
                    std::cout << "Caught exception:" << ex.what() << std::endl;
                }
                num_of_not_completed_tasks--;
            }
            else
            {
                if (tryStealWork(task))
                {
                    try
                    {
                        task->run();
                    }
                    catch(AggregateException ex)
                    {
                        std::lock_guard<std::mutex> lock(g_display_mutex);
                        std::cout << "Caught exception:" << ex.what() << std::endl;
                    }
                    num_of_not_completed_tasks--;
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        }
    }

    int num_threads;
    std::vector<std::thread> threads;
    std::vector<ThreadSafeDequeue<std::unique_ptr<IMyTask>>> thread_task_queues;
    volatile bool should_work = true;
    std::atomic<int> num_of_not_completed_tasks = 0;
};