#pragma once
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

#if defined(__clang__) || defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline)) inline
#else
#define FORCEINLINE __forceinline
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

template <class Type> class verifycxx : public verifycxx_header {
public:
    class modify_guard {
    public:
        explicit modify_guard(verifycxx& parent) :
            parent(parent) {
        }

        ~modify_guard() { parent.update_checksum(); }
        Type* operator->() { return parent.data; }
        Type& operator*() { return *parent.data; }
        Type& operator=(const Type& rhs) { return *parent.data = rhs; }

    private:
        verifycxx& parent;
    };

    template <typename... Args>
    explicit verifycxx(Args&&... args) : verifycxx_header(sizeof(Type)) {
        if constexpr (sizeof...(Args) == 1 && std::is_scalar_v<Type>) {
            data = new Type(std::forward<Args>(args)...);
        }
        else {
            data = new Type{ std::forward<Args>(args)... };
        }
        checksum = gen_checksum();
    }

    ~verifycxx() { delete data; }

    const Type* get() noexcept { return data; }
    modify_guard modify() noexcept { return modify_guard(*this); }
    FORCEINLINE bool verify() const noexcept { return checksum == gen_checksum(); }

    FORCEINLINE uint64_t get_checksum() const noexcept { return checksum; }

    operator bool() const noexcept { return verify(); }
    operator Type() const noexcept requires std::is_scalar_v<Type> {
        return *data;
    }
    operator const char*() const requires requires(Type t) { t.c_str(); } {
        return data->c_str();
    }
    template<typename T = Type>
    auto& operator[](size_t i) requires requires(T t) { t[i]; } {
        return (*data)[i];
    }

    auto begin() const noexcept { return data->begin(); }
    auto end() const noexcept { return data->end(); }

private:
    FORCEINLINE uint64_t gen_checksum() const {
        const uint16_t cookie = u.bits.cookie;
        uint64_t sum = 0;

        auto process = [&](const uint8_t* ptr, size_t size) {
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
            };

        process(reinterpret_cast<const uint8_t*>(this), sizeof(verifycxx_header));
        process(reinterpret_cast<const uint8_t*>(data), sizeof(Type));

        return sum;
    }
    void update_checksum() { checksum = gen_checksum(); }

    Type*    data{};
    uint64_t checksum;
};