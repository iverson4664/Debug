#ifndef __TEST_SINGLETON__
#define __TEST_SINGLETON__

#include "system.h"

namespace test {

template <typename T>
class Singleton {
public:
    // template <typename... Args>
    // static T* instance(Args... args)
    // {
    //     static T t(std::forward<Args>(args)...);
    //     static T* t = new T(std::forward<Args>(args)...);
    //     return t;
    // }

    template <typename... Args>
    static T& instance(Args... args)
    {
#if 0
        //slowest
        static std::mutex m_mutex;
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_upInstance == nullptr) {
            m_upInstance.reset(new T());
        }
        return *m_upInstance;
#elif 0
        // DCL: double-check locking
        if (!m_upInstance) {
            static std::mutex m_Mutex;
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!m_upInstance) {
                m_upInstance.reset(new T());
            }
        }
        return *m_upInstance;

#elif 0
        //fast
        // static std::shared_ptr<T> sp = std::make_shared<T>();
        static std::shared_ptr<T> sp(new T());
        return *sp.get();
#elif 1
        // fast
        // static T instance(std::forward<Args>(args)...);
        // return instance;

        static T* t = new T(std::forward<Args>(args)...);
        return *t;
#elif 1
        T* sp(m_pT.load());
        if (!sp) {
            static std::mutex m_Mutex;
            std::lock_guard<std::mutex> lock(m_Mutex);
            sp = m_pT.load();
            if (!sp) {
                sp = new T(std::forward<Args>(args)...);
                m_pT.store(sp);
            }
        }
        return *sp;
#elif 0
        T* sp(m_pT.load(std::memory_order_acquire));
        if (!sp) {
            static std::mutex m_Mutex;
            std::lock_guard<std::mutex> lock(m_Mutex);
            sp = m_pT.load(std::memory_order_relaxed);
            if (!sp) {
                sp = new T();
                m_pT.store(sp, std::memory_order_release);
            }
        }
        return *sp;

#else
        // little slow
        static std::once_flag fOnce;
        static std::unique_ptr<T> t;
        std::call_once(fOnce, [=]() {
            t.reset(new T(std::forward<Args>(args)...));
        });
        return *t.get();
#endif
    }

protected:
    ~Singleton() { }



private:
    // Singleton(const Singleton&) = delete;
    // Singleton(const Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
    static std::unique_ptr<T> m_upInstance;

    static std::atomic<T*> m_pT;

};

#define SINGLETON_DEFINITION(T) \
template<typename T> \
std::unique_ptr<T> Singleton<T>::m_upInstance;

template<typename T> \
std::atomic<T*> Singleton<T>::m_pT;

} // namespace test
#endif // __TEST_SINGLETON__
