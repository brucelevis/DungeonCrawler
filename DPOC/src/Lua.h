#ifndef LUA_H_
#define LUA_H_

#include <string>
#include <tuple>
#include <functional>
#include <lua.hpp>

// Lots of this adapted from this excellent article:
// http://www.jeremyong.com/blog/2014/01/14/interfacing-lua-with-templates-in-c-plus-plus-11-continued/

namespace lua
{
  namespace detail
  {
    template <typename T>
    struct assert_and_get
    {
      static T get(lua_State* L, int index)
      {
        return (T)lua_touserdata(L, index);
      }
    };

    template <>
    struct assert_and_get<float>
    {
      static float get(lua_State* L, int index)
      {
        return luaL_checknumber(L, index);
      }
    };

    template <>
    struct assert_and_get<double>
    {
      static double get(lua_State* L, int index)
      {
        return luaL_checknumber(L, index);
      }
    };

    template <>
    struct assert_and_get<int>
    {
      static int get(lua_State* L, int index)
      {
        return luaL_checkinteger(L, index);
      }
    };

    template <>
    struct assert_and_get<size_t>
    {
      static int get(lua_State* L, int index)
      {
        return luaL_checkinteger(L, index);
      }
    };

    template <>
    struct assert_and_get<bool>
    {
      static int get(lua_State* L, int index)
      {
        return lua_toboolean(L, index);
      }
    };

    template <>
    struct assert_and_get<std::string>
    {
      static std::string get(lua_State* L, int index)
      {
        return luaL_checkstring(L, index);
      }
    };

    template <>
    struct assert_and_get<const std::string&>
    {
      static std::string get(lua_State* L, int index)
      {
        return luaL_checkstring(L, index);
      }
    };

    template <>
    struct assert_and_get<const char*>
    {
      static const char* get(lua_State* L, int index)
      {
        return luaL_checkstring(L, index);
      }
    };
  }

  namespace detail
  {
    template <typename T>
    inline void push(lua_State* L, T val)
    {
      lua_pushlightuserdata(L, (void* )val);
    }

    inline void push(lua_State* L, int val)
    {
      lua_pushinteger(L, val);
    }

    inline void push(lua_State* L, size_t val)
    {
      lua_pushinteger(L, val);
    }

    inline void push(lua_State* L, const std::string& val)
    {
      lua_pushstring(L, val.c_str());
    }

    inline void push(lua_State* L, const char* val)
    {
      lua_pushstring(L, val);
    }

    inline void push(lua_State* L, float val)
    {
      lua_pushnumber(L, val);
    }

    inline void push(lua_State* L, double val)
    {
      lua_pushnumber(L, val);
    }

    inline void push(lua_State* L, bool val)
    {
      lua_pushboolean(L, val);
    }
  }

  namespace detail
  {
    template <std::size_t ... Index>
    struct parameter_pack {};

    // Recursively inherits from itself creating say, <5>, <4, 4, 5>, <3, 3, 4, 5> ... <0, 0, 1, 2, 3, 4, 5>
    template <std::size_t N, std::size_t ... Index>
    struct parameter_pack_builder : parameter_pack_builder<N - 1, N - 1, Index...> {};

    // Finally we hit specialization <0, rest>, which creates the parameter_pack<rest> giving 0,1,2,3,4,5.
    template <std::size_t ... Index>
    struct parameter_pack_builder<0, Index ...>
    {
      using type = parameter_pack<Index ...>;
    };
  }

  namespace detail
  {
    struct base_wrapper
    {
      virtual ~base_wrapper() {}
      virtual int call(lua_State* L) = 0;
    };

    template <typename ... Args, size_t ... Index>
    std::tuple<Args ...> pack_arguments(lua_State* L, parameter_pack<Index...>)
    {
      return std::make_tuple( assert_and_get<Args>::get(L, Index+1)... );
    }

    template <typename ... Args>
    std::tuple<Args ...> pack_arguments(lua_State* L)
    {
      constexpr size_t nArgs = sizeof...(Args);
      return pack_arguments<Args...>(L, typename parameter_pack_builder<nArgs>::type());
    }
  }

  namespace detail
  {
    template <typename ReturnValue, typename ... Args, size_t ... Index>
    ReturnValue call_function(std::function<ReturnValue(Args...)> func,
                              std::tuple<Args...> args,
                              parameter_pack<Index...>)
    {
      return func(std::get<Index>(args)...);
    }

    template <typename ReturnValue, typename ... Args>
    ReturnValue call_function(std::function<ReturnValue(Args...)> func,
                              std::tuple<Args...> args)
    {
      constexpr size_t nArgs = sizeof...(Args);

      return call_function(func, args, typename parameter_pack_builder<nArgs>::type());
    }
  }

  namespace detail
  {
    template <typename ReturnValue, typename ... Args>
    struct function_wrapper : public base_wrapper
    {
      function_wrapper(std::function<ReturnValue(Args...)> function)
       : m_function(function)
      {
      }

