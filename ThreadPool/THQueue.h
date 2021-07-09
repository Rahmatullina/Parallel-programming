
template<typename T>
class ThreadSafeDequeue {
    std::deque<T> Queue;
    mutable std::mutex Mutex;
public:
    ThreadSafeDequeue() = default;
    virtual ~ThreadSafeDequeue() { }

    bool empty() const {
        std::lock_guard<std::mutex> lock(Mutex);
        return Queue.empty();
    }
    unsigned long size() const {
        std::lock_guard<std::mutex> lock(Mutex);
        return Queue.size();
    }

    bool pop_front(T& result) {
        std::lock_guard<std::mutex> lock(Mutex);
        if(Queue.empty()){
            return false;
        }
        result = std::move(Queue.front());
        Queue.pop_front();
        return true;
    }
    void push_back( T item) {
        std::lock_guard<std::mutex> lock(Mutex);
        Queue.emplace_back(std::move(item));
    }
};