//
// -----------------------------------------------------------------------------
// File: hash.h
// -----------------------------------------------------------------------------
//
// This header file defines the abel `hash` library and the abel hashing
// framework. This framework consists of the following:
//
//   * The `abel::Hash` functor, which is used to invoke the hasher within the
//     abel hashing framework. `abel::Hash<T>` supports most basic types and
//     a number of abel types out of the box.
//   * `AbelHashValue`, an extension point that allows you to extend types to
//     support abel hashing without requiring you to define a hashing
//     algorithm.
//   * `HashState`, a type-erased class which implements the manipulation of the
//     hash state (H) itself, contains member functions `combine()` and
//     `combine_contiguous()`, which you can use to contribute to an existing
//     hash state when hashing your types.
//
// Unlike `std::hash` or other hashing frameworks, the abel hashing framework
// provides most of its utility by abstracting away the hash algorithm (and its
// implementation) entirely. Instead, a type invokes the abel hashing
// framework by simply combining its state with the state of known, hashable
// types. Hashing of that combined state is separately done by `abel::Hash`.
//
// One should assume that a hash algorithm is chosen randomly at the start of
// each process.  E.g., abel::Hash<int>()(9) in one process and
// abel::Hash<int>()(9) in another process are likely to differ.
//
// Example:
//
//   // Suppose we have a class `Circle` for which we want to add hashing:
//   class Circle {
//    public:
//     ...
//    private:
//     std::pair<int, int> center_;
//     int radius_;
//   };
//
//   // To add hashing support to `Circle`, we simply need to add a free
//   // (non-member) function `AbelHashValue()`, and return the combined hash
//   // state of the existing hash state and the class state. You can add such a
//   // free function using a friend declaration within the body of the class:
//   class Circle {
//    public:
//     ...
//     template <typename H>
//     friend H AbelHashValue(H h, const Circle& c) {
//       return H::combine(std::move(h), c.center_, c.radius_);
//     }
//     ...
//   };
//
// For more information, see Adding Type Support to `abel::Hash` below.
//
#ifndef ABEL_HASH_HASH_H_
#define ABEL_HASH_HASH_H_

#include <abel/hash/internal/hash.h>

