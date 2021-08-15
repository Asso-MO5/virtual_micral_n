#ifndef MICRALN_SPAN_COMPAT_H
#define MICRALN_SPAN_COMPAT_H

#ifdef __cpp_lib_concepts
// It should be __cpp_lib_span, but it's not defined on gcc 10.3.0 even if it should.
#include <span>
#else
#define TCB_SPAN_NAMESPACE_NAME std
#include "span.hpp"
#endif
#endif //MICRALN_SPAN_COMPAT_H
