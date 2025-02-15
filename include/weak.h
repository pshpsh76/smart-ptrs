#pragma once

#include <utility>
#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
        Increase();
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) : block_(other.block_), ptr_(other.ptr_) {
        Increase();
    }

    WeakPtr(WeakPtr&& other) : block_(other.block_), ptr_(other.ptr_) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other) : block_(other.block_), ptr_(other.ptr_) {
        Increase();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        Decrease();
        block_ = other.block_;
        ptr_ = other.ptr_;
        Increase();
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        Decrease();
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Decrease();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Decrease();
        block_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->GetStrongCounter();
    }

    bool Expired() const {
        if (block_ == nullptr) {
            return true;
        }
        return block_->GetStrongCounter() == 0;
    }

    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }

private:
    void Increase() {
        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }

    void Decrease() {
        if (block_ != nullptr) {
            block_->DecWeak();
            if (block_->GetCounter() == 0) {
                delete block_;
            }
        }
    }

    ControlBlock* block_ = nullptr;
    T* ptr_ = nullptr;
};
