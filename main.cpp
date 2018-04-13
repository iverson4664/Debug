#include "system.h"

#include "log.h"
#include "singleton.h"
#include "benchmark.h"
#include "tuple.h"

#include "pq/pq.h"

namespace test {

template <typename T>
class A
{
public:
    template <typename U = T>
        typename std::enable_if<std::is_same<int, U>::value>::type
        hasInt() { LOG("has int"); }
    template <typename U = T>
        typename std::enable_if<std::is_same<char, U>::value>::type
        hasChar() { LOG("has char"); }
    void f()  { }
    A(): a(0) { LOG("constructing: %p", this);  }
    ~A() { LOG("destructing: %p", this); }

private:
    int a;
};

struct B
{
    B() { LOG("B: ctor [%p]", this); }
    ~B() { LOG("B: dtor [%p]", this); }
};

struct C
{
    C(int i) : m_i(i)
    { LOG("C: ctor(%d) [%p]", m_i, this); }
    ~C() { LOG("C: dtor(%d) [%p]", m_i, this); }
    void print() { LOG("C[%p]: m_i(%d)", this, m_i); }

    int m_i;
};

template <typename T>
void swap1(T& lhs, T& rhs)
{
    T tmp = std::move(lhs);
    lhs = std::move(rhs);
    rhs = std::move(tmp);
}

std::vector<int> _random(int n)
{
    std::srand(std::time(0));
    std::vector<int> tmp(n);
    for (auto &i : tmp) {
        i = std::rand();
    }
    return tmp;
}


template <typename T1, typename T2>
auto add(T1&& t1, T2&& t2) -> decltype(t1 + t2)
{
    static_assert(std::is_integral<T1>::value, "T1 must be a int");
    static_assert(std::is_integral<T2>::value, "T2 must be a int");
    return t1 + t2;
}

struct Good: std::enable_shared_from_this<Good>
{
    std::shared_ptr<Good> getptr() {
            return shared_from_this();
        }
};

struct Bad
{
    std::shared_ptr<Bad> getptr() {
            return std::shared_ptr<Bad>(this);
        }
    ~Bad() { std::cout << "Bad::~Bad() called\n";  }
};

class Base
{
public:
    int print() { LOG(" this (%p) A addr(%p)... ", this, &__a); return 1; }
    static int __a;
};
int Base::__a = 1;

class Derive : public Base
{
};

void passSharePtr(Good* p)
{
    std::shared_ptr<Good> sp(p);
    LOG("%s: use count: %d: ", __FUNCTION__, sp.use_count());
    std::weak_ptr<Good> sp1(sp);
    LOG("%s: use count: %d: ", __FUNCTION__, sp1.use_count());
    std::shared_ptr<Good> sp2(sp1);
    LOG("%s: use count: %d: %d: %d", __FUNCTION__, sp.use_count(), sp1.use_count(), sp2.use_count());
}


class Derive1
    : public Singleton<Derive1>
    , public Derive
{
public:
    int print() { LOG(" print... "); return 1; }
    int a;

    Derive1(int a1, int a2, int a3 = 10) { LOG(" Derive1 ctor [%p] : %d, %d, %d,", this, a1, a2, a3); }
};

struct _DeclValTest
{
    _DeclValTest(const _DeclValTest&) {} // issue: no default ctor _DeclValTest() {}
    char print() { LOG(" print... "); return 'c'; }
};

SINGLETON_DEFINITION(Derive1);



template <typename T, typename M> M get_member_type(M T::*);
template <typename T, typename M> T get_class_type(M T::*);

template <typename T,
         typename R,
         R T::*M>
constexpr std::size_t offset_of__()
{
    return reinterpret_cast<std::size_t>(&(((T*)0)->*M));
}

#define OFFSET_OF(m) \
    offset_of__<decltype(get_class_type(m)), decltype(get_member_type(m)), m>()

struct S
{
    int x;
    int y;
};

struct S1 : public S
{
    int x;
    char pad[5];
    int y;
    std::array<int, 3> rgT;
    // std::array<int, 3> rgT{};
};

static_assert(OFFSET_OF(&S::x) == 0, "");

template <typename T, typename U>
constexpr std::size_t __offsetof(U T::*member)
{
    return (uint8_t*)&((T*)nullptr->*member) - (uint8_t*)nullptr;
}

template <typename T, typename U>
struct __offsetOfClass
{
    constexpr static size_t value = (char*)(static_cast<T*>(reinterpret_cast<U*>(0x1))) - (char*)0x1;
};

#define _ATL_PACKING 8
template<class Base, class Through, class Derived>
uint32_t* offsetOfClass()
{
    Derived* derivedObject = reinterpret_cast<Derived*>(_ATL_PACKING);
    Through* through = derivedObject;
    Base* base = through;

    uint32_t* offset = reinterpret_cast<uint32_t*>(base)-_ATL_PACKING;
    return offset;
}

struct CMeta {
    CMeta(uint32_t v) : value(v) {}
    int32_t value;
    int32_t get() { return value; }
};

// test overload or override(overwrite)
class OOClass
{
public:
    void f() {}
};

class OOClassDerive : public OOClass
{
public:
    using OOClass::f; // tricky: keep Base class f() unhidden
    void f(int i) {}
};

extern "C"
void testString(std::string str)
{
    LOG("%s: %s", __func__, str.c_str());
}

static unsigned int get_size_align_mask(unsigned int align)
{
    unsigned int mask = 0xFFFFFFFF;
    unsigned int mask_num = 0;

    for(mask_num = 0; mask_num < 32; mask_num++) {
        if ((0x1 & (align >> mask_num )) == 1) {
            return mask;
        }
        mask &= ~(1 << mask_num);
    }
    return 0;
}

constexpr auto kMill= 1000000;
constexpr auto tenMill = 10 * kMill;

void readDirectory(const char* path) {
    LOG("path = %s", path);
    DIR *d = opendir(path);
    if (d == nullptr) {
        LOG("open dir %s failed .", path);
        return;
    }

    struct dirent *dir;
    char b[255];
    while ((dir = readdir(d)) != nullptr) {
        // LOG("d type = %d, name = %s", dir->d_type, dir->d_name);
        if (dir->d_type == DT_REG) {
            char* dot = strrchr(dir->d_name, '.');
            if (dot && strcmp(dot, ".swp")) {
                LOG("%s", dir->d_name);
            }
        } else if (dir->d_type == DT_DIR &&
                   strcmp(dir->d_name, ".") &&
                   strcmp(dir->d_name, "..")) {
            memset(b, 0x00, sizeof(b));
            sprintf(b, "%s/%s", path, dir->d_name);
            readDirectory(b);
        } else {
            // LOG("Ignore reading type %d.", dir->d_type);
        }
    }
    closedir(d);
}

void testSTL()
{
    LOG("%s:enter", __FUNCTION__);

    // test conversion
    // unsigned int a__ = 0x11000000;
    // printf("test conversion 0x%x\n", *(unsigned short *)&a__); // expect 0x0

    // test dir reading
    // readDirectory(".");

    testString("test string...");

    // test iteratoro
    std::list<int> listInt;
    listInt.push_front(1);
    listInt.push_front(2);
    listInt.push_front(3);
    auto it = std::find(listInt.begin(),
                        listInt.end(),
                        2);
    if (it != listInt.end()) {
        LOG("Test iterator: list it %d", *it);
    }
    for (auto& it : listInt) {
        int a;
        a = it;
        LOG("Test rang loop iterator: it %d", a);
    }


    // test tuple function
    using _FunA = std::function<void(void)>;
    using _FunB = std::function<void(int)>;
    using _FunC = std::function<int(void)>;
    auto funcTuple = std::make_tuple([] () {
    LOG("FuncA ........."); }, [] (int a) {
        LOG("FuncB .........");  }, [] () -> int {
        LOG("FuncC .........");
        return 0xf; });
    // std::tuple<_FunA, _FunB, _FunC> funcTuple([] () {
    // LOG("FuncA ........."); }, [] (int a) {
    //     LOG("FuncB .........");  }, [] () -> int {
    //     LOG("FuncC .........");
    //     return 0xf; });
    std::get<0>(funcTuple)();
    int __aval;
    std::get<1>(funcTuple)(__aval);

    auto tupleret = std::get<2>(funcTuple)();
    LOG("tuple func3 ret: 0x%x.........", tupleret);

    _FunA funcA_;
    _FunB funcB_;
    _FunC funcC_;
    std::tie(funcA_, funcB_, funcC_) = funcTuple;
    funcA_();
    funcB_(__aval);
    funcC_();


    // test thread

    bool t2Flag = true;
    std::thread t2(std::thread([&t2Flag] {
        LOG("thread id 0x%x", std::this_thread::get_id());
        while (t2Flag) {
            LOG("thread id 0x%x loop", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
    }));

    LOG("thread id 0x%x step1", t2.get_id());
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));

    LOG("thread id 0x%x step2", t2.get_id());
    std::this_thread::sleep_for(std::chrono::microseconds(1000));

    t2Flag = false;
    if (t2.joinable()) {
        LOG("thread id 0x%x joinable", t2.get_id());
        t2.join();
    }

    // std::thread([] {
    //     LOG("first thread id 0x%x", std::this_thread::get_id());
    //     std::this_thread::sleep_for(std::chrono::seconds(2));
    // }).detach();

    // std::thread([] {
    //     LOG("second thread id 0x%x", std::this_thread::get_id());
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }).detach();

    // std::thread t1;
    // std::mutex LOCK;
    // std::condition_variable cv;
    // t1 = std::thread([&] {
    //     LOG("1 thread id 0x%x", std::this_thread::get_id());
    //     std::unique_lock<std::mutex> lock(LOCK);
    //     cv.wait_for(lock, std::chrono::seconds(4));
    //     LOG("1 thread id 0x%x exit", std::this_thread::get_id());
    //     // std::this_thread::sleep_for(std::chrono::seconds(2));
    // });
    // LOG("t1 step1 thread: %d", t1.joinable());
    // // t1.detach();
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    // LOG("t1 step11 thread: %d", t1.joinable());
    // if (t1.joinable()) {
    //     LOG("step11 thread id 0x%x joinable", std::this_thread::get_id());
    //     t1.join();
    // }

    // t1 = std::thread([&cv] {
    //     LOG("2 thread id 0x%x", std::this_thread::get_id());
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     LOG("2 thread id 0x%x notify", std::this_thread::get_id());
    //     cv.notify_one();
    // });
    // LOG("t2 step12 thread: %d", t1.joinable());
    // // t1.detach();
    // if (t1.joinable()) {
    //     t1.join();
    // }
    // LOG("t2 step12 thread: %d", t1.joinable());

    // test remove_if, find_if
    std::list<uint32_t> listU;
    listU.push_back(1);
    listU.push_back(2);
    listU.push_back(2);
    listU.push_back(3);

    listU.erase(std::remove(listU.begin(), listU.end(), 2), listU.end());


    auto flIt = std::find_if(listU.begin(), listU.end(), [] (auto& map) {
        return map == 1;
    });
    LOG("listU test: find found value(%d)", *flIt);

     flIt = std::remove_if(listU.begin(), listU.end(), [] (auto& map) {
        return map == 1;
    });
    LOG("listU test: found value(%d)", *flIt);

    listU.erase(flIt, listU.end());

    for (auto it = listU.begin(); it != listU.end(); ++it) {
        LOG("listU test: value(%d)", *it);
    }


    // test make_pair
    std::map<int32_t, std::shared_ptr<B>> mapB;
    auto spB = std::make_shared<B>();
    auto pairB = std::make_pair(1, spB); //error if not right ref
    // auto pairB = std::make_pair<int32_t, std::shared_ptr<B>>(1, spB); //error if not right ref
    mapB.emplace(1, spB);

    //test deque
    std::vector<long> vecInteger;
    std::deque<long> dqInteger;
    std::list<long> listInteger;
    LOG("vector size: %d, deque size: %d, list size:%d", sizeof(vecInteger), sizeof(dqInteger), sizeof(listInteger));

    // test overload or override
    OOClass ooc;
    ooc.f();
    OOClassDerive oocd;
    oocd.f(1);
    oocd.f(); // if no using 'OOClass::f' will compile error


    // using std::function; ???

    // map will sort by less f()
    // using MetaMap = std::map<uint32_t, CMeta>;
    // using MetaMap = std::unordered_map<uint32_t, CMeta>;
    using MetaMap = std::unordered_map<uint32_t, uint32_t>;

    // unordered_map is hash map, not sorted
    // using MetaMap = std::unordered_map<uint32_t, CMeta>;
    MetaMap mapMeta;
    // mapMeta.emplace(0x1010, CMeta(10));
    // mapMeta.emplace(0x1000, CMeta(10));
    // mapMeta.emplace(0x1001, CMeta(11));
    mapMeta.insert(mapMeta.begin(), MetaMap::value_type(0x1010, 10));
    mapMeta.insert(mapMeta.begin(), MetaMap::value_type(0x1000, 10));
    mapMeta.insert(mapMeta.begin(), MetaMap::value_type(0x1001, 11));
    // for (auto& it : mapMeta) {
    //     LOG("map test: key(0x%x), value(%d)", it.first, it.second.get());
    // }
    for (auto it = mapMeta.begin(); it != mapMeta.end(); ++it) {
        LOG("map test: key(0x%x), value(%d)", it->first, it->second);
    }
    // mapMeta.erase(std::find_if(mapMeta.begin(), mapMeta.end(), [] (auto& map) {
    //     return map.first == 0x1000;
    // }));


    // C++17
    // for (auto& [k, v] : mapMeta) {
    //     LOG("map test: key(0x%x), value(%d)", k, v);
    // }
    // uint32_t m_k;
    // CMeta m_v(0);
    // for (auto&& std::tie(m_k, m_v) : mapMeta) {
    //     LOG("map test: key(0x%x), value(%d)", m_k, m_v.get());
    // }


    // std::set<int> _set{1,3,2, -1};
    std::multiset<int> _set{1,3,2, -1};
    // _set.erase(_set.begin());
    _set.insert(0);
    _set.emplace(-2);
    _set.emplace(-2);
    _set.emplace(-3);
    auto count = _set.count(-2);
    LOG("__set find : %d", count);
    auto equal_rang = _set.equal_range(-2);
    for (auto begin = equal_rang.first; begin != equal_rang.second; ++begin) {
        LOG("__set rang: %d", *begin);
    }

    auto upperbound = _set.upper_bound(-1);
    LOG("__set upperbound: %d", *upperbound);

    auto lowerbound = _set.lower_bound(-1);
    LOG("__set lowerbound: %d", *lowerbound);

    for(auto i : _set) {
        LOG("__set: %d", i);
    }

    S1 __s1;
    __s1.rgT.fill(-1);
    LOG("S1 array init value: %d, %d, %d, size(%d, %d)",
        __s1.rgT[0], __s1.rgT[1], __s1.rgT[2],
        __s1.rgT.size(), __s1.rgT.max_size());

    {
        // int* arr = new int[tenMill];
        // {
        //     BenchMark<ms> bm("array vector [] test:");
        //     for (size_t i = 0; i < tenMill; i++) {
        //         arr[i] = 1;
        //     }
        // }
        // delete [] arr;

        std::array<int, 10000> arr;
        // std::array<int, 10> arr = new std::array<int, 10>;
        arr[1] = 0;
        // for (auto& i : arr) {
        //     i = 1;
        // }
        // delete arr [];

        // std::vector<int> arr;
        // for (size_t i = 0; i < tenMill; i++) {
        //     arr.push_back(1);
        // }
    }


    S1* _s1 = new S1;
    LOG("%x \n", _s1);
    S* _s = _s1;
    LOG("%x", _s);
    LOG("S1-S addr (%p, %p)", reinterpret_cast<uint8_t*>(reinterpret_cast<S1*>(0x1)),
                                                      (void*)(reinterpret_cast<uint8_t*>(static_cast<S*>(reinterpret_cast<S1*>(0x1)))));

    LOG("S offset S1: %d", offsetOfClass<S, S, S>());
    LOG("S offset S1: %d", __offsetOfClass<S, S1>::value);
    LOG("S offset: %d, %d", OFFSET_OF(&S::y), __offsetof(&S1::y));


    // LOG("sizeof declval type: %d", sizeof(_DeclValTest().print()));//compile error: no default ctor
    LOG("sizeof declval type: %d", sizeof(decltype(std::declval<_DeclValTest>().print())));
    Base __base;
    __base.print();
    Base* _pBase = new Base;
     _pBase->print();
     delete _pBase;
     _pBase = nullptr;


    std::pair<double, int> ppr = {1.0, 2};
    LOG("pair: %f, %d, %d, %f", ppr.first, ppr.second, std::get<1>(ppr), std::get<tupleIndex4Type<double, std::tuple<double, int>>::value>(ppr));
    std::tuple<int, int, char, const char*, double> ttp{1, 1, 'c', "string", 1.0};
    // std::tuple<int, int, char, const char*, double> ttp = {1, 1, 'c', "string", 1.0}; // compile error
    // auto ttp = std::tuple<int, int, char, const char*, double>{1, 1, 'c', "string", 1.0};
    // auto ttp = std::make_tuple(1, 1, 'c', "string", 1.0);
    LOG("tuple: %d %d %c", std::get<0>(ttp), std::get<1>(ttp), std::get<2>(ttp));
    LOG("tuple: %s", std::get<tupleIndex4Type<const char*, decltype(ttp)>::value>(ttp));

    std::vector<int> vecA(5, 2);
    // std::vector<int> vecA{5, 2, 1};
    for (auto& i : vecA) {
        LOG("vecA = %d", i);
    }

    LOG("test singleton start ----------");
    {
        // Base* p = &Derive1::instance(1, 2);
        BenchMark<ms> bm("once_singleton impl");
        for (size_t i = 0; i < tenMill; i++) {
            Derive1::instance(1, 2, 3);
        }
    }
    LOG("test singleton end ----------");

    // Derive1::instance().print();
    // Derive1::instance().print();


    Base* pBase = new Derive();
    LOG("convertible: %d", std::is_convertible<Derive*, Base*>::value);
    delete pBase;
    pBase = nullptr;

    auto array = {-1, 1, 3};
    for (int i : array) {
        LOG("initializer list: %d", i);
    }

    {
        std::shared_ptr<Good> gp1 = std::make_shared<Good>();
        std::cout << "gp1.befor use_count() = " << gp1.use_count() << '\n';
        // passSharePtr(gp1.get());
        // std::cout << "gp1.after use_count() = " << gp1.use_count() << '\n';
        // std::shared_ptr<Good> gp2 = gp1->getptr();
        // std::cout << "gp2.use_count() = " << gp2.use_count() << '\n';


        try {
            Good not_so_good;
            // error: need follow this way: 1-ctor of enable_shared_from_this, 2-ctor of T; 3-ctor of shared_pter;
            std::shared_ptr<Good> gp1 = not_so_good.getptr();
        } catch(std::bad_weak_ptr& e) {
            // undefined behavior (until C++17) and std::bad_weak_ptr thrown
            // (since C++17)
            std::cout << e.what() << '\n';
        }
        std::shared_ptr<Bad> bp1 = std::make_shared<Bad>();
        // std::shared_ptr<Bad> bp2 = bp1->getptr();
        // std::cout << "bp2.use_count() = " << bp2.use_count() << '\n';

    }

    bool fSame = std::is_same<int, uint32_t>::value;
    LOG("is same : %d", fSame);

    LOG("is_polymorphic: %d", std::is_polymorphic<A<int>>::value);

    A<int> a;
    // a.hasChar();
    a.hasInt();

    A<int> _a;
    swap1(a, _a);

    for (auto i : _random(2)) {
        LOG("i = %d", i);
    }

    LOG("add int: %d", add(1, 2));
    // std::array<int, 3> a1{ {1, 2, 3}  };

    // test map erase
    std::map<int, int> mapInt;
    mapInt.emplace(0, 0xf);
    mapInt.emplace(1, 0xf);
    mapInt.emplace(2, 0xf);
    mapInt.emplace(3, 0xf);
    mapInt.emplace(4, 0xf);
    mapInt.emplace(5, 0xf);

    for (auto& it : mapInt) {
        LOG("mapint key-value: %d-%d", it.first, it.second);
        if (it.first == 2) {
            LOG("mapint erase key-value: %d-%d", it.first, it.second);
            mapInt.erase(it.first);
        }
    }

    // test alignment 1M
    LOG("ALIGNMENT 1M (0x%x)", get_size_align_mask(0x100000));

    // test std::atomic
    std::atomic<bool> amFlag(false);
    if (amFlag.load()) {
        LOG("atomic true");
    } else {
        LOG("atomic false");
    }





    LOG("exit");
}

