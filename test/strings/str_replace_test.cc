//

#include <abel/strings/str_replace.h>

#include <list>
#include <map>
#include <tuple>

#include <gtest/gtest.h>
#include <abel/strings/str_cat.h>
#include <abel/strings/str_split.h>

TEST(string_replace_all, OneReplacement) {
  std::string s;

  // Empty std::string.
  s = abel::string_replace_all(s, {{"", ""}});
  EXPECT_EQ(s, "");
  s = abel::string_replace_all(s, {{"x", ""}});
  EXPECT_EQ(s, "");
  s = abel::string_replace_all(s, {{"", "y"}});
  EXPECT_EQ(s, "");
  s = abel::string_replace_all(s, {{"x", "y"}});
  EXPECT_EQ(s, "");

  // Empty substring.
  s = abel::string_replace_all("abc", {{"", ""}});
  EXPECT_EQ(s, "abc");
  s = abel::string_replace_all("abc", {{"", "y"}});
  EXPECT_EQ(s, "abc");
  s = abel::string_replace_all("abc", {{"x", ""}});
  EXPECT_EQ(s, "abc");

  // Substring not found.
  s = abel::string_replace_all("abc", {{"xyz", "123"}});
  EXPECT_EQ(s, "abc");

  // Replace entire std::string.
  s = abel::string_replace_all("abc", {{"abc", "xyz"}});
  EXPECT_EQ(s, "xyz");

  // Replace once at the start.
  s = abel::string_replace_all("abc", {{"a", "x"}});
  EXPECT_EQ(s, "xbc");

  // Replace once in the middle.
  s = abel::string_replace_all("abc", {{"b", "x"}});
  EXPECT_EQ(s, "axc");

  // Replace once at the end.
  s = abel::string_replace_all("abc", {{"c", "x"}});
  EXPECT_EQ(s, "abx");

  // Replace multiple times with varying lengths of original/replacement.
  s = abel::string_replace_all("ababa", {{"a", "xxx"}});
  EXPECT_EQ(s, "xxxbxxxbxxx");

  s = abel::string_replace_all("ababa", {{"b", "xxx"}});
  EXPECT_EQ(s, "axxxaxxxa");

  s = abel::string_replace_all("aaabaaabaaa", {{"aaa", "x"}});
  EXPECT_EQ(s, "xbxbx");

  s = abel::string_replace_all("abbbabbba", {{"bbb", "x"}});
  EXPECT_EQ(s, "axaxa");

  // Overlapping matches are replaced greedily.
  s = abel::string_replace_all("aaa", {{"aa", "x"}});
  EXPECT_EQ(s, "xa");

  // The replacements are not recursive.
  s = abel::string_replace_all("aaa", {{"aa", "a"}});
  EXPECT_EQ(s, "aa");
}

TEST(string_replace_all, ManyReplacements) {
  std::string s;

  // Empty std::string.
  s = abel::string_replace_all("", {{"", ""}, {"x", ""}, {"", "y"}, {"x", "y"}});
  EXPECT_EQ(s, "");

  // Empty substring.
  s = abel::string_replace_all("abc", {{"", ""}, {"", "y"}, {"x", ""}});
  EXPECT_EQ(s, "abc");

  // Replace entire std::string, one char at a time
  s = abel::string_replace_all("abc", {{"a", "x"}, {"b", "y"}, {"c", "z"}});
  EXPECT_EQ(s, "xyz");
  s = abel::string_replace_all("zxy", {{"z", "x"}, {"x", "y"}, {"y", "z"}});
  EXPECT_EQ(s, "xyz");

  // Replace once at the start (longer matches take precedence)
  s = abel::string_replace_all("abc", {{"a", "x"}, {"ab", "xy"}, {"abc", "xyz"}});
  EXPECT_EQ(s, "xyz");

  // Replace once in the middle.
  s = abel::string_replace_all(
      "Abc!", {{"a", "x"}, {"ab", "xy"}, {"b", "y"}, {"bc", "yz"}, {"c", "z"}});
  EXPECT_EQ(s, "Ayz!");

  // Replace once at the end.
  s = abel::string_replace_all(
      "Abc!",
      {{"a", "x"}, {"ab", "xy"}, {"b", "y"}, {"bc!", "yz?"}, {"c!", "z;"}});
  EXPECT_EQ(s, "Ayz?");

  // Replace multiple times with varying lengths of original/replacement.
  s = abel::string_replace_all("ababa", {{"a", "xxx"}, {"b", "XXXX"}});
  EXPECT_EQ(s, "xxxXXXXxxxXXXXxxx");

  // Overlapping matches are replaced greedily.
  s = abel::string_replace_all("aaa", {{"aa", "x"}, {"a", "X"}});
  EXPECT_EQ(s, "xX");
  s = abel::string_replace_all("aaa", {{"a", "X"}, {"aa", "x"}});
  EXPECT_EQ(s, "xX");

  // Two well-known sentences
  s = abel::string_replace_all("the quick brown fox jumped over the lazy dogs",
                          {
                              {"brown", "box"},
                              {"dogs", "jugs"},
                              {"fox", "with"},
                              {"jumped", "five"},
                              {"over", "dozen"},
                              {"quick", "my"},
                              {"the", "pack"},
                              {"the lazy", "liquor"},
                          });
  EXPECT_EQ(s, "pack my box with five dozen liquor jugs");
}

