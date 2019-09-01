#ifndef __CEDAR_CURRYING_HPP__
#define __CEDAR_CURRYING_HPP__

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>

namespace cedar {

template <std::size_t... Ints>
struct index_sequence {};

template <std::size_t N, std::size_t... Ints>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Ints...> {};
template <std::size_t... Ints>
struct make_index_sequence<0, Ints...> : index_sequence<Ints...> {};

template <class Tuple>
using index_sequence_for_tuple = make_index_sequence<std::tuple_size<Tuple>::value>;

template <class Fn, class... Args, std::size_t... Ints>
typename std::result_of<Fn(Args...)>::type
apply_impl(Fn fn, std::tuple<Args...> t, index_sequence<Ints...>) {
    return fn(std::get<Ints>(t)...);
}

template <class Fn, class... Args>
typename std::result_of<Fn(Args...)>::type
apply(Fn fn, std::tuple<Args...> t) {
    return apply_impl(fn, t, index_sequence_for_tuple<std::tuple<Args...>>());
}

template <class, class, class...>
class currying_impl;
template <class R, class... Args, class... TupleArgs, class YetArg, class... YetArgs>
class currying_impl<R(Args...), std::tuple<TupleArgs...>, YetArg, YetArgs...> {
  public:
    template <class Fn>
    currying_impl(Fn fn, std::tuple<TupleArgs...> t) : fn_(fn), t_(t) {}

    currying_impl<R(Args...), std::tuple<TupleArgs..., YetArg>, YetArgs...>
    operator()(YetArg arg) {
        return currying_impl<R(Args...), std::tuple<TupleArgs..., YetArg>, YetArgs...>(
            fn_, std::tuple_cat(t_, std::make_tuple(arg)));
    }

  private:
    std::function<R(Args...)> fn_;
    std::tuple<TupleArgs...> t_;
};
template <class R, class... Args, class... TupleArgs>
class currying_impl<R(Args...), std::tuple<TupleArgs...>> {
  public:
    template <class Fn>
    currying_impl(Fn fn, std::tuple<TupleArgs...> t) : fn_(fn), t_(t) {}

    R operator()() {
        return *this;
    }

    operator R() {
        return apply(fn_, t_);
    }

  private:
    std::function<R(Args...)> fn_;
    std::tuple<TupleArgs...> t_;
};

template <class>
struct currying;
template <class R, class... Args>
struct currying<R(Args...)> : currying_impl<R(Args...), std::tuple<>, Args...> {
    template <class Fn>
    currying(Fn fn) : currying_impl<R(Args...), std::tuple<>, Args...>(fn, std::make_tuple()) {}
};

}  // namespace cedar
#endif