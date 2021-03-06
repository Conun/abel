//

// Wrappers around lsan_interface functions.
// When lsan is not linked in, these functions are not available,
// therefore abel code which depends on these functions is conditioned on the
// definition of LEAK_SANITIZER.
#include <abel/debugging/leak_check.h>

#ifndef LEAK_SANITIZER

namespace abel {

bool HaveLeakSanitizer() { return false; }
void DoIgnoreLeak(const void*) { }
void RegisterLivePointers(const void*, size_t) { }
void UnRegisterLivePointers(const void*, size_t) { }
LeakCheckDisabler::LeakCheckDisabler() { }
LeakCheckDisabler::~LeakCheckDisabler() { }

}  // namespace abel

#else

#include <sanitizer/lsan_interface.h>

namespace abel {

bool HaveLeakSanitizer() { return true; }
void DoIgnoreLeak(const void* ptr) { __lsan_ignore_object(ptr); }
void RegisterLivePointers(const void* ptr, size_t size) {
  __lsan_register_root_region(ptr, size);
}
void UnRegisterLivePointers(const void* ptr, size_t size) {
  __lsan_unregister_root_region(ptr, size);
}
LeakCheckDisabler::LeakCheckDisabler() { __lsan_disable(); }
LeakCheckDisabler::~LeakCheckDisabler() { __lsan_enable(); }

}  // namespace abel

#endif  // LEAK_SANITIZER