namespace abel {


// -----------------------------------------------------------------------------
// `abel::Hash`
// -----------------------------------------------------------------------------
//
// `abel::Hash<T>` is a convenient general-purpose hash functor for any type `T`
// satisfying any of the following conditions (in order):
//
//  * T is an arithmetic or pointer type
//  * T defines an overload for `AbelHashValue(H, const T&)` for an arbitrary
//    hash state `H`.
//  - T defines a specialization of `HASH_NAMESPACE::hash<T>`
//  - T defines a specialization of `std::hash<T>`
//
// `abel::Hash` intrinsically supports the following types:
//
//   * All integral types (including bool)
//   * All enum types
//   * All floating-point types (although hashing them is discouraged)
//   * All pointer types, including nullptr_t
//   * std::pair<T1, T2>, if T1 and T2 are hashable
//   * std::tuple<Ts...>, if all the Ts... are hashable
//   * std::unique_ptr and std::shared_ptr
//   * All string-like types including:
//     * std::string
//     * std::string_view (as well as any instance of std::basic_string that
//       uses char and std::char_traits)
//  * All the standard sequence containers (provided the elements are hashable)
//  * All the standard ordered associative containers (provided the elements are
//    hashable)
//  * abel types such as the following:
//    * abel::string_view
//    * abel::InlinedVector
//    * abel::FixedArray
//    * abel::uint128
//    * abel::abel_time, abel::duration, and abel::time_zone
//
// Note: the list above is not meant to be exhaustive. Additional type support
// may be added, in which case the above list will be updated.
//
// -----------------------------------------------------------------------------
// abel::Hash Invocation Evaluation
// -----------------------------------------------------------------------------
//
// When invoked, `abel::Hash<T>` searches for supplied hash functions in the
// following order:
//
//   * Natively supported types out of the box (see above)
//   * Types for which an `AbelHashValue()` overload is provided (such as
//     user-defined types). See "Adding Type Support to `abel::Hash`" below.
//   * Types which define a `HASH_NAMESPACE::hash<T>` specialization (aka
//     `__gnu_cxx::hash<T>` for gcc/Clang or `stdext::hash<T>` for MSVC)
//   * Types which define a `std::hash<T>` specialization
//
// The fallback to legacy hash functions exists mainly for backwards
// compatibility. If you have a choice, prefer defining an `AbelHashValue`
// overload instead of specializing any legacy hash functors.
//
// -----------------------------------------------------------------------------
// The Hash State Concept, and using `HashState` for Type Erasure
// -----------------------------------------------------------------------------
//
// The `abel::Hash` framework relies on the Concept of a "hash state." Such a
// hash state is used in several places:
//
// * Within existing implementations of `abel::Hash<T>` to store the hashed
//   state of an object. Note that it is up to the implementation how it stores
//   such state. A hash table, for example, may mix the state to produce an
//   integer value; a testing framework may simply hold a vector of that state.
// * Within implementations of `AbelHashValue()` used to extend user-defined
//   types. (See "Adding Type Support to abel::Hash" below.)
// * Inside a `HashState`, providing type erasure for the concept of a hash
//   state, which you can use to extend the `abel::Hash` framework for types
//   that are otherwise difficult to extend using `AbelHashValue()`. (See the
//   `HashState` class below.)
//
// The "hash state" concept contains two member functions for mixing hash state:
//
// * `H::combine(state, values...)`
//
//   Combines an arbitrary number of values into a hash state, returning the
//   updated state. Note that the existing hash state is move-only and must be
//   passed by value.
//
//   Each of the value types T must be hashable by H.
//
//   NOTE:
//
//     state = H::combine(std::move(state), value1, value2, value3);
//
//   must be guaranteed to produce the same hash expansion as
//
//     state = H::combine(std::move(state), value1);
//     state = H::combine(std::move(state), value2);
//     state = H::combine(std::move(state), value3);
//
// * `H::combine_contiguous(state, data, size)`
//
//    Combines a contiguous array of `size` elements into a hash state,
//    returning the updated state. Note that the existing hash state is
//    move-only and must be passed by value.
//
//    NOTE:
//
//      state = H::combine_contiguous(std::move(state), data, size);
//
//    need NOT be guaranteed to produce the same hash expansion as a loop
//    (it may perform internal optimizations). If you need this guarantee, use a
//    loop instead.
//
// -----------------------------------------------------------------------------
// Adding Type Support to `abel::Hash`
// -----------------------------------------------------------------------------
//
// To add support for your user-defined type, add a proper `AbelHashValue()`
// overload as a free (non-member) function. The overload will take an
// existing hash state and should combine that state with state from the type.
//
// Example:
//
//   template <typename H>
//   H AbelHashValue(H state, const MyType& v) {
//     return H::combine(std::move(state), v.field1, ..., v.fieldN);
//   }
//
// where `(field1, ..., fieldN)` are the members you would use on your
// `operator==` to define equality.
//
// Notice that `AbelHashValue` is not a class member, but an ordinary function.
// An `AbelHashValue` overload for a type should only be declared in the same
// file and namespace as said type. The proper `AbelHashValue` implementation
// for a given type will be discovered via ADL.
//
// Note: unlike `std::hash', `abel::Hash` should never be specialized. It must
// only be extended by adding `AbelHashValue()` overloads.
//
template <typename T>
using Hash = abel::hash_internal::Hash<T>;

// HashState
//
// A type erased version of the hash state concept, for use in user-defined
// `AbelHashValue` implementations that can't use templates (such as PImpl
// classes, virtual functions, etc.). The type erasure adds overhead so it
// should be avoided unless necessary.
//
// Note: This wrapper will only erase calls to:
//     combine_contiguous(H, const unsigned char*, size_t)
//
// All other calls will be handled internally and will not invoke overloads
// provided by the wrapped class.
//
// Users of this class should still define a template `AbelHashValue` function,
// but can use `abel::HashState::Create(&state)` to erase the type of the hash
// state and dispatch to their private hashing logic.
//
// This state can be used like any other hash state. In particular, you can call
// `HashState::combine()` and `HashState::combine_contiguous()` on it.
//
// Example:
//
//   class Interface {
//    public:
//     template <typename H>
//     friend H AbelHashValue(H state, const Interface& value) {
//       state = H::combine(std::move(state), std::type_index(typeid(*this)));
//       value.HashValue(abel::HashState::Create(&state));
//       return state;
//     }
//    private:
//     virtual void HashValue(abel::HashState state) const = 0;
//   };
//
//   class Impl : Interface {
//    private:
//     void HashValue(abel::HashState state) const override {
//       abel::HashState::combine(std::move(state), v1_, v2_);
//     }
//     int v1_;
//     std::string v2_;
//   };
class HashState : public hash_internal::HashStateBase<HashState> {
 public:
  // HashState::Create()
  //
  // Create a new `HashState` instance that wraps `state`. All calls to
  // `combine()` and `combine_contiguous()` on the new instance will be
  // redirected to the original `state` object. The `state` object must outlive
  // the `HashState` instance.
  template <typename T>
  static HashState Create(T* state) {
    HashState s;
    s.Init(state);
    return s;
  }

  HashState(const HashState&) = delete;
  HashState& operator=(const HashState&) = delete;
  HashState(HashState&&) = default;
  HashState& operator=(HashState&&) = default;

  // HashState::combine()
  //
  // Combines an arbitrary number of values into a hash state, returning the
  // updated state.
  using HashState::HashStateBase::combine;

  // HashState::combine_contiguous()
  //
  // Combines a contiguous array of `size` elements into a hash state, returning
  // the updated state.
  static HashState combine_contiguous(HashState hash_state,
                                      const unsigned char* first, size_t size) {
    hash_state.combine_contiguous_(hash_state.state_, first, size);
    return hash_state;
  }
  using HashState::HashStateBase::combine_contiguous;

 private:
  HashState() = default;

  template <typename T>
  static void CombineContiguousImpl(void* p, const unsigned char* first,
                                    size_t size) {
    T& state = *static_cast<T*>(p);
    state = T::combine_contiguous(std::move(state), first, size);
  }

  template <typename T>
  void Init(T* state) {
    state_ = state;
    combine_contiguous_ = &CombineContiguousImpl<T>;
  }

  // Do not erase an already erased state.
  void Init(HashState* state) {
    state_ = state->state_;
    combine_contiguous_ = state->combine_contiguous_;
  }

  void* state_;
  void (*combine_contiguous_)(void*, const unsigned char*, size_t);
};


}  // namespace abel

#endif  // ABEL_HASH_HASH_H_
