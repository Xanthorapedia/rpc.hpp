///@file rpc.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Header-only library for serialized RPC usage
///@version 0.8.1
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#pragma once

#if defined(RPC_HPP_DOXYGEN_GEN)
///@brief Enables server-side caching abilities
#  define RPC_HPP_ENABLE_SERVER_CACHE
///@brief Indicates that rpc.hpp is being consumed by a client translation unit
#  define RPC_HPP_CLIENT_IMPL
///@brief Indicates that rpc.hpp is being consumed by a module (dynamically loaded .dll/.so) translation unit
#  define RPC_HPP_MODULE_IMPL
///@brief Indicates that rpc.hpp is being consumed by a server translation unit
#  define RPC_HPP_SERVER_IMPL
#endif

#if !defined(RPC_HPP_CLIENT_IMPL) && !defined(RPC_HPP_SERVER_IMPL) && !defined(RPC_HPP_MODULE_IMPL)
#  error At least one implementation type must be defined using 'RPC_HPP_{CLIENT, SERVER, MODULE}_IMPL'
#endif

#include <cassert>     // for assert
#include <cstddef>     // for size_t
#include <optional>    // for nullopt, optional
#include <stdexcept>   // for runtime_error
#include <string>      // for string
#include <tuple>       // for tuple, forward_as_tuple
#include <type_traits> // for declval, false_type, is_same, integral_constant
#include <utility>     // for move, index_sequence, make_index_sequence

#if defined(RPC_HPP_MODULE_IMPL) || defined(RPC_HPP_SERVER_IMPL)
#  include <functional>    // for function
#  include <unordered_map> // for unordered_map
#endif

#if defined(RPC_HPP_SERVER_IMPL) || defined(RPC_HPP_MODULE_IMPL)
#  define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) extern RETURN FUNCNAME(__VA_ARGS__)
#elif defined(RPC_HPP_CLIENT_IMPL)
#  define RPC_HEADER_FUNC(RETURN, FUNCNAME, ...) inline RETURN (*FUNCNAME)(__VA_ARGS__) = nullptr
#endif

#define RPC_HPP_PRECONDITION(EXPR) assert(EXPR)
#define RPC_HPP_POSTCONDITION(EXPR) assert(EXPR)

#if defined(__GNUC__) || defined(__clang__)
#  define RPC_HPP_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define RPC_HPP_INLINE __forceinline
#else
#  define RPC_HPP_INLINE
#endif

///@brief Top-level namespace for rpc.hpp classes and functions
namespace rpc_hpp
{
///@brief Array containing the version information for rpc.hpp
static constexpr unsigned version[]{ 0, 8, 1 };

enum class exception_type
{
    none,
    func_not_found,
    remote_exec,
    serialization,
    deserialization,
    signature_mismatch,
    client_send,
    client_receive,
    server_send,
    server_receive,
};

class rpc_exception : public std::runtime_error
{
public:
    explicit rpc_exception(const std::string& mesg, exception_type type) noexcept
        : std::runtime_error(mesg), m_type(type)
    {
    }

    explicit rpc_exception(const char* mesg, exception_type type) noexcept
        : std::runtime_error(mesg), m_type(type)
    {
    }

    exception_type get_type() const noexcept { return m_type; }

private:
    exception_type m_type;
};

class function_not_found : public rpc_exception
{
public:
    explicit function_not_found(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }

    explicit function_not_found(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::func_not_found)
    {
    }
};

class remote_exec_error : public rpc_exception
{
public:
    explicit remote_exec_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }

    explicit remote_exec_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::remote_exec)
    {
    }
};

class serialization_error : public rpc_exception
{
public:
    explicit serialization_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::serialization)
    {
    }

    explicit serialization_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::serialization)
    {
    }
};

class deserialization_error : public rpc_exception
{
public:
    explicit deserialization_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }

    explicit deserialization_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::deserialization)
    {
    }
};

class function_mismatch : public rpc_exception
{
public:
    explicit function_mismatch(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }

    explicit function_mismatch(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::signature_mismatch)
    {
    }
};

