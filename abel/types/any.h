//
//
// -----------------------------------------------------------------------------
// any.h
// -----------------------------------------------------------------------------
//
// This header file define the `abel::any` type for holding a type-safe value
// of any type. The 'abel::any` type is useful for providing a way to hold
// something that is, as yet, unspecified. Such unspecified types
// traditionally are passed between API boundaries until they are later cast to
// their "destination" types. To cast to such a destination type, use
// `abel::any_cast()`. Note that when casting an `abel::any`, you must cast it
// to an explicit type; implicit conversions will throw.
//
// Example:
//
//   auto a = abel::any(65);
//   abel::any_cast<int>(a);         // 65
//   abel::any_cast<char>(a);        // throws abel::bad_any_cast
//   abel::any_cast<std::string>(a); // throws abel::bad_any_cast
//
// `abel::any` is a C++11 compatible version of the C++17 `std::any` abstraction
// and is designed to be a drop-in replacement for code compliant with C++17.
//
// Traditionally, the behavior of casting to a temporary unspecified type has
// been accomplished with the `void *` paradigm, where the pointer was to some
// other unspecified type. `abel::any` provides an "owning" version of `void *`
// that avoids issues of pointer management.
//
// Note: just as in the case of `void *`, use of `abel::any` (and its C++17
// version `std::any`) is a code smell indicating that your API might not be
// constructed correctly. We have seen that most uses of `any` are unwarranted,
// and `abel::any`, like `std::any`, is difficult to use properly. Before using
// this abstraction, make sure that you should not instead be rewriting your
// code to be more specific.
//
// abel expects to release an `abel::variant` type shortly (a C++11 compatible
// version of the C++17 `std::variant), which is generally preferred for use
// over `abel::any`.
#ifndef ABEL_TYPES_ANY_H_
#define ABEL_TYPES_ANY_H_

#include <abel/base/profile.h>
#include <abel/utility/utility.h>

#ifdef ABEL_USES_STD_ANY

#include <any>  // IWYU pragma: export

namespace abel {

using std::any;
using std::any_cast;
using std::bad_any_cast;
using std::make_any;

}  // namespace abel

#else  // ABEL_USES_STD_ANY

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <abel/base/profile.h>
#include <abel/meta/type_traits.h>
#include <abel/types/bad_any_cast.h>

// NOTE: This macro is an implementation detail that is undefined at the bottom
// of the file. It is not intended for expansion directly from user code.
#ifdef ABEL_ANY_DETAIL_HAS_RTTI
#error ABEL_ANY_DETAIL_HAS_RTTI cannot be directly set
#elif !defined(__GNUC__) || defined(__GXX_RTTI)
#define ABEL_ANY_DETAIL_HAS_RTTI 1
#endif  // !defined(__GNUC__) || defined(__GXX_RTTI)

