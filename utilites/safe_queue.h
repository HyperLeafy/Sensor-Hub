#include <iostream>
#include <atomic>
#include <mutex>
#include <queue>
#include <optional>

template <typename T>
class safeQueue {
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;

public:
    void push_in_queue(const T& item){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(item);
    }

    std::optional<T> pop_from_queue(){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty()){
            return std::nullopt;
        }
        T op = m_queue.front();
        m_queue.pop();
        return op;
    }

    void printQueue() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::queue<T> temp = m_queue;

        if (temp.empty()) {
            std::cout << "Queue is empty\n";
            return;
        }

        std::cout << "Queue elements: ";
        while (!temp.empty()) {
            auto d = temp.front();
            std::cout << d.value << " ";
            temp.pop();
        }
        std::cout << std::endl;
    }

    bool try_pop(T& result) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return false;
        result = m_queue.front();
        m_queue.pop();
        return true;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

};
