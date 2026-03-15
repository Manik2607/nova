#pragma once

#include <cstdint>
#include <source_location>
#include <iostream>
#include <cstdlib>

#if defined(NOVA_SHARED)
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(NOVA_BUILD_DLL)
            #define NOVA_API __declspec(dllexport)
        #else
            #define NOVA_API __declspec(dllimport)
        #endif
    #else
        #define NOVA_API __attribute__((visibility("default")))
    #endif
#else
    #define NOVA_API
#endif

namespace nova {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;

} // namespace nova

#if defined(NOVA_ENABLE_ASSERTS)
    #define NOVA_ASSERT(cond, msg) \
        do { \
            if (!(cond)) { \
                auto loc = std::source_location::current(); \
                std::cerr << "Assertion failed: " << #cond << "\n" \
                          << "Message: " << msg << "\n" \
                          << "File: " << loc.file_name() << ":" << loc.line() << "\n" \
                          << "Function: " << loc.function_name() << "\n"; \
                std::abort(); \
            } \
        } while (false)
#else
    #define NOVA_ASSERT(cond, msg) do { (void)sizeof(cond); } while(0)
#endif
