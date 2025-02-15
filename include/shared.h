#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <memory>
#include <type_traits>
#include <utility>

class ESFTBase {};

template <typename T>
class EnableSharedFromThis;

class ControlBlock {
private:
    size_t strong_counter_ = 1;
    size_t weak_counter_ = 0;

public:
    void IncStrong() {
        ++strong_counter_;
    }

    void DecStrong() {
        --strong_counter_;
    }

    size_t GetStrongCounter() const {
        return strong_counter_;
    }

    void IncWeak() {
        ++weak_counter_;
    }

    void DecWeak() {
        --weak_counter_;
    }

    size_t GetWeakCounter() const {
        return weak_counter_;
    }

    size_t GetCounter() const {
        return GetStrongCounter() + GetWeakCounter();
    }

    virtual ~ControlBlock() = default;
    virtual void DeleteObject() {
    }
};

template <typename T>
class ControlBlockWithPointer : public ControlBlock {
public:
    ControlBlockWithPointer(T* ptr) : ptr_(ptr) {
    }

    ~ControlBlockWithPointer() override {
    }

    void DeleteObject() override {
        delete ptr_;
    }

private:
    T* ptr_;
};

template <typename T>
class ControlBlockWithObj : public ControlBlock {
public:
    ~ControlBlockWithObj() override = default;

    T* GetPointer() {
        return reinterpret_cast<T*>(&storage_);
    }

    template <typename... Args>
    ControlBlockWithObj(Args&&... args) {
        new (&storage_) T(std::forward<Args>(args)...);
    }

    void DeleteObject() override {
        std::destroy_at(GetPointer());
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
    template <typename Y>
    friend class EnableSharedFromThis;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
    }

    explicit SharedPtr(T* ptr) : block_(new ControlBlockWithPointer<T>(ptr)), ptr_(ptr) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            ptr->self_ = *this;
        }
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr) : block_(new ControlBlockWithPointer<Y>(ptr)), ptr_(ptr) {
        if constexpr (std::is_convertible_v<Y*, ESFTBase*>) {
            ptr->self_ = *this;
        }
    }

    SharedPtr(const SharedPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            ptr_->self_ = *this;
        }
        Increase();
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : block_(other.block_), ptr_(other.ptr_) {
        if constexpr (std::is_convertible_v<Y*, ESFTBase*>) {
            ptr_->self_ = *this;
        }
        Increase();
    }

    SharedPtr(SharedPtr<T>&& other) : block_(other.block_), ptr_(other.ptr_) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            ptr_->self_ = *this;
        }
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) : block_(other.block_), ptr_(other.ptr_) {
        if constexpr (std::is_convertible_v<Y*, ESFTBase*>) {
            ptr_->self_ = *this;
        }
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    SharedPtr(ControlBlockWithObj<T>* block) : block_(block), ptr_(block->GetPointer()) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            ptr_->self_ = *this;
        }
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : block_(other.block_), ptr_(ptr) {
        Increase();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (other.Expired()) {
            throw BadWeakPtr{};
        }
        Increase();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        Decrease();
        block_ = other.block_;
        ptr_ = other.ptr_;
        Increase();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        Decrease();
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Decrease();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Decrease();
        block_ = nullptr;
        ptr_ = nullptr;
    }

    void Reset(T* ptr) {
        Decrease();
        block_ = new ControlBlockWithPointer<T>(ptr);
        ptr_ = ptr;
    }

    template <typename Y>
    void Reset(Y* ptr) {
        Decrease();
        block_ = new ControlBlockWithPointer<Y>(ptr);
        ptr_ = ptr;
    }

    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    size_t UseCount() const {
        if (ptr_ != nullptr) {
            return block_->GetStrongCounter();
        }
        return 0;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    void Decrease() {
        if (ptr_ != nullptr) {
            block_->DecStrong();
            if (UseCount() == 0) {
                size_t cnt = block_->GetWeakCounter();
                block_->DeleteObject();
                if (cnt == 0) {
                    if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
                        if (!ptr_->self_.Expired()) {
                            ptr_->self_.Reset();
                        }
                        block_ = nullptr;
                    }
                    delete block_;
                    block_ = nullptr;
                }
                ptr_ = nullptr;
            }
        }
    }

    void Increase() {
        if (ptr_ != nullptr) {
            block_->IncStrong();
        }
    }
    ControlBlock* block_ = nullptr;
    T* ptr_ = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new ControlBlockWithObj<T>(std::forward<Args>(args)...));
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public ESFTBase {
private:
    WeakPtr<T> self_;

public:
    template <typename Y>
    friend class SharedPtr;
    SharedPtr<T> SharedFromThis() {
        return self_.Lock();
    }
    SharedPtr<const T> SharedFromThis() const {
        return self_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return self_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return self_;
    }
};
