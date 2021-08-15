#ifndef MICRALN_SPAN_COMPAT_H
#define MICRALN_SPAN_COMPAT_H

#if defined(__has_include)
#if __has_include(<span>)
#define HAS_SPAN
#endif
#endif

#if defined(HAS_SPAN)
#include <span>

#if not defined(__cpp_lib_span)
#pragma message(                                                                                   \
        "span header was found but does not define std::span. Needs new compatibility case.")
#error
#endif

#else
#define TCB_SPAN_NAMESPACE_NAME std
#include "span.hpp"
#endif
#endif //MICRALN_SPAN_COMPAT_H
