std::mutex g_display_mutex;

template<typename T>
class ThreadSafeQueue {
    std::queue<T> Queue;
    mutable std::mutex Mutex;
public:
    ThreadSafeQueue() = default;
    virtual ~ThreadSafeQueue() { }

    bool empty() const {
        std::lock_guard<std::mutex> lock(Mutex);
        return Queue.empty();
    }
    unsigned long size() const {
        std::lock_guard<std::mutex> lock(Mutex);
        return Queue.size();
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(Mutex);
        if(Queue.empty()){
            return {};
        }
        T val = Queue.front();
        Queue.pop();
        return std::optional<T>{val};
    }
    void push( T item) {
        std::lock_guard<std::mutex> lock(Mutex);
        Queue.emplace(item);
    }
};
template<typename T>
class TestStruct{
    T cons_Sum;
    T prod_Sum;
    mutable std::mutex g_cons_Sum_mutex{};
    mutable std::mutex g_prod_Sum_mutex{};

public:
    TestStruct(T cons , T prod): cons_Sum(cons), prod_Sum(prod){}

    void addConsumerVal(T consVal){
        std::lock_guard<std::mutex>(this->g_cons_Sum_mutex);
        this->cons_Sum +=consVal;
    }
    void addProducerVal(T prodVal){
        std::lock_guard<std::mutex>(this->g_prod_Sum_mutex);
        this->prod_Sum +=prodVal;
    }
    bool test(){
        std::lock_guard<std::mutex>(this->g_cons_Sum_mutex);
        std::lock_guard<std::mutex>(this->g_prod_Sum_mutex);
        if(this->cons_Sum == this->prod_Sum)
            return true;
        else
            return false;
    }
};
template<typename T>
class Worker{
protected:
    std::reference_wrapper<ThreadSafeQueue<T>> qRef;
    std::reference_wrapper<TestStruct<T>> testRef;
    std::uniform_int_distribution<T> dist;
    std::mt19937 gen;
    std::thread myThread;
    volatile bool continue_ = true;
public:
    Worker(std::reference_wrapper<ThreadSafeQueue<T>> q,
           std::reference_wrapper<TestStruct<T>> test): qRef(q), testRef(test), gen(std::random_device{}()), dist(1, 2){}
    void stop(){
        continue_ = false;
        this->myThread.join();
    }
    virtual void run() = 0;
    virtual void start() = 0;

};
template<typename T>
class Consumer: public Worker<T>{
public:
    Consumer( Consumer<T>&&) = default;
    Consumer(std::reference_wrapper<ThreadSafeQueue<T>> q,
             std::reference_wrapper<TestStruct<T>> test) : Worker<T>(q, test){}
    void run() override {
        std::thread::id this_id = std::this_thread::get_id();
        while(continue_){
            std::optional<T> val = this->qRef.get().pop();
            g_display_mutex.lock();
            if(val.has_value()) {
                std::cout << "Consumer thread " << this_id << " acquired value " << val.value() << std::endl;
                this->testRef.get().addConsumerVal(val.value());
            }
            else{
                std::cout << "Consumer thread " << this_id << " acquired EMPTY value" << std::endl;;
            }
            g_display_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(this->dist(this->gen)));
        }
    }

    void start() override {
        this->myThread = std::thread(&Consumer::run, this);
    }
};

template<typename T>
class Producer: public Worker<T>{
    std::uniform_int_distribution<T> distVal;
public :
    Producer(Producer<T>&&) = default;
    Producer(std::reference_wrapper<ThreadSafeQueue<T>> q,
             std::reference_wrapper<TestStruct<T>> test) : Worker<T>(q, test), distVal(1, 10){}
    void run() override {
        std::thread::id this_id = std::this_thread::get_id();
        while(continue_) {
            T val = distVal(this->gen);
            this->qRef.get().push(val);
            this->testRef.get().addProducerVal(val);
            g_display_mutex.lock();
            std::cout << "Producer thread " << this_id << " put value " << val << std::endl;
            g_display_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(this->dist(this->gen)));
        }
    }
    void start() override {
        this->myThread = std::thread(&Producer::run, this);
    }

};

