#ifndef BGL_LUA_H
#define BGL_LUA_H

#include <string>
#include <tuple>
#include <functional>
#include <map>
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
      static const std::string& get(lua_State* L, int index)
      {
        static std::map<int, std::string> _storage_map = {};
        _storage_map[index] = luaL_checkstring(L, index);
        return _storage_map[index];
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

    inline void pushmany(lua_State* L)
    {
    }

    template <typename T, typename ... Args>
    void pushmany(lua_State* L, T val, Args... args)
    {
      push(L, val);
      pushmany(L, args...);
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
      // forward_as_tuple seems to be required to preserve rvalues.
      return std::forward_as_tuple( assert_and_get<Args>::get(L, Index+1)... );
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
  }

  class LuaEnv
  {
  public:
    LuaEnv()
     : m_state(luaL_newstate())
    {
      luaopen_base(m_state);
      luaopen_math(m_state);
    }

    ~LuaEnv()
    {
      lua_close(m_state);
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
      auto function = (typename detail::lambda_traits<Functor>::Function)(functor);

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

    template <typename ... Args>
    void call_function(const char* func_name, Args... args)
    {
      lua_getglobal(m_state, func_name);

      size_t nArgs = sizeof...(Args);
      detail::pushmany(m_state, args...);

      lua_call(m_state, nArgs, 0);
    }

    template <typename T>
    void register_global(const char* global_name, T value)
    {
      detail::push(m_state, value);
      lua_setglobal(m_state, global_name);
    }

    bool executeFile(const std::string& filename)
    {
      if (luaL_dofile(m_state, filename.c_str()) != 0)
      {
        m_error = lua_tostring(m_state, -1);

        return false;
      }

      m_error.clear();

      return true;
    }

    bool executeLine(const std::string& line)
    {
      if (luaL_dostring(m_state, line.c_str()) != 0)
      {
        m_error = lua_tostring(m_state, -1);

        return false;
      }

      m_error.clear();

      return true;
    }

    const std::string& getError() const
    {
      return m_error;
    }
  private:
    lua_State* m_state;
    std::string m_error;
  };

  struct reg
  {
    reg(LuaEnv& _state)
     : state(_state) {}

    // std::function.
    template <typename ReturnValue, typename ... Args>
    reg& operator()(const char* func_name, std::function<ReturnValue(Args...)> func)
    {
      state.register_function<ReturnValue, Args...>(func_name, func);

      return *this;
    }

    template <typename Functor>
    reg& operator()(const char* func_name, Functor func)
    {
      state.register_function<Functor>(func_name, func);

      return *this;
    }

    // Function pointers.
    template <typename ReturnValue, typename ... Args>
    reg& operator()(const char* func_name, ReturnValue (*funcPtr) (Args...))
    {
      state.register_function<ReturnValue, Args...>(func_name, funcPtr);

      return *this;
    }

    // Member function pointers.
    template <typename ReturnValue, typename Type, typename ... Args>
    reg& operator()(const char* func_name, ReturnValue (Type::*funcPtr) (Args...))
    {
      state.register_function<ReturnValue, Type, Args...>(func_name, funcPtr);

      return *this;
    }

    template <typename ReturnValue, typename Type, typename ... Args>
    reg& operator()(const char* func_name, ReturnValue (Type::*funcPtr) (Args...) const)
    {
      state.register_function<ReturnValue, Type, Args...>(func_name, funcPtr);

      return *this;
    }
  private:
    LuaEnv& state;
  };
}

#endif
