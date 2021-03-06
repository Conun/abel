
#ifndef ABEL_TIME_CLOCK_H_
#define ABEL_TIME_CLOCK_H_

#include <abel/base/profile.h>
#include <abel/chrono/time.h>

namespace abel {


// now()
//
// Returns the current time, expressed as an `abel::abel_time` absolute time value.
abel::abel_time now();

// get_current_time_nanos()
//
// Returns the current time, expressed as a count of nanoseconds since the Unix
// Epoch (https://en.wikipedia.org/wiki/Unix_time). Prefer `abel::now()` instead
// for all but the most performance-sensitive cases (i.e. when you are calling
// this function hundreds of thousands of times per second).
int64_t get_current_time_nanos();

// sleep_for()
//
// Sleeps for the specified duration, expressed as an `abel::duration`.
//
// Notes:
// * signal interruptions will not reduce the sleep duration.
// * Returns immediately when passed a nonpositive duration.
void sleep_for(abel::duration duration);


}  // namespace abel

// -----------------------------------------------------------------------------
// Implementation Details
// -----------------------------------------------------------------------------

// In some build configurations we pass --detect-odr-violations to the
// gold linker.  This causes it to flag weak symbol overrides as ODR
// violations.  Because ODR only applies to C++ and not C,
// --detect-odr-violations ignores symbols not mangled with C++ names.
// By changing our extension points to be extern "C", we dodge this
// check.
extern "C" {
void abel_internal_sleep_for(abel::duration duration);
}  // extern "C"

ABEL_FORCE_INLINE void abel::sleep_for(abel::duration duration) {
  abel_internal_sleep_for(duration);
}

#endif  // ABEL_TIME_CLOCK_H_
