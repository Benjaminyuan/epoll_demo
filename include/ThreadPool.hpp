#ifndef _THREAD_POOL_
#define _THREAD_POOL_
#include <functional>
#include <atomic>
#include<vector>
#include<pthread.h>
#include<queue>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<thread>
class ThreadPool{
    public:
        using Task = std::function<void()>;
        ThreadPool(const ThreadPool&) = delete;
       	ThreadPool& operator=(const ThreadPool& other) = delete;
        // ThreadPool& operater=(const ThreadPool& pool) = delete;
        explicit ThreadPool(int num): thread_num(num),is_running(false){

        }
        ~ThreadPool(){
            if (is_running)
            {
                stop();
            }
        }
        void start(){
            is_running = true;
            for(int i=0;i<thread_num;i++){
                threads.emplace_back(std::thread(&ThreadPool::work,this));
            }
        }
        void stop(){
            {
                std::unique_lock<std::mutex> lk(mtx);
                is_running = false;
                cond.notify_all();
            }
            for(std::thread& t : threads){
                if(t.joinable()){
                    t.join();
                }
            }
        }
        void appendTask(const Task& task){
            if(is_running){
                std::unique_lock<std::mutex> lk(mtx);
                std::cout<<"task queue size: " <<tasks.size()<<std::endl;
                tasks.push(task);
                std::cout<<"task queue size: " <<tasks.size()<<std::endl;
                cond.notify_one();
            }
        }
    private:
        int thread_num;
        std::vector<std::thread> threads;
        std::queue<Task> tasks;
        std::atomic_bool is_running;
        std:: mutex mtx;
        std::condition_variable cond;

        void work(){
            std::cout << "work thread: "<<std::this_thread::get_id()<<" start" << std::endl;
            while (is_running){
                Task task;
                {
                    std::unique_lock<std::mutex> lk(mtx);
                    if(!tasks.empty()){
                        task = tasks.front();
                        std::cout<<"task queue size: " <<tasks.size()<<std::endl;
                        tasks.pop();
                        std::cout<<"task queue size: " <<tasks.size()<<std::endl;
                    }else if (is_running && tasks.empty()){
                        std::cout<<"task is empty,thread: "<< std::this_thread::get_id() << " sleep"<< std::endl;
                        cond.wait(lk);
                        std::cout<<"task is coming,thread: "<< std::this_thread::get_id() << " wake up"<< std::endl;
                    }
                }
                if(task){
                    task();
                }
            }
            std::cout<<"work thread: " << std::this_thread::get_id() << " end "<< std::endl;
        }

};
#endif 