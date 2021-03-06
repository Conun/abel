//

#ifndef ABEL_RANDOM_INTERNAL_RANDEN_DETECT_H_
#define ABEL_RANDOM_INTERNAL_RANDEN_DETECT_H_

#include <abel/base/profile.h>

namespace abel {

namespace random_internal {

// Returns whether the current CPU supports RandenHwAes implementation.
// This typically involves supporting cryptographic extensions on whichever
// platform is currently running.
bool CPUSupportsRandenHwAes();

}  // namespace random_internal

}  // namespace abel

#endif  // ABEL_RANDOM_INTERNAL_RANDEN_DETECT_H_
