//

#include <abel/container/internal/raw_hash_set.h>

#include <atomic>
#include <cstddef>

#include <abel/base/profile.h>

namespace abel {

namespace container_internal {

constexpr size_t Group::kWidth;

// Returns "random" seed.
ABEL_FORCE_INLINE size_t RandomSeed() {
#if ABEL_HAVE_THREAD_LOCAL
  static thread_local size_t counter = 0;
  size_t value = ++counter;
#else   // ABEL_HAVE_THREAD_LOCAL
  static std::atomic<size_t> counter(0);
  size_t value = counter.fetch_add(1, std::memory_order_relaxed);
#endif  // ABEL_HAVE_THREAD_LOCAL
  return value ^ static_cast<size_t>(reinterpret_cast<uintptr_t>(&counter));
}

bool ShouldInsertBackwards(size_t hash, ctrl_t* ctrl) {
  // To avoid problems with weak hashes and single bit tests, we use % 13.
  // TODO(kfm,sbenza): revisit after we do unconditional mixing
  return (H1(hash, ctrl) ^ RandomSeed()) % 13 > 6;
}

}  // namespace container_internal

}  // namespace abel
