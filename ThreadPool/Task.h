
#ifndef THREADPOL_TASK_H
#define THREADPOL_TASK_H

#endif
struct AggregateException : public std::exception
{
    std::string s;
    AggregateException(std::string ss) : s(ss) {}
    ~AggregateException() throw () {}
    const char* what() const throw() { return s.c_str(); }
};

template<class Result, class ... Args>
class FunctionWrapper{
public:
    FunctionWrapper(std::function<Result(Args ...)>inputFunc) : func(inputFunc){};

    Result execute(Args... args) const{
        return func(args...);
    }
private:
    std::function<Result(Args...)> func;
};
template<typename ... Args>
class FunctionWrapper<void, Args...>{
public:
    FunctionWrapper(std::function<void(Args ...)>inputFunc) : func(inputFunc){};

    void execute(Args... args) const{
        func(args...);
    }
private:
    std::function<void(Args...)> func;
};

class IMyTask{
public:
    virtual void run() = 0;
};
template<class TResult>

class MyTask : public IMyTask{
public:
    MyTask(FunctionWrapper<TResult> inputFuncWrapper) : funcWrapper(inputFuncWrapper){}

    template<class TNewResult>
    std::unique_ptr<MyTask<TNewResult>> ContinueWith(FunctionWrapper<TNewResult, TResult> functionWrapper) {
        auto func = [this, functionWrapper]()
        {
            TResult res = this->getResult();
            return functionWrapper.execute(res);
        };
        return std::make_unique<MyTask<TNewResult>>(FunctionWrapper<TNewResult>(std::move(func)));
    };

    TResult getResult() {
        while (!this->isCompleted.load())
        {
            std::this_thread::yield();
        }
        return result;
    }
    void run() override{
        try {
            result = funcWrapper.execute();
        }
        catch(const std::exception& ex){
            throw AggregateException(ex.what());
        }
        catch(...){
            throw AggregateException("Unknown failure occurred. Possible memory corruption\n" );
        }

        this->isCompleted.store(true);
    }
     std::atomic<bool> isCompleted  = {false};
private:
    TResult result;
    FunctionWrapper<TResult> funcWrapper;
};

template<>
class MyTask<void> : public IMyTask{
public:
    MyTask(FunctionWrapper<void> inputFuncWrapper) : funcWrapper(std::move(inputFuncWrapper)){}

    template<class TNewResult>
    std::unique_ptr<MyTask<TNewResult>> ContinueWith(FunctionWrapper<TNewResult> functionWrapper) {
        auto func = [this, functionWrapper]
        {
            getResult();
            return functionWrapper.execute();
        };
        return std::make_unique<MyTask<TNewResult>>(FunctionWrapper<TNewResult>(std::move(func)));
    };

    void getResult() {
        while (!this->isCompleted.load())
        {
            std::this_thread::yield();
        }
        return;
    }
    void run() override{
        try {
            funcWrapper.execute();
        }
        catch(const std::exception& ex){
            throw AggregateException(ex.what());
        }
        catch(...){
            throw AggregateException("Unknown failure occurred. Possible memory corruption\n" );
        }

        this->isCompleted.store(true);
    }
    std::atomic<bool> isCompleted = {false};

private:
    FunctionWrapper<void> funcWrapper;
};
