#pragma once

#include <exception>

// trying to make proper control block:
class ControlBlockBase {
public:
    int GetStrongCounter() const {
        return strong_counter;
    }

    int GetWeakCounter() const {
        return weak_counter;
    }

    void IncrementStrongCounter() {
        ++strong_counter;
    }

    void DecrementStrongCounter() {
        --strong_counter;
    }

    void IncrementWeakCounter() {
        ++weak_counter;
    }

    void DecrementWeakCounter() {
        --weak_counter;
    }

    virtual ~ControlBlockBase() = default;

    virtual void DeletePointer() = 0;

public:
    int strong_counter = 0;
    int weak_counter = 0;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    ControlBlockPointer(T* ptr) : ptr_(ptr) {
        strong_counter = 1;
        weak_counter = 0;
    }

    ~ControlBlockPointer() override {
        //        std::cout << "~ControlBlockPointer\n";
    }  // does nothing

    void DeletePointer() override {
        delete ptr_;
    }

    T* GetPointer() const {
        return ptr_;
    }

protected:
    T* ptr_;
};

template <typename T>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args&&... args) {
        strong_counter = 1;
        new (&storage_) T(std::forward<Args>(args)...);
    }

    T* GetPointer() {
        return reinterpret_cast<T*>(&storage_);
    }

    ~ControlBlockHolder() override {
        //        std::cout << "~ControlBlockHolder\n";
    }  // does nothing

    void DeletePointer() override {
        GetPointer()->~T();
    }

protected:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