TEST(string_replace_all, ManyReplacementsInMap) {
  std::map<const char *, const char *> replacements;
  replacements["$who"] = "Bob";
  replacements["$count"] = "5";
  replacements["#Noun"] = "Apples";
  std::string s = abel::string_replace_all("$who bought $count #Noun. Thanks $who!",
                                      replacements);
  EXPECT_EQ("Bob bought 5 Apples. Thanks Bob!", s);
}

TEST(string_replace_all, ReplacementsInPlace) {
  std::string s = std::string("$who bought $count #Noun. Thanks $who!");
  int count;
  count = abel::string_replace_all({{"$count", abel::string_cat(5)},
                              {"$who", "Bob"},
                              {"#Noun", "Apples"}}, &s);
  EXPECT_EQ(count, 4);
  EXPECT_EQ("Bob bought 5 Apples. Thanks Bob!", s);
}

TEST(string_replace_all, ReplacementsInPlaceInMap) {
  std::string s = std::string("$who bought $count #Noun. Thanks $who!");
  std::map<abel::string_view, abel::string_view> replacements;
  replacements["$who"] = "Bob";
  replacements["$count"] = "5";
  replacements["#Noun"] = "Apples";
  int count;
  count = abel::string_replace_all(replacements, &s);
  EXPECT_EQ(count, 4);
  EXPECT_EQ("Bob bought 5 Apples. Thanks Bob!", s);
}

struct Cont {
  Cont() {}
  explicit Cont(abel::string_view src) : data(src) {}

  abel::string_view data;
};

template <int index>
abel::string_view get(const Cont& c) {
  auto splitter = abel:: string_split(c.data, ':');
  auto it = splitter.begin();
  for (int i = 0; i < index; ++i) ++it;

  return *it;
}

TEST(string_replace_all, VariableNumber) {
  std::string s;
  {
    std::vector<std::pair<std::string, std::string>> replacements;

    s = "abc";
    EXPECT_EQ(0, abel::string_replace_all(replacements, &s));
    EXPECT_EQ("abc", s);

    s = "abc";
    replacements.push_back({"a", "A"});
    EXPECT_EQ(1, abel::string_replace_all(replacements, &s));
    EXPECT_EQ("Abc", s);

    s = "abc";
    replacements.push_back({"b", "B"});
    EXPECT_EQ(2, abel::string_replace_all(replacements, &s));
    EXPECT_EQ("ABc", s);

    s = "abc";
    replacements.push_back({"d", "D"});
    EXPECT_EQ(2, abel::string_replace_all(replacements, &s));
    EXPECT_EQ("ABc", s);

    EXPECT_EQ("ABcABc", abel::string_replace_all("abcabc", replacements));
  }

  {
    std::map<const char*, const char*> replacements;
    replacements["aa"] = "x";
    replacements["a"] = "X";
    s = "aaa";
    EXPECT_EQ(2, abel::string_replace_all(replacements, &s));
    EXPECT_EQ("xX", s);

    EXPECT_EQ("xxX", abel::string_replace_all("aaaaa", replacements));
  }

  {
    std::list<std::pair<abel::string_view, abel::string_view>> replacements = {
        {"a", "x"}, {"b", "y"}, {"c", "z"}};

    std::string st = abel::string_replace_all("abc", replacements);
    EXPECT_EQ(st, "xyz");
  }

  {
    using X = std::tuple<abel::string_view, std::string, int>;
    std::vector<X> replacements(3);
    replacements[0] = X{"a", "x", 1};
    replacements[1] = X{"b", "y", 0};
    replacements[2] = X{"c", "z", -1};

    std::string st = abel::string_replace_all("abc", replacements);
    EXPECT_EQ(st, "xyz");
  }

  {
    std::vector<Cont> replacements(3);
    replacements[0] = Cont{"a:x"};
    replacements[1] = Cont{"b:y"};
    replacements[2] = Cont{"c:z"};

    std::string st = abel::string_replace_all("abc", replacements);
    EXPECT_EQ(st, "xyz");
  }
}

