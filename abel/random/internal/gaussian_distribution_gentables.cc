//

// Generates gaussian_distribution.cc
//
// $ blaze run :gaussian_distribution_gentables > gaussian_distribution.cc
//
#include <abel/random/gaussian_distribution.h>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>

#include <abel/base/profile.h>

namespace abel {

namespace random_internal {
namespace {

template <typename T, size_t N>
void FormatArrayContents(std::ostream* os, T (&data)[N]) {
  if (!std::numeric_limits<T>::is_exact) {
    // Note: T is either an integer or a float.
    // float requires higher precision to ensure that values are
    // reproduced exactly.
    // Trivia: C99 has hexadecimal floating point literals, but C++11 does not.
    // Using them would remove all concern of precision loss.
    os->precision(std::numeric_limits<T>::max_digits10 + 2);
  }
  *os << "    {";
  std::string separator = "";
  for (size_t i = 0; i < N; ++i) {
    *os << separator << data[i];
    if ((i + 1) % 3 != 0) {
      separator = ", ";
    } else {
      separator = ",\n     ";
    }
  }
  *os << "}";
}

}  // namespace

class TableGenerator : public gaussian_distribution_base {
 public:
  TableGenerator();
  void Print(std::ostream* os);

  using gaussian_distribution_base::kMask;
  using gaussian_distribution_base::kR;
  using gaussian_distribution_base::kV;

 private:
  Tables tables_;
};

// Ziggurat gaussian initialization.  For an explanation of the algorithm, see
// the Marsaglia paper, "The Ziggurat Method for Generating Random Variables".
//   http://www.jstatsoft.org/v05/i08/
//
// Further details are available in the Doornik paper
//   https://www.doornik.com/research/ziggurat.pdf
//
TableGenerator::TableGenerator() {
  // The constants here should match the values in gaussian_distribution.h
  static constexpr int kC = kMask + 1;

  static_assert((ABEL_ARRAYSIZE(tables_.x) == kC + 1),
                "xArray must be length kMask + 2");

  static_assert((ABEL_ARRAYSIZE(tables_.x) == ABEL_ARRAYSIZE(tables_.f)),
                "fx and x arrays must be identical length");

  auto f = [](double x) { return std::exp(-0.5 * x * x); };
  auto f_inv = [](double x) { return std::sqrt(-2.0 * std::log(x)); };

  tables_.x[0] = kV / f(kR);
  tables_.f[0] = f(tables_.x[0]);

  tables_.x[1] = kR;
  tables_.f[1] = f(tables_.x[1]);

  tables_.x[kC] = 0.0;
  tables_.f[kC] = f(tables_.x[kC]);  // 1.0

  for (int i = 2; i < kC; i++) {
    double v = (kV / tables_.x[i - 1]) + tables_.f[i - 1];
    tables_.x[i] = f_inv(v);
    tables_.f[i] = v;
  }
}

void TableGenerator::Print(std::ostream* os) {
  *os << "// BEGIN GENERATED CODE; DO NOT EDIT\n"
         "// clang-format off\n"
         "\n"
         "#include \"abel/random/gaussian_distribution.h\"\n"
         "\n"
         // "namespace " and "abel" are broken apart so as not to conflict with
         // script that adds the LTS inline namespace.
         "namespace "
         "abel {\n"
         "namespace "
         "random_internal {\n"
         "\n"
         "const gaussian_distribution_base::Tables\n"
         "    gaussian_distribution_base::zg_ = {\n";
  FormatArrayContents(os, tables_.x);
  *os << ",\n";
  FormatArrayContents(os, tables_.f);
  *os << "};\n"
         "\n"
         "}  // namespace "
         "random_internal\n"
         "}  // namespace "
         "abel\n"
         "\n"
         "// clang-format on\n"
         "// END GENERATED CODE";
  *os << std::endl;
}

}  // namespace random_internal

}  // namespace abel

int main(int, char**) {
  std::cerr << "\nCopy the output to gaussian_distribution.cc" << std::endl;
  abel::random_internal::TableGenerator generator;
  generator.Print(&std::cout);
  return 0;
}