namespace abel {


namespace any_internal {

template <typename Type>
struct TypeTag {
  constexpr static char dummy_var = 0;
};

template <typename Type>
constexpr char TypeTag<Type>::dummy_var;

// FastTypeId<Type>() evaluates at compile/link-time to a unique pointer for the
// passed in type. These are meant to be good match for keys into maps or
// straight up comparisons.
template<typename Type>
constexpr ABEL_FORCE_INLINE const void* FastTypeId() {
  return &TypeTag<Type>::dummy_var;
}

}  // namespace any_internal

class any;

// swap()
//
// Swaps two `abel::any` values. Equivalent to `x.swap(y) where `x` and `y` are
// `abel::any` types.
void swap(any& x, any& y) noexcept;

// make_any()
//
// Constructs an `abel::any` of type `T` with the given arguments.
template <typename T, typename... Args>
any make_any(Args&&... args);

// Overload of `abel::make_any()` for constructing an `abel::any` type from an
// initializer list.
template <typename T, typename U, typename... Args>
any make_any(std::initializer_list<U> il, Args&&... args);

// any_cast()
//
// Statically casts the value of a `const abel::any` type to the given type.
// This function will throw `abel::bad_any_cast` if the stored value type of the
// `abel::any` does not match the cast.
//
// `any_cast()` can also be used to get a reference to the internal storage iff
// a reference type is passed as its `ValueType`:
//
// Example:
//
//   abel::any my_any = std::vector<int>();
//   abel::any_cast<std::vector<int>&>(my_any).push_back(42);
template <typename ValueType>
ValueType any_cast(const any& operand);

// Overload of `any_cast()` to statically cast the value of a non-const
// `abel::any` type to the given type. This function will throw
// `abel::bad_any_cast` if the stored value type of the `abel::any` does not
// match the cast.
template <typename ValueType>
ValueType any_cast(any& operand);  // NOLINT(runtime/references)

// Overload of `any_cast()` to statically cast the rvalue of an `abel::any`
// type. This function will throw `abel::bad_any_cast` if the stored value type
// of the `abel::any` does not match the cast.
template <typename ValueType>
ValueType any_cast(any&& operand);

// Overload of `any_cast()` to statically cast the value of a const pointer
// `abel::any` type to the given pointer type, or `nullptr` if the stored value
// type of the `abel::any` does not match the cast.
template <typename ValueType>
const ValueType* any_cast(const any* operand) noexcept;

// Overload of `any_cast()` to statically cast the value of a pointer
// `abel::any` type to the given pointer type, or `nullptr` if the stored value
// type of the `abel::any` does not match the cast.
template <typename ValueType>
ValueType* any_cast(any* operand) noexcept;

// -----------------------------------------------------------------------------
// abel::any
// -----------------------------------------------------------------------------
//
// An `abel::any` object provides the facility to either store an instance of a
// type, known as the "contained object", or no value. An `abel::any` is used to
// store values of types that are unknown at compile time. The `abel::any`
// object, when containing a value, must contain a value type; storing a
// reference type is neither desired nor supported.
//
// An `abel::any` can only store a type that is copy-constructible; move-only
// types are not allowed within an `any` object.
//
// Example:
//
//   auto a = abel::any(65);                 // Literal, copyable
//   auto b = abel::any(std::vector<int>()); // Default-initialized, copyable
//   std::unique_ptr<Foo> my_foo;
//   auto c = abel::any(std::move(my_foo));  // Error, not copy-constructible
//
// Note that `abel::any` makes use of decayed types (`abel::decay_t` in this
// context) to remove const-volatile qualifiers (known as "cv qualifiers"),
// decay functions to function pointers, etc. We essentially "decay" a given
// type into its essential type.
//
// `abel::any` makes use of decayed types when determining the basic type `T` of
// the value to store in the any's contained object. In the documentation below,
// we explicitly denote this by using the phrase "a decayed type of `T`".
//
// Example:
//
//   const int a = 4;
//   abel::any foo(a);  // Decay ensures we store an "int", not a "const int&".
//
//   void my_function() {}
//   abel::any bar(my_function);  // Decay ensures we store a function pointer.
//
// `abel::any` is a C++11 compatible version of the C++17 `std::any` abstraction
// and is designed to be a drop-in replacement for code compliant with C++17.
class any {
 private:
  template <typename T>
  struct IsInPlaceType;

 public:
  // Constructors

  // Constructs an empty `abel::any` object (`any::has_value()` will return
  // `false`).
  constexpr any() noexcept;

  // Copy constructs an `abel::any` object with a "contained object" of the
  // passed type of `other` (or an empty `abel::any` if `other.has_value()` is
  // `false`.
  any(const any& other)
      : obj_(other.has_value() ? other.obj_->Clone()
                               : std::unique_ptr<ObjInterface>()) {}

  // Move constructs an `abel::any` object with a "contained object" of the
  // passed type of `other` (or an empty `abel::any` if `other.has_value()` is
  // `false`).
  any(any&& other) noexcept = default;

  // Constructs an `abel::any` object with a "contained object" of the decayed
  // type of `T`, which is initialized via `std::forward<T>(value)`.
  //
  // This constructor will not participate in overload resolution if the
  // decayed type of `T` is not copy-constructible.
  template <
      typename T, typename VT = abel::decay_t<T>,
      abel::enable_if_t<!abel::disjunction<
          std::is_same<any, VT>, IsInPlaceType<VT>,
          abel::negation<std::is_copy_constructible<VT> > >::value>* = nullptr>
  any(T&& value) : obj_(new Obj<VT>(in_place, std::forward<T>(value))) {}

  // Constructs an `abel::any` object with a "contained object" of the decayed
  // type of `T`, which is initialized via `std::forward<T>(value)`.
  template <typename T, typename... Args, typename VT = abel::decay_t<T>,
            abel::enable_if_t<abel::conjunction<
                std::is_copy_constructible<VT>,
                std::is_constructible<VT, Args...>>::value>* = nullptr>
  explicit any(in_place_type_t<T> /*tag*/, Args&&... args)
      : obj_(new Obj<VT>(in_place, std::forward<Args>(args)...)) {}

  // Constructs an `abel::any` object with a "contained object" of the passed
  // type `VT` as a decayed type of `T`. `VT` is initialized as if
  // direct-non-list-initializing an object of type `VT` with the arguments
  // `initializer_list, std::forward<Args>(args)...`.
  template <
      typename T, typename U, typename... Args, typename VT = abel::decay_t<T>,
      abel::enable_if_t<
          abel::conjunction<std::is_copy_constructible<VT>,
                            std::is_constructible<VT, std::initializer_list<U>&,
                                                  Args...>>::value>* = nullptr>
  explicit any(in_place_type_t<T> /*tag*/, std::initializer_list<U> ilist,
               Args&&... args)
      : obj_(new Obj<VT>(in_place, ilist, std::forward<Args>(args)...)) {}

