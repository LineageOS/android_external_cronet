//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03 && !stdlib=libc++

// <utility>

// template <class T>
//     typename conditional
//     <
//         !is_nothrow_move_constructible<T>::value && is_copy_constructible<T>::value,
//         const T&,
//         T&&
//     >::type
//     move_if_noexcept(T& x);

#include <type_traits>
#include <utility>

#include "test_macros.h"

class A
{
    A(const A&);
    A& operator=(const A&);
public:

    A() {}
    A(A&&) {}
};

struct legacy
{
    legacy() {}
    legacy(const legacy&);
};

int main(int, char**)
{
    int i = 0;
    const int ci = 0;

    legacy l;
    A a;
    const A ca;

    static_assert((std::is_same<decltype(std::move_if_noexcept(i)), int&&>::value), "");
    static_assert((std::is_same<decltype(std::move_if_noexcept(ci)), const int&&>::value), "");
    static_assert((std::is_same<decltype(std::move_if_noexcept(a)), A&&>::value), "");
    static_assert((std::is_same<decltype(std::move_if_noexcept(ca)), const A&&>::value), "");
    static_assert((std::is_same<decltype(std::move_if_noexcept(l)), const legacy&>::value), "");

#if TEST_STD_VER > 11
    constexpr int i1 = 23;
    constexpr int i2 = std::move_if_noexcept(i1);
    static_assert(i2 == 23, "" );
#endif


  return 0;
}
