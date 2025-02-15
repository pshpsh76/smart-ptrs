#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <utility>

template <typename T>
struct DefaultDeleter {
    constexpr DefaultDeleter() noexcept = default;

    template <typename U>
    DefaultDeleter(const DefaultDeleter<U>&) {
    }

    template <typename U>
    DefaultDeleter(DefaultDeleter<U>&&) {
    }

    template <typename U>
    DefaultDeleter<T> operator=(DefaultDeleter<U>&&) {
        return *this;
    }

    void operator()(T* p) const {
        delete p;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* p) const {
        delete[] p;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) noexcept : pair_(ptr, std::forward<Deleter>(deleter)) {
    }

    template <class U, typename NewDeleter>
    UniquePtr(UniquePtr<U, NewDeleter>&& other) noexcept
        : pair_(other.Release(), std::forward<NewDeleter>(other.GetDeleter())) {
    }

    UniquePtr(UniquePtr& other) = delete;

    UniquePtr& operator=(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename U, typename NewDeleter>
    UniquePtr& operator=(UniquePtr<U, NewDeleter>&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<NewDeleter>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() noexcept {
        Clean();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ptr = GetPointer();
        GetPointer() = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) noexcept {
        if (ptr == GetPointer()) {
            return;
        }
        T* old_ptr = GetPointer();
        GetPointer() = ptr;
        GetDeleter()(old_ptr);
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(GetPointer(), other.GetPointer());
        std::swap(GetDeleter(), other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return GetPointer();
    }

    Deleter& GetDeleter() noexcept {
        return pair_.GetSecond();
    }

    const Deleter& GetDeleter() const noexcept {
        return pair_.GetSecond();
    }

    explicit operator bool() const noexcept {
        return GetPointer() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const noexcept {
        return *GetPointer();
    }

    T* operator->() const noexcept {
        return GetPointer();
    }

private:
    CompressedPair<T*, Deleter> pair_;
    T*& GetPointer() {
        return pair_.GetFirst();
    }

    T* GetPointer() const {
        return pair_.GetFirst();
    }

    void Clean() {
        if (GetPointer() != nullptr) {
            GetDeleter()(GetPointer());
            GetPointer() = nullptr;
        }
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) noexcept : pair_(ptr, std::forward<Deleter>(deleter)) {
    }

    template <class U, typename NewDeleter>
    UniquePtr(UniquePtr<U, NewDeleter>&& other) noexcept
        : pair_(other.Release(), std::forward<NewDeleter>(other.GetDeleter())) {
    }

    UniquePtr(UniquePtr& other) = delete;

    UniquePtr& operator=(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename U, typename NewDeleter>
    UniquePtr& operator=(UniquePtr<U, NewDeleter>&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<NewDeleter>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() noexcept {
        Clean();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ptr = GetPointer();
        GetPointer() = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) noexcept {
        if (ptr == GetPointer()) {
            return;
        }
        T* old_ptr = GetPointer();
        GetPointer() = ptr;
        GetDeleter()(old_ptr);
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(GetPointer(), other.GetPointer());
        std::swap(GetDeleter(), other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return GetPointer();
    }

    Deleter& GetDeleter() noexcept {
        return pair_.GetSecond();
    }

    const Deleter& GetDeleter() const noexcept {
        return pair_.GetSecond();
    }

    explicit operator bool() const noexcept {
        return GetPointer() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const noexcept {
        return *GetPointer();
    }

    typename std::add_lvalue_reference<T>::type operator[](size_t i) {
        return GetPointer()[i];
    }

    T* operator->() const noexcept {
        return GetPointer();
    }

private:
    CompressedPair<T*, Deleter> pair_;
    T*& GetPointer() {
        return pair_.GetFirst();
    }

    T* GetPointer() const {
        return pair_.GetFirst();
    }

    void Clean() {
        GetDeleter()(GetPointer());
        GetPointer() = nullptr;
    }
};