      int call(lua_State* L)
      {
        auto args = pack_arguments<Args...>(L);
        ReturnValue value = call_function(m_function, args);
        push(L, value);

        return 1;
      }
    private:
      std::function<ReturnValue(Args...)> m_function;
    };

    template <typename ... Args>
    struct function_wrapper<void, Args...> : public base_wrapper
    {
      function_wrapper(std::function<void(Args...)> function)
       : m_function(function)
      {
      }

      int call(lua_State* L)
      {
        auto args = pack_arguments<Args...>(L);
        call_function(m_function, args);

        return 0;
      }
    private:
      std::function<void(Args...)> m_function;
    };

  }

  namespace detail
  {
    inline base_wrapper* _get_function(lua_State* L)
    {
      return (base_wrapper* )lua_touserdata(L, lua_upvalueindex(1));
    }

    static int _lua_function_wrapper(lua_State* L)
    {
      base_wrapper* func = _get_function(L);

      return func->call(L);
    }

    inline void _register_function(lua_State* L, const char* func_name, base_wrapper* wrap)
    {
      //lua_pushstring(L, func_name);
      lua_pushlightuserdata(L, (void*)wrap);
      lua_pushcclosure(L, _lua_function_wrapper, 1);
      lua_setglobal(L, func_name);
    }
  }

  namespace detail
  {
    // From selene. Not sure how this works exactly.
    template <typename T>
    struct lambda_traits : public lambda_traits<decltype(&T::operator())> {};

    template <typename T, typename ReturnValue, typename ... Args>
    struct lambda_traits<ReturnValue(T::*)(Args...) const>
    {
      using Function = std::function<ReturnValue(Args...)>;
    };

    class LuaEnv
    {
    public:
      static LuaEnv& instance()
      {
        static LuaEnv env;
        return env;
      }

      template <typename ReturnValue, typename ... Args>
      void register_function(const char* func_name, std::function<ReturnValue(Args...)> func)
      {
        auto wrap = new detail::function_wrapper<ReturnValue, Args...>(func);

        detail::_register_function(m_state, func_name, wrap);
      }

      template <typename Functor>
      void register_function(const char* func_name, Functor functor)
      {
        auto function = (typename lambda_traits<Functor>::Function)(functor);

        register_function(func_name, function);
      }

      template <typename ReturnValue, typename ... Args>
      void register_function(const char* func_name, ReturnValue (*funcPtr) (Args...))
      {
        std::function<ReturnValue(Args...)> func { funcPtr };

        register_function(func_name, func);
      }

      template <typename ReturnValue, typename Type, typename ... Args>
      void register_function(const char* func_name, ReturnValue (Type::*funcPtr) (Args...))
      {
        std::function<ReturnValue(Type*, Args...)> func { funcPtr };

        register_function(func_name, func);
      }

      template <typename ReturnValue, typename Type, typename ... Args>
      void register_function(const char* func_name, ReturnValue (Type::*funcPtr) (Args...) const)
      {
        std::function<ReturnValue(Type*, Args...)> func { funcPtr };

        register_function(func_name, func);
      }

      bool executeFile(const std::string& filename)
      {
        if (luaL_dofile(m_state, filename.c_str()) != 0)
        {
          return false;
        }

        return true;
      }
    private:
      LuaEnv()
       : m_state(luaL_newstate())
      {
        luaopen_base(m_state);
        luaopen_math(m_state);
      }
    private:
      lua_State* m_state;
    };
  }

  struct reg
  {
    // std::function.
    template <typename ReturnValue, typename ... Args>
    reg operator()(const char* func_name, std::function<ReturnValue(Args...)> func)
    {
      detail::LuaEnv::instance().register_function<ReturnValue, Args...>(func_name, func);

      return reg{};
    }

    template <typename Functor>
    reg operator()(const char* func_name, Functor func)
    {
      detail::LuaEnv::instance().register_function<Functor>(func_name, func);

      return reg{};
    }

    // Function pointers.
    template <typename ReturnValue, typename ... Args>
    reg operator()(const char* func_name, ReturnValue (*funcPtr) (Args...))
    {
      detail::LuaEnv::instance().register_function<ReturnValue, Args...>(func_name, funcPtr);

      return reg{};
    }

    // Member function pointers.
    template <typename ReturnValue, typename Type, typename ... Args>
    reg operator()(const char* func_name, ReturnValue (Type::*funcPtr) (Args...))
    {
      detail::LuaEnv::instance().register_function<ReturnValue, Type, Args...>(func_name, funcPtr);

      return reg{};
    }

    template <typename ReturnValue, typename Type, typename ... Args>
    reg operator()(const char* func_name, ReturnValue (Type::*funcPtr) (Args...) const)
    {
      detail::LuaEnv::instance().register_function<ReturnValue, Type, Args...>(func_name, funcPtr);

      return reg{};
    }
  };

  inline bool run(const std::string& filename)
  {
    return detail::LuaEnv::instance().executeFile(filename);
  }
}

#endif
