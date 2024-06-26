///@file rpc_adapters/rpc_njson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting nlohmann's JSON library (https://github.com/nlohmann/json)
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

#include "../rpc.hpp"

#include <nlohmann/json.hpp>

namespace rpc_hpp
{
namespace adapters
{
    class njson_adapter;

    template<>
    struct serial_traits<njson_adapter>
    {
        using serial_t = nlohmann::json;
        using bytes_t = std::string;
    };

    class njson_adapter : public detail::serial_adapter_base<njson_adapter>
    {
    public:
        [[nodiscard]] static std::string to_bytes(nlohmann::json&& serial_obj)
        {
            auto msg_pack = nlohmann::json::to_msgpack(serial_obj);
            return {msg_pack.begin(), msg_pack.end()};
        }

        [[nodiscard]] static std::optional<nlohmann::json> from_bytes(std::string&& bytes)
        {
            nlohmann::json obj;

            try
            {
                obj = nlohmann::json::from_msgpack(bytes);
            }
            catch (const nlohmann::json::parse_error&)
            {
                return std::nullopt;
            }

            if (!obj.is_object())
            {
                return std::nullopt;
            }

            if (obj.contains("except_type"))
            {
                if (obj["except_type"] != 0 && !obj.contains("err_mesg"))
                {
                    return std::nullopt;
                }

                // Objects with exceptions can be otherwise empty
                return std::make_optional(std::move(obj));
            }

            if (const auto fname_it = obj.find("func_name");
                fname_it == obj.end() || !fname_it->is_string() || fname_it->empty())
            {
                return std::nullopt;
            }

            if (const auto args_it = obj.find("args"); args_it == obj.end() || !args_it->is_array())
            {
                return std::nullopt;
            }

            return std::make_optional(std::move(obj));
        }

        static nlohmann::json empty_object() { return nlohmann::json::object(); }

        template<typename R, typename... Args>
        [[nodiscard]] static nlohmann::json serialize_pack(
            const detail::packed_func<R, Args...>& pack)
        {
            nlohmann::json obj{};
            obj["func_name"] = pack.get_func_name();
            obj["args"] = nlohmann::json::array();
            auto& arg_arr = obj["args"];
            arg_arr.get_ref<nlohmann::json::array_t&>().reserve(sizeof...(Args));

            detail::for_each_tuple(pack.get_args(),
                [&arg_arr](auto&& elem)
                { push_args(std::forward<decltype(elem)>(elem), arg_arr); });

            if (!pack)
            {
                obj["except_type"] = pack.get_except_type();
                obj["err_mesg"] = pack.get_err_mesg();
                return obj;
            }

            if constexpr (!std::is_void_v<R>)
            {
                obj["result"] = {};
                push_arg(pack.get_result(), obj["result"]);
            }

            return obj;
        }

        template<typename R, typename... Args>
        [[nodiscard]] static detail::packed_func<R, Args...> deserialize_pack(
            const nlohmann::json& serial_obj)
        {
            const auto& args_val = serial_obj["args"];
            [[maybe_unused]] unsigned arg_counter = 0;
            typename detail::packed_func<R, Args...>::args_t args{ parse_args<Args>(
                args_val, arg_counter)... };

            if constexpr (std::is_void_v<R>)
            {
                detail::packed_func<void, Args...> pack(serial_obj["func_name"], std::move(args));

                if (serial_obj.contains("except_type"))
                {
                    pack.set_exception(serial_obj["err_mesg"],
                        static_cast<exception_type>(serial_obj["except_type"]));
                }

                return pack;
            }
            else
            {
                if (serial_obj.contains("result") && !serial_obj["result"].is_null())
                {
                    return detail::packed_func<R, Args...>(serial_obj["func_name"],
                        parse_arg<R>(serial_obj["result"]), std::move(args));
                }

                detail::packed_func<R, Args...> pack(
                    serial_obj["func_name"], std::nullopt, std::move(args));

                if (serial_obj.contains("except_type"))
                {
                    pack.set_exception(serial_obj["err_mesg"],
                        static_cast<exception_type>(serial_obj["except_type"]));
                }

                return pack;
            }
        }

        [[nodiscard]] static std::string get_func_name(const nlohmann::json& serial_obj)
        {
            return serial_obj["func_name"];
        }

        [[nodiscard]] static rpc_exception extract_exception(const nlohmann::json& serial_obj)
        {
            return rpc_exception{ serial_obj.at("err_mesg").get<std::string>(),
                static_cast<exception_type>(serial_obj.at("except_type").get<int>()) };
        }

        static void set_exception(nlohmann::json& serial_obj, const rpc_exception& ex)
        {
            serial_obj["except_type"] = ex.get_type();
            serial_obj["err_mesg"] = ex.what();
        }

        template<typename T>
        static nlohmann::json serialize(const T& val) = delete;