  // Assignment operators

  // Copy assigns an `abel::any` object with a "contained object" of the
  // passed type.
  any& operator=(const any& rhs) {
    any(rhs).swap(*this);
    return *this;
  }

  // Move assigns an `abel::any` object with a "contained object" of the
  // passed type. `rhs` is left in a valid but otherwise unspecified state.
  any& operator=(any&& rhs) noexcept {
    any(std::move(rhs)).swap(*this);
    return *this;
  }

  // Assigns an `abel::any` object with a "contained object" of the passed type.
  template <typename T, typename VT = abel::decay_t<T>,
            abel::enable_if_t<abel::conjunction<
                abel::negation<std::is_same<VT, any>>,
                std::is_copy_constructible<VT>>::value>* = nullptr>
  any& operator=(T&& rhs) {
    any tmp(in_place_type_t<VT>(), std::forward<T>(rhs));
    tmp.swap(*this);
    return *this;
  }

  // Modifiers

  // any::emplace()
  //
  // Emplaces a value within an `abel::any` object by calling `any::reset()`,
  // initializing the contained value as if direct-non-list-initializing an
  // object of type `VT` with the arguments `std::forward<Args>(args)...`, and
  // returning a reference to the new contained value.
  //
  // Note: If an exception is thrown during the call to `VT`'s constructor,
  // `*this` does not contain a value, and any previously contained value has
  // been destroyed.
  template <
      typename T, typename... Args, typename VT = abel::decay_t<T>,
      abel::enable_if_t<std::is_copy_constructible<VT>::value &&
                        std::is_constructible<VT, Args...>::value>* = nullptr>
  VT& emplace(Args&&... args) {
    reset();  // NOTE: reset() is required here even in the world of exceptions.
    Obj<VT>* const object_ptr =
        new Obj<VT>(in_place, std::forward<Args>(args)...);
    obj_ = std::unique_ptr<ObjInterface>(object_ptr);
    return object_ptr->value;
  }

  // Overload of `any::emplace()` to emplace a value within an `abel::any`
  // object by calling `any::reset()`, initializing the contained value as if
  // direct-non-list-initializing an object of type `VT` with the arguments
  // `initializer_list, std::forward<Args>(args)...`, and returning a reference
  // to the new contained value.
  //
  // Note: If an exception is thrown during the call to `VT`'s constructor,
  // `*this` does not contain a value, and any previously contained value has
  // been destroyed. The function shall not participate in overload resolution
  // unless `is_copy_constructible_v<VT>` is `true` and
  // `is_constructible_v<VT, initializer_list<U>&, Args...>` is `true`.
  template <
      typename T, typename U, typename... Args, typename VT = abel::decay_t<T>,
      abel::enable_if_t<std::is_copy_constructible<VT>::value &&
                        std::is_constructible<VT, std::initializer_list<U>&,
                                              Args...>::value>* = nullptr>
  VT& emplace(std::initializer_list<U> ilist, Args&&... args) {
    reset();  // NOTE: reset() is required here even in the world of exceptions.
    Obj<VT>* const object_ptr =
        new Obj<VT>(in_place, ilist, std::forward<Args>(args)...);
    obj_ = std::unique_ptr<ObjInterface>(object_ptr);
    return object_ptr->value;
  }

  // any::reset()
  //
  // Resets the state of the `abel::any` object, destroying the contained object
  // if present.
  void reset() noexcept { obj_ = nullptr; }

  // any::swap()
  //
  // Swaps the passed value and the value of this `abel::any` object.
  void swap(any& other) noexcept { obj_.swap(other.obj_); }

  // Observers

  // any::has_value()
  //
  // Returns `true` if the `any` object has a contained value, otherwise
  // returns `false`.
  bool has_value() const noexcept { return obj_ != nullptr; }

#if ABEL_ANY_DETAIL_HAS_RTTI
  // Returns: typeid(T) if *this has a contained object of type T, otherwise
  // typeid(void).
  const std::type_info& type() const noexcept {
    if (has_value()) {
      return obj_->Type();
    }

    return typeid(void);
  }
#endif  // ABEL_ANY_DETAIL_HAS_RTTI

 private:
  // Tagged type-erased abstraction for holding a cloneable object.
  class ObjInterface {
   public:
    virtual ~ObjInterface() = default;
    virtual std::unique_ptr<ObjInterface> Clone() const = 0;
    virtual const void* ObjTypeId() const noexcept = 0;
#if ABEL_ANY_DETAIL_HAS_RTTI
    virtual const std::type_info& Type() const noexcept = 0;
#endif  // ABEL_ANY_DETAIL_HAS_RTTI
  };

