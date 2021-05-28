#include <cstdint>
#include <cstddef>
#include <memory>

typedef struct hash_r {
    uint64_t r1 = 0, r2 = 0, mod = 65536;

    void r_1_block(std::unique_ptr<char[]>& buf, size_t k, size_t L) {
        r1 = 0;
        for (size_t  i = 0; i < L; ++i) {
            r1 += buf[i + k] % mod;
        } r1 %= mod;
    }

    void r_2_block(std::unique_ptr<char[]>& buf, size_t k, size_t L) {
        r2 = 0;
        for (size_t i = 0; i < L; ++i) {
            r2 += ((L-i)*buf[i+k]) % mod;
        } r2 %= mod;
    }

    uint64_t r_block(std::unique_ptr<char[]>& buf, size_t k, size_t L) {
        r_1_block(buf, k, L);
        r_2_block(buf, k, L);
        return (r1 + mod*r2);
    }

    void r_1(std::unique_ptr<char[]>& buf, size_t k, size_t L) {
        r1 = (r1 - buf[k - 1] + buf[k+L-1]) % mod;
    }
    void r_2(std::unique_ptr<char[]>& buf, size_t k, size_t L) {
        r2 = (r2 - L*buf[k - 1] + r1) % mod;
    }
    uint64_t r(std::unique_ptr<char[]>& buf, size_t k, size_t L) {
        r_1(buf, k, L);
        r_2(buf, k, L);
        return (r1+mod*r2);
    }
} hash_r;