        template<typename T>
        static T deserialize(const nlohmann::json& serial_obj) = delete;

    private:
        // nodiscard because this function is pointless without checking the bool
        template<typename T>
        [[nodiscard]] static constexpr bool validate_arg(const nlohmann::json& arg) noexcept
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                return arg.is_boolean();
            }
            else if constexpr (std::is_integral_v<T>)
            {
                return arg.is_number() && !arg.is_number_float();
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                return arg.is_number_float();
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return arg.is_string();
            }
            else if constexpr (detail::is_container_v<T> && !std::is_same_v<T, nlohmann::json>)
            {
                return arg.is_array();
            }
            else
            {
                return !arg.is_null();
            }
        }

        // nodiscard because expect_type is consumed by the function
        [[nodiscard]] static std::string mismatch_string(
            std::string&& expect_type, const nlohmann::json& arg)
        {
            return { "njson expected type: " + std::move(expect_type)
                + ", got type: " + arg.type_name() };
        }

        template<typename T>
        static void push_arg(T&& arg, nlohmann::json& obj)
        {
            using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

            if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string> ||
                          std::is_same_v<no_ref_t, nlohmann::json>) {
                obj = std::forward<T>(arg);
            }
            else if constexpr (detail::is_container_v<no_ref_t>)
            {
                obj = nlohmann::json::array();
                obj.get_ref<nlohmann::json::array_t&>().reserve(arg.size());

                for (auto&& val : arg)
                {
                    push_args(std::forward<decltype(val)>(val), obj);
                }
            }
            else if constexpr (detail::is_tuple<no_ref_t>::value)
            {
                obj = nlohmann::json::array();
                obj.get_ref<nlohmann::json::array_t&>().reserve(std::tuple_size<no_ref_t>::value);
                unsigned arg_counter = 0;

                detail::for_each_tuple(arg, [&obj, &arg_counter](auto &&val){
                    nlohmann::json tmp{};
                    push_arg(std::forward<decltype(val)>(val), tmp);
                    obj[arg_counter++] = std::move(tmp);
                });
            }
            else if constexpr (detail::is_serializable_v<njson_adapter, no_ref_t>)
            {
                obj = no_ref_t::template serialize<njson_adapter>(std::forward<T>(arg));
            }
            else
            {
                obj = serialize<no_ref_t>(std::forward<T>(arg));
            }
        }

        template<typename T>
        static void push_args(T&& arg, nlohmann::json& obj_arr)
        {
            nlohmann::json tmp{};
            push_arg(std::forward<T>(arg), tmp);
            obj_arr.push_back(std::move(tmp));
        }

        // nodiscard because parsing can be expensive, and it makes no sense to not use the parsed result
        template<typename T>
        [[nodiscard]] static std::remove_cv_t<std::remove_reference_t<T>> parse_arg(
            const nlohmann::json& arg)
        {
            using no_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

            if (!validate_arg<T>(arg))
            {
                throw function_mismatch(mismatch_string(typeid(T).name(), arg));
            }

            if constexpr (std::is_same_v<no_ref_t, nlohmann::json>)
            {
                return arg;
            }
            else if constexpr (std::is_arithmetic_v<no_ref_t> || std::is_same_v<no_ref_t, std::string>)
            {
                return arg.get<no_ref_t>();
            }
            else if constexpr (detail::is_container_v<no_ref_t>)
            {
                using value_t = typename no_ref_t::value_type;

                no_ref_t container{};
                container.reserve(arg.size());
                unsigned arg_counter = 0;

                for (const auto& val : arg)
                {
                    // TODO val -> arg? Note that the first argument of parse_args requires an array
                    container.push_back(parse_args<value_t>(val, arg_counter));
                }

                return container;
            }
            else if constexpr (detail::is_tuple_v<no_ref_t>)
            {
                no_ref_t container{};
                unsigned arg_counter = 0;

                detail::for_each_tuple(container, [&arg, &arg_counter](auto &val) {
                    val = parse_args<decltype(val)>(arg, arg_counter);
                });
                return container;
            }
            else if constexpr (detail::is_serializable_v<njson_adapter, no_ref_t>)
            {
                return no_ref_t::template deserialize<njson_adapter>(arg);
            }
            else
            {
                return deserialize<no_ref_t>(arg);
            }
        }

        // nodiscard because parsing can be expensive, and it makes no sense to not use the parsed result
        template<typename T>
        [[nodiscard]] static std::remove_cv_t<std::remove_reference_t<T>> parse_args(
            const nlohmann::json& arg_arr, unsigned& index)
        {
            if (index >= arg_arr.size())
            {
                throw function_mismatch("Argument count mismatch");
            }

            const auto& arg = arg_arr.is_array() ? arg_arr[index++] : arg_arr;
            return parse_arg<T>(arg);
        }
    };
} // namespace adapters
} // namespace rpc_hpp
