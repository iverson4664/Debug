#ifndef __TUPLE_EXAMPLE__
#define __TUPLE_EXAMPLE__

#include "system.h"

namespace test {

template <class T, class Tuple>
struct tupleIndex4Type;

template <class T, class... Types>
struct tupleIndex4Type<T, std::tuple<T, Types...>> {
    static constexpr auto value = 0;
};

template <class T, class U, class... Types>
struct tupleIndex4Type<T, std::tuple<U, Types...>> {
    static constexpr auto value = 1 + tupleIndex4Type<T, std::tuple<Types...>>::value;
};

} // namespace test
#endif // __TUPLE_EXAMPLE__
