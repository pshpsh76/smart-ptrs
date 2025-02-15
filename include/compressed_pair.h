#pragma once

#include <iostream>
#include <type_traits>
#include <utility>

template <typename V>
inline constexpr bool kIsCompressedV = std::is_empty_v<V> && !std::is_final_v<V>;

template <typename T, bool IsFirst, bool IsCompressed = kIsCompressedV<T>>
class CompressedPairElement {
public:
    CompressedPairElement() = default;

    // CompressedPairElement(T other) {
    //     if constexpr (std::is_copy_assignable_v<T>) {
    //         value_ = other;
    //     }
    // }

    CompressedPairElement(const T& other) {
        if constexpr (std::is_copy_assignable_v<T>) {
            value_ = other;
        }
    }

    CompressedPairElement(T& other) {
        std::cout << "Link\n";
        if constexpr (std::is_copy_assignable_v<T>) {
            value_ = other;
        } else if constexpr (std::is_move_assignable_v<T>) {
            value_ = std::forward<T>(other);
        }
    }

    CompressedPairElement(T&& value) : value_(std::forward<T>(value)) {
    }

    T& Get() {
        return value_;
    }

    const T& Get() const {
        return value_;
    }

private:
    T value_;
};

template <typename T, bool IsFirst>
class CompressedPairElement<T, IsFirst, true> : T {
public:
    CompressedPairElement() = default;

    CompressedPairElement(const T& other) : T(other) {
    }

    CompressedPairElement(T&&) {
    }

    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : CompressedPairElement<F, true>, CompressedPairElement<S, false> {
    using FirstElement = CompressedPairElement<F, true>;
    using SecondElement = CompressedPairElement<S, false>;

public:
    CompressedPair() = default;

    CompressedPair(CompressedPair&& other)
        : FirstElement(std::forward<F>(other.GetFirst())),
          SecondElement(std::forward<S>(other.GetSecond())) {
    }

    CompressedPair& operator=(CompressedPair&& other) {
        FirstElement::Get() = std::forward<F>(other.GetFirst());
        SecondElement::Get() = std::forward<S>(other.GetSecond());
        return *this;
    }

    CompressedPair(const F& first, const S& second) : FirstElement(first), SecondElement(second) {
    }

    CompressedPair(F&& first, const S& second)
        : FirstElement(std::forward<F>(first)), SecondElement(second) {
    }

    CompressedPair(const F& first, S&& second)
        : FirstElement(first), SecondElement(std::forward<S>(second)) {
    }

    CompressedPair(F&& first, S&& second)
        : FirstElement(std::forward<F>(first)), SecondElement(std::forward<S>(second)) {
    }

    F& GetFirst() {
        return FirstElement::Get();
    }

    const F& GetFirst() const {
        return FirstElement::Get();
    }

    S& GetSecond() {
        return SecondElement::Get();
    }

    const S& GetSecond() const {
        return SecondElement::Get();
    };
};