  // Hold a value of some queryable type, with an ability to Clone it.
  template <typename T>
  class Obj : public ObjInterface {
   public:
    template <typename... Args>
    explicit Obj(in_place_t /*tag*/, Args&&... args)
        : value(std::forward<Args>(args)...) {}

    std::unique_ptr<ObjInterface> Clone() const final {
      return std::unique_ptr<ObjInterface>(new Obj(in_place, value));
    }

    const void* ObjTypeId() const noexcept final { return IdForType<T>(); }

#if ABEL_ANY_DETAIL_HAS_RTTI
    const std::type_info& Type() const noexcept final { return typeid(T); }
#endif  // ABEL_ANY_DETAIL_HAS_RTTI

    T value;
  };

  std::unique_ptr<ObjInterface> CloneObj() const {
    if (!obj_) return nullptr;
    return obj_->Clone();
  }

  template <typename T>
  constexpr static const void* IdForType() {
    // Note: This type dance is to make the behavior consistent with typeid.
    using NormalizedType =
        typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    return any_internal::FastTypeId<NormalizedType>();
  }

  const void* GetObjTypeId() const {
    return obj_ ? obj_->ObjTypeId() : any_internal::FastTypeId<void>();
  }

  // `abel::any` nonmember functions //

  // Description at the declaration site (top of file).
  template <typename ValueType>
  friend ValueType any_cast(const any& operand);

  // Description at the declaration site (top of file).
  template <typename ValueType>
  friend ValueType any_cast(any& operand);  // NOLINT(runtime/references)

  // Description at the declaration site (top of file).
  template <typename T>
  friend const T* any_cast(const any* operand) noexcept;

  // Description at the declaration site (top of file).
  template <typename T>
  friend T* any_cast(any* operand) noexcept;

  std::unique_ptr<ObjInterface> obj_;
};

// -----------------------------------------------------------------------------
// Implementation Details
// -----------------------------------------------------------------------------

constexpr any::any() noexcept = default;

template <typename T>
struct any::IsInPlaceType : std::false_type {};

template <typename T>
struct any::IsInPlaceType<in_place_type_t<T>> : std::true_type {};

ABEL_FORCE_INLINE void swap(any& x, any& y) noexcept { x.swap(y); }

// Description at the declaration site (top of file).
template <typename T, typename... Args>
any make_any(Args&&... args) {
  return any(in_place_type_t<T>(), std::forward<Args>(args)...);
}

// Description at the declaration site (top of file).
template <typename T, typename U, typename... Args>
any make_any(std::initializer_list<U> il, Args&&... args) {
  return any(in_place_type_t<T>(), il, std::forward<Args>(args)...);
}

// Description at the declaration site (top of file).
template <typename ValueType>
ValueType any_cast(const any& operand) {
  using U = typename std::remove_cv<
      typename std::remove_reference<ValueType>::type>::type;
  static_assert(std::is_constructible<ValueType, const U&>::value,
                "Invalid ValueType");
  auto* const result = (any_cast<U>)(&operand);
  if (result == nullptr) {
    any_internal::ThrowBadAnyCast();
  }
  return static_cast<ValueType>(*result);
}

// Description at the declaration site (top of file).
template <typename ValueType>
ValueType any_cast(any& operand) {  // NOLINT(runtime/references)
  using U = typename std::remove_cv<
      typename std::remove_reference<ValueType>::type>::type;
  static_assert(std::is_constructible<ValueType, U&>::value,
                "Invalid ValueType");
  auto* result = (any_cast<U>)(&operand);
  if (result == nullptr) {
    any_internal::ThrowBadAnyCast();
  }
  return static_cast<ValueType>(*result);
}

// Description at the declaration site (top of file).
template <typename ValueType>
ValueType any_cast(any&& operand) {
  using U = typename std::remove_cv<
      typename std::remove_reference<ValueType>::type>::type;
  static_assert(std::is_constructible<ValueType, U>::value,
                "Invalid ValueType");
  return static_cast<ValueType>(std::move((any_cast<U&>)(operand)));
}

// Description at the declaration site (top of file).
template <typename T>
const T* any_cast(const any* operand) noexcept {
  using U =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  return operand && operand->GetObjTypeId() == any::IdForType<U>()
             ? std::addressof(
                   static_cast<const any::Obj<U>*>(operand->obj_.get())->value)
             : nullptr;
}

// Description at the declaration site (top of file).
template <typename T>
T* any_cast(any* operand) noexcept {
  using U =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  return operand && operand->GetObjTypeId() == any::IdForType<U>()
             ? std::addressof(
                   static_cast<any::Obj<U>*>(operand->obj_.get())->value)
             : nullptr;
}


}  // namespace abel

#undef ABEL_ANY_DETAIL_HAS_RTTI

#endif  // ABEL_USES_STD_ANY

#endif  // ABEL_TYPES_ANY_H_
