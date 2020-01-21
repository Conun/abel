//

#include <abel/container/node_hash_set.h>

#include <test/container/unordered_set_constructor_test.h>
#include <test/container/unordered_set_lookup_test.h>
#include <test/container/unordered_set_members_test.h>
#include <test/container/unordered_set_modifiers_test.h>

namespace abel {

namespace container_internal {
namespace {
using ::abel::container_internal::hash_internal::Enum;
using ::abel::container_internal::hash_internal::EnumClass;
using ::testing::IsEmpty;
using ::testing::Pointee;
using ::testing::UnorderedElementsAre;

using SetTypes = ::testing::Types<
    node_hash_set<int, StatefulTestingHash, StatefulTestingEqual, Alloc<int>>,
    node_hash_set<std::string, StatefulTestingHash, StatefulTestingEqual,
                  Alloc<std::string>>,
    node_hash_set<Enum, StatefulTestingHash, StatefulTestingEqual, Alloc<Enum>>,
    node_hash_set<EnumClass, StatefulTestingHash, StatefulTestingEqual,
                  Alloc<EnumClass>>>;

INSTANTIATE_TYPED_TEST_SUITE_P(NodeHashSet, ConstructorTest, SetTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(NodeHashSet, LookupTest, SetTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(NodeHashSet, MembersTest, SetTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(NodeHashSet, ModifiersTest, SetTypes);

TEST(NodeHashSet, MoveableNotCopyableCompiles) {
  node_hash_set<std::unique_ptr<void*>> t;
  node_hash_set<std::unique_ptr<void*>> u;
  u = std::move(t);
}

TEST(NodeHashSet, MergeExtractInsert) {
  struct Hash {
    size_t operator()(const std::unique_ptr<int>& p) const { return *p; }
  };
  struct Eq {
    bool operator()(const std::unique_ptr<int>& a,
                    const std::unique_ptr<int>& b) const {
      return *a == *b;
    }
  };
  abel::node_hash_set<std::unique_ptr<int>, Hash, Eq> set1, set2;
  set1.insert(abel::make_unique<int>(7));
  set1.insert(abel::make_unique<int>(17));

  set2.insert(abel::make_unique<int>(7));
  set2.insert(abel::make_unique<int>(19));

  EXPECT_THAT(set1, UnorderedElementsAre(Pointee(7), Pointee(17)));
  EXPECT_THAT(set2, UnorderedElementsAre(Pointee(7), Pointee(19)));

  set1.merge(set2);

  EXPECT_THAT(set1, UnorderedElementsAre(Pointee(7), Pointee(17), Pointee(19)));
  EXPECT_THAT(set2, UnorderedElementsAre(Pointee(7)));

  auto node = set1.extract(abel::make_unique<int>(7));
  EXPECT_TRUE(node);
  EXPECT_THAT(node.value(), Pointee(7));
  EXPECT_THAT(set1, UnorderedElementsAre(Pointee(17), Pointee(19)));

  auto insert_result = set2.insert(std::move(node));
  EXPECT_FALSE(node);
  EXPECT_FALSE(insert_result.inserted);
  EXPECT_TRUE(insert_result.node);
  EXPECT_THAT(insert_result.node.value(), Pointee(7));
  EXPECT_EQ(**insert_result.position, 7);
  EXPECT_NE(insert_result.position->get(), insert_result.node.value().get());
  EXPECT_THAT(set2, UnorderedElementsAre(Pointee(7)));

  node = set1.extract(abel::make_unique<int>(17));
  EXPECT_TRUE(node);
  EXPECT_THAT(node.value(), Pointee(17));
  EXPECT_THAT(set1, UnorderedElementsAre(Pointee(19)));

  node.value() = abel::make_unique<int>(23);

  insert_result = set2.insert(std::move(node));
  EXPECT_FALSE(node);
  EXPECT_TRUE(insert_result.inserted);
  EXPECT_FALSE(insert_result.node);
  EXPECT_EQ(**insert_result.position, 23);
  EXPECT_THAT(set2, UnorderedElementsAre(Pointee(7), Pointee(23)));
}

bool IsEven(int k) { return k % 2 == 0; }

TEST(NodeHashSet, EraseIf) {
  // Erase all elements.
  {
    node_hash_set<int> s = {1, 2, 3, 4, 5};
    erase_if(s, [](int) { return true; });
    EXPECT_THAT(s, IsEmpty());
  }
  // Erase no elements.
  {
    node_hash_set<int> s = {1, 2, 3, 4, 5};
    erase_if(s, [](int) { return false; });
    EXPECT_THAT(s, UnorderedElementsAre(1, 2, 3, 4, 5));
  }
  // Erase specific elements.
  {
    node_hash_set<int> s = {1, 2, 3, 4, 5};
    erase_if(s, [](int k) { return k % 2 == 1; });
    EXPECT_THAT(s, UnorderedElementsAre(2, 4));
  }
  // Predicate is function reference.
  {
    node_hash_set<int> s = {1, 2, 3, 4, 5};
    erase_if(s, IsEven);
    EXPECT_THAT(s, UnorderedElementsAre(1, 3, 5));
  }
  // Predicate is function pointer.
  {
    node_hash_set<int> s = {1, 2, 3, 4, 5};
    erase_if(s, &IsEven);
    EXPECT_THAT(s, UnorderedElementsAre(1, 3, 5));
  }
}

}  // namespace
}  // namespace container_internal

}  // namespace abel
