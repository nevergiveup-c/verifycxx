#pragma once
#include <windows.h>

#include <cstdint>
#include <utility>

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM_NEON)
#include <arm_neon.h>
#elif defined(__clang__) || defined(__GNUC__)
#include <immintrin.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#else
#include <immintrin.h>
#endif

#if defined(NON_CRT_PROJECT) || (defined(_KERNEL_MODE) || defined(_WIN64_DRIVER))
inline constexpr bool HAS_CRT = false;
namespace std {
    class shared_mutex;
    template <class T> class unique_lock;
    template <class T> class shared_lock;
}
#else
inline constexpr bool HAS_CRT = true;
#include <shared_mutex>
#endif

#ifndef FORCEINLINE
#if defined(__clang__) || defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define FORCEINLINE __forceinline
#endif
#endif

constexpr uint64_t splitmix64(uint64_t x) {
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

class atomic_lock {
public:
    explicit atomic_lock(volatile long& atm) : guard(atm) {
        while (_InterlockedExchange(&guard, 1)) {
            _mm_pause();
        }
    }
    ~atomic_lock() {
        _InterlockedExchange(&guard, 0);
    }
private:
    volatile long& guard;
};

#if defined(NON_CRT_PROJECT)
#define READ_LOCK(m)  atomic_lock __lock__(m);
#define WRITE_LOCK(m) atomic_lock __lock__(m);
#else
#define READ_LOCK(m)  std::shared_lock<std::shared_mutex> __lock__(m);
#define WRITE_LOCK(m) std::unique_lock<std::shared_mutex> __lock__(m);
#endif

struct verifycxx_header {
    static auto constexpr VDH_MAGIC = 0x5644482F; //VDH/
    explicit verifycxx_header(const uint16_t size) {
        u.bits.magic = VDH_MAGIC;
        u.bits.size = size;
        u.bits.cookie = static_cast<uint16_t>(reinterpret_cast<uint64_t>(this) & 0xFFFF);
    }
    union {
        struct {
            uint32_t magic : 32;
            uint16_t size : 16;
            uint16_t cookie : 16;
        } bits;
        uint64_t header{};
    } u;

    bool is_valid() const noexcept { return u.bits.magic == VDH_MAGIC; }
};

template <class Type> class verifycxx;

template <typename Type> class verifycxx_modify_guard {
    using lock_guard = std::conditional_t<HAS_CRT, std::unique_lock<std::shared_mutex>, atomic_lock>;
public:
    explicit verifycxx_modify_guard(verifycxx<Type>& parent) :
        parent(parent), lock(parent.mutex) {}
    ~verifycxx_modify_guard() { parent.update_checksum(); }

    Type* operator->() { return parent.data(); }
    Type& operator*() { return *parent.data(); }

    Type& operator=(const Type& rhs) { return *parent.data() = rhs; }

    Type& operator+=(const Type& rhs) { *parent.data() += rhs; return *parent.data(); }
    Type& operator-=(const Type& rhs) { *parent.data() -= rhs; return *parent.data(); }
    Type& operator*=(const Type& rhs) { *parent.data() *= rhs; return *parent.data(); }
    Type& operator/=(const Type& rhs) { *parent.data() /= rhs; return *parent.data(); }
    Type& operator%=(const Type& rhs) { *parent.data() %= rhs; return *parent.data(); }

    Type& operator&=(const Type& rhs) { *parent.data() &= rhs; return *parent.data(); }
    Type& operator|=(const Type& rhs) { *parent.data() |= rhs; return *parent.data(); }
    Type& operator^=(const Type& rhs) { *parent.data() ^= rhs; return *parent.data(); }
    Type& operator<<=(const Type& rhs) { *parent.data() <<= rhs; return *parent.data(); }
    Type& operator>>=(const Type& rhs) { *parent.data() >>= rhs; return *parent.data(); }

    Type& operator++() { ++(*parent.data()); return *parent.data(); }
    Type operator++(int) { return (*parent.data())++; }
    Type& operator--() { --(*parent.data()); return *parent.data(); }
    Type operator--(int) { return (*parent.data())--; }

    Type operator+(const Type& rhs) const { return *parent.data() + rhs; }
    Type operator-(const Type& rhs) const { return *parent.data() - rhs; }
    Type operator*(const Type& rhs) const { return *parent.data() * rhs; }
    Type operator/(const Type& rhs) const { return *parent.data() / rhs; }
    Type operator%(const Type& rhs) const { return *parent.data() % rhs; }

    template<typename T = Type> auto& operator[](size_t i) requires requires(T t) { t[i]; } {
        return (*parent.data())[i];
    }

private:
    verifycxx<Type>& parent;
    lock_guard lock;
};

template <class Type> class verifycxx : public verifycxx_header {
    friend class verifycxx_modify_guard<Type>;
    static constexpr bool use_soo = std::is_scalar_v<Type> &&
        sizeof (Type) < sizeof(uint64_t);
    using modify_guard = verifycxx_modify_guard<Type>;
    using storage_type = std::conditional_t<use_soo, Type, Type*>;
    using mutex_type = std::conditional_t<HAS_CRT, std::shared_mutex, volatile long>;
public:
    template <typename... Args> explicit verifycxx(Args&&... args) : verifycxx_header(sizeof(Type)) {
        if constexpr (sizeof...(Args) == 1 && std::is_scalar_v<Type>) {
            if constexpr (use_soo) {
                storage = []<typename T0>(T0&& first, auto&&...) {
                    return std::forward<T0>(first);
                }(std::forward<Args>(args)...);
            }
            else {
                storage = new Type(std::forward<Args>(args)...);
            }
        }
        else {
            storage = new Type{std::forward<Args>(args)...};
        }

        checksum = gen_checksum();
    }

    verifycxx(verifycxx&& other) noexcept : verifycxx_header(other.u.bits.size) {
        storage = std::exchange(other.storage, {});
        checksum = gen_checksum();
    }
    verifycxx& operator=(verifycxx&& other) noexcept {
        if (this != &other) {
            if constexpr (!use_soo) {
                delete storage;
            }
            storage = std::exchange(other.storage, {});
            u = other.u;
            checksum = gen_checksum();
        }
        return *this;
    }
    verifycxx(const verifycxx&) = delete;
    verifycxx& operator=(const verifycxx&) = delete;

    ~verifycxx() requires use_soo = default;
    ~verifycxx() requires (!use_soo) { delete storage; }

    FORCEINLINE const Type* get() const noexcept;
    FORCEINLINE modify_guard modify() noexcept { return modify_guard(*this); }

    FORCEINLINE bool verify() const noexcept;
    FORCEINLINE uint64_t get_checksum() const noexcept { return checksum; }

    FORCEINLINE operator bool() const noexcept { return verify(); }

    FORCEINLINE operator Type() const noexcept requires std::is_scalar_v<Type> { return *data(); }
    FORCEINLINE operator const char*() const requires requires(Type t) { t.c_str(); } { return data()->c_str(); }

    FORCEINLINE const Type& operator*() const noexcept { return *data(); }
    FORCEINLINE const Type* operator->() const noexcept { return data(); }

    template<typename T = Type> FORCEINLINE auto& operator[](size_t i) requires requires(T t) { t[i]; } { return (*data())[i]; }

    FORCEINLINE auto begin() const noexcept { return data()->begin(); }
    FORCEINLINE auto end() const noexcept { return data()->end(); }

private:
    Type* data() {
        if constexpr (use_soo) return &storage;
        else return storage;
    }
    const Type* data() const {
        if constexpr (use_soo) return &storage;
        else return storage;
    }

    FORCEINLINE uint64_t process_simd(const uint8_t* ptr, size_t size) const;
    FORCEINLINE uint64_t gen_checksum() const;
    FORCEINLINE void update_checksum() { checksum = gen_checksum(); }

    storage_type storage{};
    uint64_t checksum{};
    mutable mutex_type mutex{};
};

template<class Type> const Type* verifycxx<Type>::get() const noexcept {
    READ_LOCK(mutex)
    return data();
}

template<class Type> bool verifycxx<Type>::verify() const noexcept {
    READ_LOCK(mutex)
    return checksum == gen_checksum();
}

template<class Type> uint64_t verifycxx<Type>::process_simd(const uint8_t *ptr, size_t size) const {
    const uint16_t cookie = u.bits.cookie;
    uint64_t sum = 0;
    size_t i = 0;

#if defined(__aarch64__) || defined(_M_ARM64)
        uint64x2_t neon_sum = vdupq_n_u64(0);
        for (; i + 16 <= size; i += 16) {
            uint8x16_t data = vld1q_u8(ptr + i);
            uint16x8_t cookie_vec = vdupq_n_u16(cookie);
            neon_sum = vpadalq_u16(neon_sum, veorq_u16(vmovl_u8(vget_low_u8(data)), cookie_vec));
            neon_sum = vpadalq_u16(neon_sum, veorq_u16(vmovl_u8(vget_high_u8(data)), cookie_vec));
        }
        sum += vgetq_lane_u64(neon_sum, 0) + vgetq_lane_u64(neon_sum, 1);

#elif defined(__clang__) || defined(__GNUC__)
    __m128i xmm_sum = _mm_setzero_si128();
    __m128i xmm_cookie = _mm_set1_epi16(cookie);
    for (; i + 16 <= size; i += 16) {
        __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr + i));
        xmm_sum = _mm_add_epi64(xmm_sum, _mm_sad_epu8(_mm_xor_si128(data, xmm_cookie), _mm_setzero_si128()));
    }
    alignas(16) uint64_t temp[2];
    _mm_store_si128(reinterpret_cast<__m128i*>(temp), xmm_sum);
    sum += temp[0] + temp[1];

#else
        __m256i mm256_sum = _mm256_setzero_si256();
        __m256i mm256_cookie = _mm256_set1_epi16(cookie);
        for (; i + 32 <= size; i += 32) {
            __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr + i));
            mm256_sum = _mm256_add_epi64(mm256_sum, _mm256_sad_epu8(_mm256_xor_si256(data, mm256_cookie), _mm256_setzero_si256()));
        }
        alignas(32) uint64_t temp[4];
        _mm256_store_si256(reinterpret_cast<__m256i*>(temp), mm256_sum);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
#endif

    for (; i < size; ++i) sum += ptr[i] ^ cookie;

    return sum;
}

template<class Type> uint64_t verifycxx<Type>::gen_checksum() const {
    uint64_t sum = 0;
    sum += process_simd(reinterpret_cast<const uint8_t*>(this), sizeof(verifycxx_header));
    sum += process_simd(reinterpret_cast<const uint8_t*>(data()), sizeof(Type));
    return splitmix64(sum);
}