class client_send_error : public rpc_exception
{
public:
    explicit client_send_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::client_send)
    {
    }

    explicit client_send_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::client_send)
    {
    }
};

class client_receive_error : public rpc_exception
{
public:
    explicit client_receive_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }

    explicit client_receive_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::client_receive)
    {
    }
};

class server_send_error : public rpc_exception
{
public:
    explicit server_send_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::server_send)
    {
    }

    explicit server_send_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::server_send)
    {
    }
};

class server_receive_error : public rpc_exception
{
public:
    explicit server_receive_error(const std::string& mesg) noexcept
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }

    explicit server_receive_error(const char* mesg) noexcept
        : rpc_exception(mesg, exception_type::server_receive)
    {
    }
};

namespace adapters
{
    template<typename T>
    struct serial_traits;
}

///@brief Namespace for implementation details, should not be used outside of the library
namespace detail
{
#if !defined(RPC_HPP_DOXYGEN_GEN)
    template<typename, typename T>
    struct is_serializable_base
    {
        static_assert(std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type");
    };

    template<typename C, typename R, typename... Args>
    struct is_serializable_base<C, R(Args...)>
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().serialize(std::declval<Args>()...)),
                R>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename, typename T>
    struct is_deserializable_base
    {
        static_assert(std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type");
    };

    template<typename C, typename R, typename... Args>
    struct is_deserializable_base<C, R(Args...)>
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().deserialize(std::declval<Args>()...)),
                R>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename Serial, typename Value>
    struct is_serializable :
        std::integral_constant<bool,
            is_serializable_base<Value, typename Serial::serial_t(const Value&)>::value
                && is_deserializable_base<Value, Value(const typename Serial::serial_t&)>::value>
    {
    };

    template<typename Serial, typename Value>
    inline constexpr bool is_serializable_v = is_serializable<Serial, Value>::value;