// Same as above, but using the in-place variant of abel::string_replace_all,
// that returns the # of replacements performed.
TEST(string_replace_all, Inplace) {
  std::string s;
  int reps;

  // Empty std::string.
  s = "";
  reps = abel::string_replace_all({{"", ""}, {"x", ""}, {"", "y"}, {"x", "y"}}, &s);
  EXPECT_EQ(reps, 0);
  EXPECT_EQ(s, "");

  // Empty substring.
  s = "abc";
  reps = abel::string_replace_all({{"", ""}, {"", "y"}, {"x", ""}}, &s);
  EXPECT_EQ(reps, 0);
  EXPECT_EQ(s, "abc");

  // Replace entire std::string, one char at a time
  s = "abc";
  reps = abel::string_replace_all({{"a", "x"}, {"b", "y"}, {"c", "z"}}, &s);
  EXPECT_EQ(reps, 3);
  EXPECT_EQ(s, "xyz");
  s = "zxy";
  reps = abel::string_replace_all({{"z", "x"}, {"x", "y"}, {"y", "z"}}, &s);
  EXPECT_EQ(reps, 3);
  EXPECT_EQ(s, "xyz");

  // Replace once at the start (longer matches take precedence)
  s = "abc";
  reps = abel::string_replace_all({{"a", "x"}, {"ab", "xy"}, {"abc", "xyz"}}, &s);
  EXPECT_EQ(reps, 1);
  EXPECT_EQ(s, "xyz");

  // Replace once in the middle.
  s = "Abc!";
  reps = abel::string_replace_all(
      {{"a", "x"}, {"ab", "xy"}, {"b", "y"}, {"bc", "yz"}, {"c", "z"}}, &s);
  EXPECT_EQ(reps, 1);
  EXPECT_EQ(s, "Ayz!");

  // Replace once at the end.
  s = "Abc!";
  reps = abel::string_replace_all(
      {{"a", "x"}, {"ab", "xy"}, {"b", "y"}, {"bc!", "yz?"}, {"c!", "z;"}}, &s);
  EXPECT_EQ(reps, 1);
  EXPECT_EQ(s, "Ayz?");

  // Replace multiple times with varying lengths of original/replacement.
  s = "ababa";
  reps = abel::string_replace_all({{"a", "xxx"}, {"b", "XXXX"}}, &s);
  EXPECT_EQ(reps, 5);
  EXPECT_EQ(s, "xxxXXXXxxxXXXXxxx");

  // Overlapping matches are replaced greedily.
  s = "aaa";
  reps = abel::string_replace_all({{"aa", "x"}, {"a", "X"}}, &s);
  EXPECT_EQ(reps, 2);
  EXPECT_EQ(s, "xX");
  s = "aaa";
  reps = abel::string_replace_all({{"a", "X"}, {"aa", "x"}}, &s);
  EXPECT_EQ(reps, 2);
  EXPECT_EQ(s, "xX");

  // Two well-known sentences
  s = "the quick brown fox jumped over the lazy dogs";
  reps = abel::string_replace_all(
      {
          {"brown", "box"},
          {"dogs", "jugs"},
          {"fox", "with"},
          {"jumped", "five"},
          {"over", "dozen"},
          {"quick", "my"},
          {"the", "pack"},
          {"the lazy", "liquor"},
      },
      &s);
  EXPECT_EQ(reps, 8);
  EXPECT_EQ(s, "pack my box with five dozen liquor jugs");
}
