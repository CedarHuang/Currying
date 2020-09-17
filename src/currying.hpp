#ifndef __CEDAR_CURRYING_HPP__
#define __CEDAR_CURRYING_HPP__

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>

namespace cedar {

template <std::size_t...>
struct index_sequence {};

template <std::size_t N, std::size_t... Ints>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Ints...> {};
template <std::size_t... Ints>
struct make_index_sequence<0, Ints...> : index_sequence<Ints...> {};

template <class Tuple>
using index_sequence_for_tuple = make_index_sequence<std::tuple_size<Tuple>::value>;

template <class _Fn, class... _Args, std::size_t... _Ints>
typename std::result_of<_Fn(_Args...)>::type
_apply_impl(_Fn&& _fn, const std::tuple<_Args...>& _t, index_sequence<_Ints...>) {
    return _fn(std::get<_Ints>(_t)...);
}

template <class Fn, class... Args>
typename std::result_of<Fn(Args...)>::type
apply(Fn&& fn, const std::tuple<Args...>& t) {
    return _apply_impl(std::forward<Fn>(fn), t, index_sequence_for_tuple<std::tuple<Args...>>());
}

template <class, class, class...>
class _currying_impl;
template <class _R, class... _Args, class... _TupleArgs, class _YetArg, class... _YetArgs>
class _currying_impl<_R(_Args...), std::tuple<_TupleArgs...>, _YetArg, _YetArgs...> {
  public:
    template <class _Fn>
    _currying_impl(_Fn&& _fn, const std::tuple<_TupleArgs...>& _t) : _fn_(std::forward<_Fn>(_fn)), _t_(_t) {}
    template <class _Fn>
    _currying_impl(_Fn&& _fn, std::tuple<_TupleArgs...>&& _t) : _fn_(std::forward<_Fn>(_fn)), _t_(std::move(_t)) {}

    _currying_impl<_R(_Args...), std::tuple<_TupleArgs..., _YetArg>, _YetArgs...>
    operator()(_YetArg _arg) const& {
        return _currying_impl<_R(_Args...), std::tuple<_TupleArgs..., _YetArg>, _YetArgs...>(
            _fn_, std::tuple_cat(_t_, std::tuple<_YetArg>(std::forward<_YetArg>(_arg))));
    }
    _currying_impl<_R(_Args...), std::tuple<_TupleArgs..., _YetArg>, _YetArgs...>
    operator()(_YetArg _arg) && {
        return _currying_impl<_R(_Args...), std::tuple<_TupleArgs..., _YetArg>, _YetArgs...>(
            std::move(_fn_), std::tuple_cat(std::move(_t_), std::tuple<_YetArg>(std::forward<_YetArg>(_arg))));
    }

  private:
    template <class _MaybeArg, bool _ArgsSize, typename std::enable_if<_ArgsSize != 0>::type* = nullptr, class... _MaybeArgs>
    auto _mut_args_call(_MaybeArg&& _arg, _MaybeArgs&&... _args)
        -> decltype(operator()(std::forward<_MaybeArg>(_arg))(std::forward<_MaybeArgs>(_args)...)) {
        return operator()(std::forward<_MaybeArg>(_arg))(std::forward<_MaybeArgs>(_args)...);
    }

  public:
    template <class _MaybeArg, class... _MaybeArgs>
    auto operator()(_MaybeArg&& _arg, _MaybeArgs&&... _args)
        -> decltype(_mut_args_call<_MaybeArg&&, sizeof...(_MaybeArgs), nullptr, _MaybeArgs&&...>(std::forward<_MaybeArg>(_arg), std::forward<_MaybeArgs>(_args)...)) {
        return _mut_args_call<_MaybeArg&&, sizeof...(_MaybeArgs), nullptr, _MaybeArgs&&...>(std::forward<_MaybeArg>(_arg), std::forward<_MaybeArgs>(_args)...);
    }

  private:
    std::function<_R(_Args...)> _fn_;
    std::tuple<_TupleArgs...> _t_;
};
template <class _R, class... _Args, class... _TupleArgs>
class _currying_impl<_R(_Args...), std::tuple<_TupleArgs...>> {
  public:
    template <class _Fn>
    _currying_impl(_Fn&& _fn, const std::tuple<_TupleArgs...>& _t) : _fn_(std::forward<_Fn>(_fn)), _t_(_t) {}
    template <class _Fn>
    _currying_impl(_Fn&& _fn, std::tuple<_TupleArgs...>&& _t) : _fn_(std::forward<_Fn>(_fn)), _t_(std::move(_t)) {}

    _R operator()() {
        return *this;
    }

    operator _R() {
        return apply(_fn_, _t_);
    }

  private:
    std::function<_R(_Args...)> _fn_;
    std::tuple<_TupleArgs...> _t_;
};

template <class>
struct currying;
template <class R, class... Args>
struct currying<R(Args...)> : _currying_impl<R(Args...), std::tuple<>, Args...> {
    template <class Fn>
    currying(Fn&& fn) : _currying_impl<R(Args...), std::tuple<>, Args...>(std::forward<Fn>(fn), std::make_tuple()) {}
};

template <class Fn>
struct function_traits : function_traits<decltype(&Fn::operator())> {};
template <class Fn>
struct function_traits<Fn&> : public function_traits<Fn> {};
template <class Fn>
struct function_traits<Fn&&> : public function_traits<Fn> {};
template <class R, class... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using args_type = std::tuple<Args...>;
};
template <class R, class... Args>
struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {};
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(Args...)> {};
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R(Args...)> {};
template <class C, class R>
struct function_traits<R(C::*)> : public function_traits<R()> {};

struct _make_currying_impl {
    template <class _Fn, std::size_t... _Ints>
    currying<typename function_traits<_Fn&&>::return_type(typename std::tuple_element<_Ints, typename function_traits<_Fn&&>::args_type>::type...)>
    operator()(_Fn&& _fn, index_sequence<_Ints...>) {
        return std::forward<_Fn>(_fn);
    }
};

template <class Fn>
typename std::result_of<_make_currying_impl(Fn&&, index_sequence_for_tuple<typename function_traits<Fn&&>::args_type>)>::type
make_currying(Fn&& fn) {
    return _make_currying_impl()(std::forward<Fn>(fn), index_sequence_for_tuple<typename function_traits<Fn&&>::args_type>());
}

}  // namespace cedar

#endif