    template<typename C>
    struct has_begin
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().begin()), typename T::iterator>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct has_end
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().end()), typename T::iterator>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct has_size
    {
    private:
        template<typename T>
        static constexpr auto check(T*) noexcept ->
            typename std::is_same<decltype(std::declval<T>().size()), size_t>::type;

        template<typename>
        static constexpr std::false_type check(...) noexcept;

        using type = decltype(check<C>(nullptr));

    public:
        static constexpr bool value = type::value;
    };

    template<typename C>
    struct is_container :
        std::integral_constant<bool, has_size<C>::value && has_begin<C>::value && has_end<C>::value>
    {
    };

    template<typename C>
    inline constexpr bool is_container_v = is_container<C>::value;

    template <typename>
    struct is_tuple: std::false_type {};

    template <typename ...T>
    struct is_tuple<std::tuple<T...>>: std::true_type {};

    template<typename T>
    inline constexpr bool is_tuple_v = is_tuple<T>::value;

    template<typename F, typename... Ts, size_t... Is>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func,
        [[maybe_unused]] std::index_sequence<Is...> iseq)
    {
        using expander = int[];
        std::ignore = expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, const F& func)
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

    template<typename F, typename... Ts, size_t... Is>
    constexpr void for_each_tuple(std::tuple<Ts...>& tuple, const F& func,
                                  [[maybe_unused]] std::index_sequence<Is...> iseq)
    {
        using expander = int[];
        std::ignore = expander{ 0, ((void)func(std::get<Is>(tuple)), 0)... };
    }

    template<typename F, typename... Ts>
    constexpr void for_each_tuple(std::tuple<Ts...>& tuple, const F& func)
    {
        for_each_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

#  if defined(RPC_HPP_CLIENT_IMPL)
    // Allows passing in string literals
    template<typename T>
    struct decay_str
    {
        static_assert(!std::is_pointer_v<std::remove_cv_t<std::remove_reference_t<T>>>,
            "Pointer parameters are not allowed");

        static_assert(!std::is_array_v<std::remove_cv_t<std::remove_reference_t<T>>>,
            "C-style array parameters are not allowed");

        using type = T;
    };

    template<>
    struct decay_str<const char*>
    {
        using type = const std::string&;
    };

    template<>
    struct decay_str<const char*&>
    {
        using type = const std::string&;
    };

    template<>
    struct decay_str<const char* const&>
    {
        using type = const std::string&;
    };

    template<size_t N>
    struct decay_str<const char (&)[N]>
    {
        using type = const std::string&;
    };

    template<typename T>
    using decay_str_t = typename decay_str<T>::type;

    template<typename... Args, size_t... Is>
    constexpr void tuple_bind(
        const std::tuple<std::remove_cv_t<std::remove_reference_t<decay_str_t<Args>>>...>& src,
        std::index_sequence<Is...>, Args&&... dest)
    {
        using expander = int[];
        std::ignore = expander{ 0,
            (
                (void)[](auto&& x, auto&& y) {
                    if constexpr (
                        std::is_reference_v<
                            decltype(x)> && !std::is_const_v<std::remove_reference_t<decltype(x)>> && !std::is_pointer_v<std::remove_reference_t<decltype(x)>>)
                    {
                        x = std::forward<decltype(y)>(y);
                    }
                }(dest, std::get<Is>(src)),
                0)... };
    }

    template<typename... Args>
    constexpr void tuple_bind(
        const std::tuple<std::remove_cv_t<std::remove_reference_t<decay_str_t<Args>>>...>& src,
        Args&&... dest)
    {
        tuple_bind(src, std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(dest)...);
    }
#  endif

    template<typename... Args>
    class packed_func_base
    {
    public:
        using args_t = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

        packed_func_base() = delete;
        packed_func_base(std::string func_name, args_t args) noexcept
            : m_func_name(std::move(func_name)), m_args(std::move(args))
        {
        }

        explicit operator bool() const noexcept { return m_except_type == exception_type::none; }
        const std::string& get_err_mesg() const noexcept { return m_err_mesg; }
        const std::string& get_func_name() const noexcept { return m_func_name; }
        exception_type get_except_type() const noexcept { return m_except_type; }

        void set_exception(std::string&& mesg, const exception_type type) & noexcept
        {
            m_except_type = type;
            m_err_mesg = std::move(mesg);
        }

        const args_t& get_args() const noexcept { return m_args; }
        args_t& get_args() noexcept { return m_args; }

    protected:
        ~packed_func_base() noexcept = default;
        packed_func_base(const packed_func_base&) = default;
        packed_func_base(packed_func_base&&) noexcept = default;
        packed_func_base& operator=(const packed_func_base&) & = default;
        packed_func_base& operator=(packed_func_base&&) & noexcept = default;

        [[noreturn]] void throw_ex() const noexcept(false)
        {
            switch (m_except_type)
            {
                case exception_type::func_not_found:
                    throw function_not_found(m_err_mesg);

                case exception_type::remote_exec:
                    throw remote_exec_error(m_err_mesg);

                case exception_type::serialization:
                    throw serialization_error(m_err_mesg);

                case exception_type::deserialization:
                    throw deserialization_error(m_err_mesg);

                case exception_type::signature_mismatch:
                    throw function_mismatch(m_err_mesg);

                case exception_type::client_send:
                    throw client_send_error(m_err_mesg);

                case exception_type::client_receive:
                    throw client_receive_error(m_err_mesg);

                case exception_type::server_send:
                    throw server_send_error(m_err_mesg);

                case exception_type::server_receive:
                    throw server_receive_error(m_err_mesg);

                case exception_type::none:
                default:
                    throw rpc_exception(m_err_mesg, exception_type::none);
            }
        }

    private:
        exception_type m_except_type{ exception_type::none };
        std::string m_func_name;
        std::string m_err_mesg{};
        args_t m_args;
    };

    template<typename R, typename... Args>
    class packed_func final : public packed_func_base<Args...>
    {
    public:
        using result_t = R;
        using typename packed_func_base<Args...>::args_t;

        packed_func(std::string func_name, std::optional<result_t> result, args_t args) noexcept
            : packed_func_base<Args...>(std::move(func_name), std::move(args)),
              m_result(std::move(result))
        {
        }

        explicit operator bool() const noexcept
        {
            return m_result.has_value() && packed_func_base<Args...>::operator bool();
        }

        const R& get_result() const
        {
            if (!static_cast<bool>(*this))
            {
                // throws exception based on except_type
                this->throw_ex();
            }

            return m_result.value();
        }

        void set_result(const R& value) & noexcept(std::is_nothrow_copy_assignable_v<R>)
        {
            m_result = value;
        }

        void set_result(R&& value) & noexcept(std::is_nothrow_move_assignable_v<R>)
        {
            m_result = std::move(value);
        }

        void clear_result() & noexcept { m_result.reset(); }

    private:
        std::optional<result_t> m_result{};
    };

    template<typename... Args>
    class packed_func<void, Args...> final : public packed_func_base<Args...>
    {
    public:
        using result_t = void;
        using typename packed_func_base<Args...>::args_t;
        using packed_func_base<Args...>::packed_func_base;
        using packed_func_base<Args...>::operator bool;

        void get_result() const
        {
            if (!static_cast<bool>(*this))
            {
                // throws exception based on except_type
                this->throw_ex();
            }
        }
    };

    template<typename Adapter>
    struct serial_adapter_base
    {
        using serial_t = typename adapters::serial_traits<Adapter>::serial_t;
        using bytes_t = typename adapters::serial_traits<Adapter>::bytes_t;

        static std::optional<serial_t> from_bytes(bytes_t&& bytes) = delete;
        static bytes_t to_bytes(serial_t&& serial_obj) = delete;
        static serial_t empty_object() = delete;

        template<typename R, typename... Args>
        static serial_t serialize_pack(const packed_func<R, Args...>& pack) = delete;

        template<typename R, typename... Args>
        static packed_func<R, Args...> deserialize_pack(const serial_t& serial_obj) = delete;

        static std::string get_func_name(const serial_t& serial_obj) = delete;
        static rpc_exception extract_exception(const serial_t& serial_obj) = delete;
        static void set_exception(serial_t& serial_obj, const rpc_exception& ex) = delete;
    };
#endif
} // namespace detail

