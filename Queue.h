#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>

template<class T>
class Queue
{
public:
    explicit Queue(int size= -1): size_(size){

    }

    void Put(const T &v){
        std::lock_guard<std::mutex> lock(mtx_);
        if(size_ == -1){
            data_queue_.push(v);
        }else{
            if(data_queue_.size() >= size_){
                data_queue_.pop();
            }
            data_queue_.push(v);
        }
        cv_.notify_one();
    }

    void Quit(){
        quit_ = true;
    }

    const T Pop(bool sync = true){
        if(sync){
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock,[this]{return !data_queue_.empty() || quit_;});
            T d;
            if(quit_)return d;
            d = std::move(data_queue_.front());
            data_queue_.pop();
            return d;
        }else{
            std::lock_guard<std::mutex> lock(mtx_);
            T d;
            if(!data_queue_.empty()){
                d = std::move(data_queue_.front());
                data_queue_.pop();
            }

            return d;
        }
    }

private:
    std::queue<T> data_queue_;
    std::condition_variable cv_;
    std::mutex mtx_;
    int size_ = 0;
    bool quit_ = false;
};
