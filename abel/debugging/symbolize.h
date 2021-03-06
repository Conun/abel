//
// -----------------------------------------------------------------------------
// File: symbolize.h
// -----------------------------------------------------------------------------
//
// This file configures the abel symbolizer for use in converting instruction
// pointer addresses (program counters) into human-readable names (function
// calls, etc.) within abel code.
//
// The symbolizer may be invoked from several sources:
//
//   * Implicitly, through the installation of an abel failure signal handler.
//     (See failure_signal_handler.h for more information.)
//   * By calling `Symbolize()` directly on a program counter you obtain through
//     `abel::GetStackTrace()` or `abel::GetStackFrames()`. (See stacktrace.h
//     for more information.
//   * By calling `Symbolize()` directly on a program counter you obtain through
//     other means (which would be platform-dependent).
//
// In all of the above cases, the symbolizer must first be initialized before
// any program counter values can be symbolized. If you are installing a failure
// signal handler, initialize the symbolizer before you do so.
//
// Example:
//
//   int main(int argc, char** argv) {
//     // Initialize the Symbolizer before installing the failure signal handler
//     abel::InitializeSymbolizer(argv[0]);
//
//     // Now you may install the failure signal handler
//     abel::FailureSignalHandlerOptions options;
//     abel::InstallFailureSignalHandler(options);
//
//     // Start running your main program
//     ...
//     return 0;
//  }
//
#ifndef ABEL_DEBUGGING_SYMBOLIZE_H_
#define ABEL_DEBUGGING_SYMBOLIZE_H_

#include <abel/debugging/internal/symbolize.h>

namespace abel {


// InitializeSymbolizer()
//
// Initializes the program counter symbolizer, given the path of the program
// (typically obtained through `main()`s `argv[0]`). The abel symbolizer
// allows you to read program counters (instruction pointer values) using their
// human-readable names within output such as stack traces.
//
// Example:
//
// int main(int argc, char *argv[]) {
//   abel::InitializeSymbolizer(argv[0]);
//   // Now you can use the symbolizer
// }
void InitializeSymbolizer(const char* argv0);

// Symbolize()
//
// Symbolizes a program counter (instruction pointer value) `pc` and, on
// success, writes the name to `out`. The symbol name is demangled, if possible.
// Note that the symbolized name may be truncated and will be NUL-terminated.
// Demangling is supported for symbols generated by GCC 3.x or newer). Returns
// `false` on failure.
//
// Example:
//
//   // Print a program counter and its symbol name.
//   static void DumpPCAndSymbol(void *pc) {
//     char tmp[1024];
//     const char *symbol = "(unknown)";
//     if (abel::Symbolize(pc, tmp, sizeof(tmp))) {
//       symbol = tmp;
//     }
//     abel::printf("%*p  %s\n", pc, symbol);
//  }
bool Symbolize(const void *pc, char *out, int out_size);


}  // namespace abel

#endif  // ABEL_DEBUGGING_SYMBOLIZE_H_