#if defined(RPC_HPP_SERVER_IMPL) || defined(RPC_HPP_MODULE_IMPL)
///@brief Namespace containing functions and classes only relevant to "server-side" implentations
///
///@note Is only compiled by defining either @ref RPC_HPP_SERVER_IMPL AND/OR @ref RPC_HPP_MODULE_IMPL
inline namespace server
{
    ///@brief Class defining an interface for serving functions via RPC
    ///
    ///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
    template<typename Serial>
    class server_interface
    {
    public:
        using adapter_t = Serial;

        server_interface() noexcept = default;

        // Prevent copying
        server_interface(const server_interface&) = delete;
        server_interface& operator=(const server_interface&) = delete;

        server_interface(server_interface&&) noexcept = default;
        server_interface& operator=(server_interface&&) noexcept = default;

#  if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        ///@brief Gets a reference to the server's function cache
        ///
        ///@tparam Val Type of the return value for a function
        ///@param func_name Name of the function to get the cached return value(s) for
        ///@return std::unordered_map<typename Serial::bytes_t, Val>& Reference to the hashmap containing the return values with the serialized function call as the key
        template<typename Val>
        std::unordered_map<typename Serial::bytes_t, Val>& get_func_cache(
            const std::string& func_name)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            update_all_cache<Val>(func_name);
            return *static_cast<std::unordered_map<typename Serial::bytes_t, Val>*>(
                m_cache_map.at(func_name));
        }

        ///@brief Clears the server's function cache
        RPC_HPP_INLINE void clear_all_cache() noexcept { m_cache_map.clear(); }
#  endif

        ///@brief Binds a string to a callback, utilizing the server's cache
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func_name Name to bind the callback to
        ///@param func_ptr Pointer to callback that runs when dispatch is called with bound name
        template<typename R, typename... Args>
        void bind_cached(std::string func_name, std::function<R(Args...)> &&func)
        {
            m_dispatch_table.emplace(std::move(func_name),
                [this, func = std::forward<decltype(func)>(func)](typename Serial::serial_t& serial_obj)
                {
                    try
                    {
                        dispatch_cached_func(std::forward<decltype(func)>(func), serial_obj);
                    }
                    catch (const rpc_exception& ex)
                    {
                        Serial::set_exception(serial_obj, ex);
                    }
                });
        }

        ///@brief Binds a string to a callback, utilizing the server's cache
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@tparam F Callback type (could be function or lambda or functor)
        ///@param func_name Name to bind the callback to
        ///@param func Callback to run when dispatch is called with bound name
        template<typename R, typename... Args>
        RPC_HPP_INLINE void bind_cached(std::string func_name, R (*func_ptr)(Args...))
        {
            using fptr_t = std::function<R(Args...)>;

            bind_cached(std::move(func_name), fptr_t{ func_ptr });
        }

        ///@brief Binds a string to a callback
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@param func_name Name to bind the callback to
        ///@param func_ptr Pointer to callback that runs when dispatch is called with bound name
        template<typename R, typename... Args>
        void bind(std::string func_name, std::function<R(Args...)> &&func)
        {
            m_dispatch_table.emplace(std::move(func_name),
                [func = std::forward<decltype(func)>(func)](typename Serial::serial_t& serial_obj)
                {
                    try
                    {
                        dispatch_func(std::forward<decltype(func)>(func), serial_obj);
                    }
                    catch (const rpc_exception& ex)
                    {
                        Serial::set_exception(serial_obj, ex);
                    }
                });
        }

        ///@brief Binds a string to a callback
        ///
        ///@tparam R Return type of the callback function
        ///@tparam Args Variadic argument type(s) for the function
        ///@tparam F Callback type (could be function or lambda or functor)
        ///@param func_name Name to bind the callback to
        ///@param func Callback to run when dispatch is called with bound name
        template<typename R, typename... Args>
        RPC_HPP_INLINE void bind(std::string func_name, R (*func_ptr)(Args...))
        {
            using fptr_t = std::function<R(Args...)>;

            bind(std::move(func_name), fptr_t{ func_ptr });
        }

        ///@brief Parses the received serialized data and determines which function to call
        ///
        ///@param bytes Data to be parsed into a serial object
        ///@return Serial::bytes_t Data parsed out of a serial object after dispatching the callback
        ///@note nodiscard because original bytes are consumed
        [[nodiscard]] typename Serial::bytes_t dispatch(typename Serial::bytes_t&& bytes) const
        {
            auto serial_obj = Serial::from_bytes(std::move(bytes));

            if (!serial_obj.has_value())
            {
                auto err_obj = Serial::empty_object();
                Serial::set_exception(err_obj, server_receive_error("Invalid RPC object received"));
                return Serial::to_bytes(std::move(err_obj));
            }

            const auto func_name = adapter_t::get_func_name(serial_obj.value());

            if (const auto it = m_dispatch_table.find(func_name); it != m_dispatch_table.end())
            {
                it->second(serial_obj.value());
                return Serial::to_bytes(std::move(serial_obj).value());
            }

            Serial::set_exception(serial_obj.value(),
                function_not_found("RPC error: Called function: \"" + func_name + "\" not found"));

            return Serial::to_bytes(std::move(serial_obj).value());
        }

    protected:
        ~server_interface() noexcept = default;

#  if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        template<typename R, typename... Args>
        void dispatch_cached_func(std::function<R(Args...)> &func, typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto pack = [&serial_obj]
            {
                try
                {
                    return Serial::template deserialize_pack<R, Args...>(serial_obj);
                }
                catch (const rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw deserialization_error(ex.what());
                }
            }();

            auto& result_cache = get_func_cache<R>(pack.get_func_name());

            if constexpr (!std::is_void_v<R>)
            {
                auto bytes = Serial::to_bytes(std::move(serial_obj));

                if (const auto it = result_cache.find(bytes); it != result_cache.end())
                {
                    pack.set_result(it->second);

                    try
                    {
                        serial_obj = Serial::template serialize_pack<R, Args...>(pack);
                        return;
                    }
                    catch (const rpc_exception&)
                    {
                        throw;
                    }
                    catch (const std::exception& ex)
                    {
                        throw serialization_error(ex.what());
                    }
                }

                run_callback(std::forward<decltype(func)>(func), pack);
                result_cache[std::move(bytes)] = pack.get_result();
            }
            else
            {
                run_callback(std::forward<decltype(func)>(func), pack);
            }

            try
            {
                serial_obj = Serial::template serialize_pack<R, Args...>(pack);
            }
            catch (const rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw serialization_error(ex.what());
            }
        }
#  else
        template<typename R, typename... Args>
        RPC_HPP_INLINE void dispatch_cached_func(
            const std::function<R(Args...)> &&func, typename Serial::serial_t& serial_obj) const
        {
            dispatch_func(std::forward<decltype(func)>(func), serial_obj);
        }
#  endif

        template<typename R, typename... Args>
        static void dispatch_func(const std::function<R(Args...)> &&func, typename Serial::serial_t& serial_obj)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto pack = [&serial_obj]
            {
                try
                {
                    return Serial::template deserialize_pack<R, Args...>(serial_obj);
                }
                catch (const rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw deserialization_error(ex.what());
                }
            }();

            run_callback(std::forward<decltype(func)>(func), pack);

            try
            {
                serial_obj = Serial::template serialize_pack<R, Args...>(pack);
            }
            catch (const rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw serialization_error(ex.what());
            }
        }

    private:
        template<typename R, typename... Args>
        static void run_callback(const std::function<R(Args...)> &&func, detail::packed_func<R, Args...>& pack)
        {
            RPC_HPP_PRECONDITION(func != nullptr);

            auto& args = pack.get_args();

            if constexpr (std::is_void_v<R>)
            {
                try
                {
                    std::apply(func, args);
                }
                catch (const std::exception& ex)
                {
                    throw remote_exec_error(ex.what());
                }
            }
            else
            {
                try
                {
                    auto result = std::apply(func, args);
                    pack.set_result(std::move(result));
                }
                catch (const std::exception& ex)
                {
                    throw remote_exec_error(ex.what());
                }
            }
        }

