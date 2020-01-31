//
// Created by liyinbin on 2020/1/31.
//

#ifndef ABEL_META_APPLY_H_
#define ABEL_ABEL_META_APPLY_H_

#include <tuple>
#include <utility>
#include <cstddef>
#include <abel/meta/type_traits.h>

namespace abel {

template<typename Func, typename Args, typename IndexList>
struct apply_helper;

template<typename Func, typename Tuple, size_t... I>
struct apply_helper<Func, Tuple, abel::index_sequence<I...>> {
    static auto apply (Func &&func, Tuple args) {
        return func(std::get<I>(std::forward<Tuple>(args))...);
    }
};

template<typename Func, typename... T>
inline
auto apply (Func &&func, std::tuple<T...> &&args) {
    using helper = apply_helper<Func, std::tuple<T...> &&, abel::index_sequence_for<T...>>;
    return helper::apply(std::forward<Func>(func), std::move(args));
}

template<typename Func, typename... T>
inline
auto apply (Func &&func, std::tuple<T...> &args) {
    using helper = apply_helper<Func, std::tuple<T...> &, abel::index_sequence_for<T...>>;
    return helper::apply(std::forward<Func>(func), args);
}

template<typename Func, typename... T>
inline
auto apply (Func &&func, const std::tuple<T...> &args) {
    using helper = apply_helper<Func, const std::tuple<T...> &, abel::index_sequence_for<T...>>;
    return helper::apply(std::forward<Func>(func), args);
}

} //namespace abel

#endif //ABEL_ABEL_META_APPLY_H_