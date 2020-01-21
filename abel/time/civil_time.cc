//

#include <abel/time/civil_time.h>

#include <cstdlib>
#include <string>

#include <abel/strings/str_cat.h>
#include <abel/time/time.h>

namespace abel {


namespace {

// Since a civil time has a larger year range than abel::abel_time (64-bit years vs
// 64-bit seconds, respectively) we normalize years to roughly +/- 400 years
// around the year 2400, which will produce an equivalent year in a range that
// abel::abel_time can handle.
ABEL_FORCE_INLINE civil_year_t NormalizeYear(civil_year_t year) {
  return 2400 + year % 400;
}

// Formats the given CivilSecond according to the given format.
std::string FormatYearAnd(string_view fmt, CivilSecond cs) {
  const CivilSecond ncs(NormalizeYear(cs.year()), cs.month(), cs.day(),
                        cs.hour(), cs.minute(), cs.second());
  const time_zone utc = utc_time_zone();
  // TODO(abel-team): Avoid conversion of fmt std::string.
  return string_cat(cs.year(),
                format_time(std::string(fmt), from_civil(ncs, utc), utc));
}

template <typename CivilT>
bool ParseYearAnd(string_view fmt, string_view s, CivilT* c) {
  // Civil times support a larger year range than abel::abel_time, so we need to
  // parse the year separately, normalize it, then use abel::parse_time on the
  // normalized std::string.
  const std::string ss = std::string(s);  // TODO(abel-team): Avoid conversion.
  const char* const np = ss.c_str();
  char* endp;
  errno = 0;
  const civil_year_t y =
      std::strtoll(np, &endp, 10);  // NOLINT(runtime/deprecated_fn)
  if (endp == np || errno == ERANGE) return false;
  const std::string norm = string_cat(NormalizeYear(y), endp);

  const time_zone utc = utc_time_zone();
  abel_time t;
  if (parse_time(string_cat("%Y", fmt), norm, utc, &t, nullptr)) {
    const auto cs = to_civil_second(t, utc);
    *c = CivilT(y, cs.month(), cs.day(), cs.hour(), cs.minute(), cs.second());
    return true;
  }

  return false;
}

// Tries to parse the type as a CivilT1, but then assigns the result to the
// argument of type CivilT2.
template <typename CivilT1, typename CivilT2>
bool ParseAs(string_view s, CivilT2* c) {
  CivilT1 t1;
  if (ParseCivilTime(s, &t1)) {
    *c = CivilT2(t1);
    return true;
  }
  return false;
}

template <typename CivilT>
bool ParseLenient(string_view s, CivilT* c) {
  // A fastpath for when the given std::string data parses exactly into the given
  // type T (e.g., s="YYYY-MM-DD" and CivilT=CivilDay).
  if (ParseCivilTime(s, c)) return true;
  // Try parsing as each of the 6 types, trying the most common types first
  // (based on csearch results).
  if (ParseAs<CivilDay>(s, c)) return true;
  if (ParseAs<CivilSecond>(s, c)) return true;
  if (ParseAs<CivilHour>(s, c)) return true;
  if (ParseAs<CivilMonth>(s, c)) return true;
  if (ParseAs<CivilMinute>(s, c)) return true;
  if (ParseAs<CivilYear>(s, c)) return true;
  return false;
}
}  // namespace

std::string FormatCivilTime(CivilSecond c) {
  return FormatYearAnd("-%m-%dT%H:%M:%S", c);
}
std::string FormatCivilTime(CivilMinute c) {
  return FormatYearAnd("-%m-%dT%H:%M", c);
}
std::string FormatCivilTime(CivilHour c) {
  return FormatYearAnd("-%m-%dT%H", c);
}
std::string FormatCivilTime(CivilDay c) { return FormatYearAnd("-%m-%d", c); }
std::string FormatCivilTime(CivilMonth c) { return FormatYearAnd("-%m", c); }
std::string FormatCivilTime(CivilYear c) { return FormatYearAnd("", c); }

bool ParseCivilTime(string_view s, CivilSecond* c) {
  return ParseYearAnd("-%m-%dT%H:%M:%S", s, c);
}
bool ParseCivilTime(string_view s, CivilMinute* c) {
  return ParseYearAnd("-%m-%dT%H:%M", s, c);
}
bool ParseCivilTime(string_view s, CivilHour* c) {
  return ParseYearAnd("-%m-%dT%H", s, c);
}
bool ParseCivilTime(string_view s, CivilDay* c) {
  return ParseYearAnd("-%m-%d", s, c);
}
bool ParseCivilTime(string_view s, CivilMonth* c) {
  return ParseYearAnd("-%m", s, c);
}
bool ParseCivilTime(string_view s, CivilYear* c) {
  return ParseYearAnd("", s, c);
}

bool ParseLenientCivilTime(string_view s, CivilSecond* c) {
  return ParseLenient(s, c);
}
bool ParseLenientCivilTime(string_view s, CivilMinute* c) {
  return ParseLenient(s, c);
}
bool ParseLenientCivilTime(string_view s, CivilHour* c) {
  return ParseLenient(s, c);
}
bool ParseLenientCivilTime(string_view s, CivilDay* c) {
  return ParseLenient(s, c);
}
bool ParseLenientCivilTime(string_view s, CivilMonth* c) {
  return ParseLenient(s, c);
}
bool ParseLenientCivilTime(string_view s, CivilYear* c) {
  return ParseLenient(s, c);
}

namespace time_internal {

std::ostream& operator<<(std::ostream& os, CivilYear y) {
  return os << FormatCivilTime(y);
}
std::ostream& operator<<(std::ostream& os, CivilMonth m) {
  return os << FormatCivilTime(m);
}
std::ostream& operator<<(std::ostream& os, CivilDay d) {
  return os << FormatCivilTime(d);
}
std::ostream& operator<<(std::ostream& os, CivilHour h) {
  return os << FormatCivilTime(h);
}
std::ostream& operator<<(std::ostream& os, CivilMinute m) {
  return os << FormatCivilTime(m);
}
std::ostream& operator<<(std::ostream& os, CivilSecond s) {
  return os << FormatCivilTime(s);
}

}  // namespace time_internal


}  // namespace abel
