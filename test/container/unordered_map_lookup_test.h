//

#ifndef ABEL_CONTAINER_INTERNAL_UNORDERED_MAP_LOOKUP_TEST_H_
#define ABEL_CONTAINER_INTERNAL_UNORDERED_MAP_LOOKUP_TEST_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <testing/hash_generator_testing.h>
#include <test/container/hash_policy_testing.h>

namespace abel {

namespace container_internal {

template<class UnordMap>
class LookupTest : public ::testing::Test { };

TYPED_TEST_SUITE_P(LookupTest);

TYPED_TEST_P(LookupTest, At) {
    using T = hash_internal::GeneratedType<TypeParam>;
    std::vector<T> values;
    std::generate_n(std::back_inserter(values), 10,
                    hash_internal::Generator<T>());
    TypeParam m(values.begin(), values.end());
    for (const auto &p : values) {
        const auto &val = m.at(p.first);
        EXPECT_EQ(p.second, val) << ::testing::PrintToString(p.first);
    }
}

TYPED_TEST_P(LookupTest, OperatorBracket) {
    using T = hash_internal::GeneratedType<TypeParam>;
    using V = typename TypeParam::mapped_type;
    std::vector<T> values;
    std::generate_n(std::back_inserter(values), 10,
                    hash_internal::Generator<T>());
    TypeParam m;
    for (const auto &p : values) {
        auto &val = m[p.first];
        EXPECT_EQ(V(), val) << ::testing::PrintToString(p.first);
        val = p.second;
    }
    for (const auto &p : values)
        EXPECT_EQ(p.second, m[p.first]) << ::testing::PrintToString(p.first);
}

TYPED_TEST_P(LookupTest, Count) {
    using T = hash_internal::GeneratedType<TypeParam>;
    std::vector<T> values;
    std::generate_n(std::back_inserter(values), 10,
                    hash_internal::Generator<T>());
    TypeParam m;
    for (const auto &p : values)
        EXPECT_EQ(0, m.count(p.first)) << ::testing::PrintToString(p.first);
    m.insert(values.begin(), values.end());
    for (const auto &p : values)
        EXPECT_EQ(1, m.count(p.first)) << ::testing::PrintToString(p.first);
}

TYPED_TEST_P(LookupTest, Find) {
    using std::get;
    using T = hash_internal::GeneratedType<TypeParam>;
    std::vector<T> values;
    std::generate_n(std::back_inserter(values), 10,
                    hash_internal::Generator<T>());
    TypeParam m;
    for (const auto &p : values)
        EXPECT_TRUE(m.end() == m.find(p.first))
                    << ::testing::PrintToString(p.first);
    m.insert(values.begin(), values.end());
    for (const auto &p : values) {
        auto it = m.find(p.first);
        EXPECT_TRUE(m.end() != it) << ::testing::PrintToString(p.first);
        EXPECT_EQ(p.second, get<1>(*it)) << ::testing::PrintToString(p.first);
    }
}

TYPED_TEST_P(LookupTest, EqualRange) {
    using std::get;
    using T = hash_internal::GeneratedType<TypeParam>;
    std::vector<T> values;
    std::generate_n(std::back_inserter(values), 10,
                    hash_internal::Generator<T>());
    TypeParam m;
    for (const auto &p : values) {
        auto r = m.equal_range(p.first);
        ASSERT_EQ(0, std::distance(r.first, r.second));
    }
    m.insert(values.begin(), values.end());
    for (const auto &p : values) {
        auto r = m.equal_range(p.first);
        ASSERT_EQ(1, std::distance(r.first, r.second));
        EXPECT_EQ(p.second, get<1>(*r.first)) << ::testing::PrintToString(p.first);
    }
}

REGISTER_TYPED_TEST_SUITE_P(LookupTest, At, OperatorBracket, Count, Find,
                            EqualRange);

}  // namespace container_internal

}  // namespace abel

#endif  // ABEL_CONTAINER_INTERNAL_UNORDERED_MAP_LOOKUP_TEST_H_