#  if defined(RPC_HPP_SERVER_IMPL) && defined(RPC_HPP_ENABLE_SERVER_CACHE)
        template<typename Val>
        static void* get_func_cache_impl(const std::string& func_name)
        {
            static std::unordered_map<std::string,
                std::unordered_map<typename Serial::bytes_t, Val>>
                cache{};

            return &cache[func_name];
        }

        template<typename Val>
        void update_all_cache(const std::string& func_name)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            m_cache_map.insert_or_assign(func_name, get_func_cache_impl<Val>(func_name));
        }

        std::unordered_map<std::string, void*> m_cache_map{};
#  endif

        std::unordered_map<std::string, std::function<void(typename Serial::serial_t&)>>
            m_dispatch_table{};
    };
} // namespace server
#endif

#if defined(RPC_HPP_CLIENT_IMPL)
#  define call_header_func(FUNCNAME, ...) call_header_func_impl(FUNCNAME, #  FUNCNAME, __VA_ARGS__)

///@brief Namespace containing functions and classes only relevant to "client-side" implentations
///
///@note Is only compiled by defining @ref RPC_HPP_CLIENT_IMPL
inline namespace client
{
    ///@brief Class defining an interface for calling into an RPC server or module
    ///
    ///@tparam Serial serial_adapter type that controls how objects are serialized/deserialized
    template<typename Serial>
    class client_interface
    {
    public:
        virtual ~client_interface() noexcept = default;
        client_interface() noexcept = default;

        // Prevent copying
        client_interface(const client_interface&) = delete;
        client_interface& operator=(const client_interface&) = delete;

        // Prevent slicing
        client_interface& operator=(client_interface&&) = delete;

        ///@brief Sends an RPC call request to a server, waits for a response, then returns the result
        ///
        ///@tparam R Return type of the remote function to call
        ///@tparam Args Variadic argument type(s) of the remote function to call
        ///@param func_name Name of the remote function to call
        ///@param args Argument(s) for the remote function
        ///@return R Result of the function call, will throw with server's error message if the result does not exist
        ///@throws client_send_error Thrown if error occurs during the @ref send function
        ///@throws client_receive_error Thrown if error occurs during the @ref receive function
        ///@note nodiscard because an expensive remote procedure call is being performed
        template<typename R = void, typename... Args>
        [[nodiscard]] R call_func(std::string func_name, Args&&... args)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            auto bytes =
                serialize_call<R, Args...>(std::move(func_name), std::forward<Args>(args)...);

            try
            {
                send(std::move(bytes));
            }
            catch (const std::exception& ex)
            {
                throw client_send_error(ex.what());
            }

            try
            {
                bytes = receive();
            }
            catch (const std::exception& ex)
            {
                throw client_receive_error(ex.what());
            }

            const auto pack = deserialize_call<R, Args...>(std::move(bytes));

            // Assign values back to any (non-const) reference members
            detail::tuple_bind(pack.get_args(), std::forward<Args>(args)...);
            return pack.get_result();
        }

        ///@brief Sends an RPC call request to a server, waits for a response, then returns the result
        ///
        ///@tparam R Return type of the remote function to call
        ///@tparam Args Variadic argument type(s) of the remote function to call
        ///@param func Pointer to the function definition to deduce signature
        ///@param func_name Name of the remote function to call
        ///@param args Argument(s) for the remote function
        ///@return R Result of the function call, will throw with server's error message if the result does not exist
        ///@note nodiscard because an expensive remote procedure call is being performed
        template<typename R, typename... Args>
        [[nodiscard]] R call_header_func_impl(
            [[maybe_unused]] std::function<R(Args...)> &func, std::string func_name, Args&&... args)
        {
            RPC_HPP_PRECONDITION(!func_name.empty());

            return call_func<R, Args...>(std::move(func_name), std::forward<Args>(args)...);
        }

    protected:
        client_interface(client_interface&&) noexcept = default;

        ///@brief Sends serialized data to a server or module
        ///
        ///@param bytes Serialized data to be sent
        virtual void send(const typename Serial::bytes_t& bytes) = 0;

        ///@brief Receives serialized data from a server or module
        ///
        ///@return Serial::bytes_t Received serialized data
        virtual typename Serial::bytes_t receive() = 0;

    private:
        template<typename R, typename... Args>
        static RPC_HPP_INLINE typename Serial::bytes_t serialize_call(
            std::string func_name, Args&&... args)
        {
            detail::packed_func<R, detail::decay_str_t<Args>...> pack = [&]() noexcept
            {
                if constexpr (std::is_void_v<R>)
                {
                    return detail::packed_func<void, detail::decay_str_t<Args>...>{
                        std::move(func_name), std::forward_as_tuple(args...)
                    };
                }
                else
                {
                    return detail::packed_func<R, detail::decay_str_t<Args>...>{
                        std::move(func_name), std::nullopt, std::forward_as_tuple(args...)
                    };
                }
            }();

            auto serial_obj = [&pack]
            {
                try
                {
                    return Serial::serialize_pack(pack);
                }
                catch (const rpc_exception&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw serialization_error(ex.what());
                }
            }();

            return Serial::to_bytes(std::move(serial_obj));
        }

        template<typename R, typename... Args>
        static RPC_HPP_INLINE auto deserialize_call(typename Serial::bytes_t&& bytes)
        {
            const auto ret_obj = Serial::from_bytes(std::move(bytes));

            if (!ret_obj.has_value())
            {
                throw client_receive_error("Client received invalid RPC object");
            }

            try
            {
                return Serial::template deserialize_pack<R, detail::decay_str_t<Args>...>(
                    ret_obj.value());
            }
            catch (const rpc_exception&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error(ex.what());
            }
        }
    };
} // namespace client
#endif
} // namespace rpc_hpp
