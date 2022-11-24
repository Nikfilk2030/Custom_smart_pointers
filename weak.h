#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    template <class U>
    friend class SharedPtr;

public:
    T* ptr_;
    ControlBlockBase* block_;

protected:
    void IncrementBlockWeakCounter() {
        if (block_ == nullptr) {
            return;
        }
        block_->IncrementWeakCounter();
    }

    void DecrementBlockWeakCounter() {
        if (block_ == nullptr) {
            return;
        }
        if (block_->GetStrongCounter() == 0 && block_->GetWeakCounter() <= 1) {
            delete block_;
            return;
        }
        block_->DecrementWeakCounter();
    }

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : ptr_(nullptr), block_(nullptr){};

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        IncrementBlockWeakCounter();
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
        IncrementBlockWeakCounter();
    }

    WeakPtr(WeakPtr&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <typename U>
    WeakPtr(const SharedPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
        IncrementBlockWeakCounter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        DecrementBlockWeakCounter();
        ptr_ = other.ptr_;
        block_ = other.block_;
        IncrementBlockWeakCounter();
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        DecrementBlockWeakCounter();
        ptr_ = nullptr;
        block_ = nullptr;
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        DecrementBlockWeakCounter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecrementBlockWeakCounter();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
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
        return block_ == nullptr || block_->GetStrongCounter() == 0;
    }
    SharedPtr<T> Lock() const {
        if (UseCount() == 0) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};