void testSo()
{
    std::string ld_path(getenv("LD_LIBRARY_PATH"));
    LOG("getenv:[%s]", ld_path.c_str());
    ld_path.append("$(pwd):");
    setenv("LD_LIBRARY_PATH", ld_path.c_str(), 1);

    LOG("getenv: new env path[%s]", getenv("LD_LIBRARY_PATH"));

    void* pHdl = ::dlopen("out/libmain.so", RTLD_LAZY | RTLD_LOCAL);
    if (pHdl != nullptr) {
        using func_ptr = void(*)();
        func_ptr pFunc = reinterpret_cast<func_ptr>(::dlsym(pHdl, "testSoSymbol"));
        if (pFunc != nullptr) {
            pFunc(); // (*pFunc)();
        } else {
            LOG("so dlsym failed: %s\n", dlerror());
        }
        ::dlclose(pHdl);
    } else {
        LOG("so dlopen failed: %s\n", dlerror());
    }
}

} // namespace test

extern "C"
void testSoSymbol()
{
    LOG("loading so successful ....");
}

int main()
{
    LOG("....................................................");
    LOG("__cplusplus version: %ld", __cplusplus); // 201103L

    test::testSo();
    test::testSTL();
    // test::testPq();

    LOG("....................................................");
    return 0;
}
