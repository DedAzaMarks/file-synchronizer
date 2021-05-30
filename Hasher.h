#include <vector>

#include "Types.h"
#include "xxhash.c"

class Hasher {
   private:
    u64 r1 = 0, r2 = 0, mod = 65536, seed;

    void r_1_block(std::vector<char> buf, u64 k, u64 L) {
        r1 = 0;
        for (u64 i = 0; i < L; ++i) {
            r1 += buf[i + k] % mod;
        }
        r1 %= mod;
    }

    void r_2_block(std::vector<char> buf, u64 k, u64 L) {
        r2 = 0;
        for (u64 i = 0; i < L; ++i) {
            r2 += ((L - i) * buf[i + k]) % mod;
        }
        r2 %= mod;
    }

    void r_1(std::vector<char> buf, u64 k, u64 L) { r1 = (r1 - buf[k - 1] + buf[k + L - 1]) % mod; }

    void r_2(std::vector<char> buf, u64 k, u64 L) { r2 = (r2 - L * buf[k - 1] + r1) % mod; }

   public:
    Hasher(u64 _seed) : seed(_seed) {}

    u64 r_block(std::vector<char> buf, u64 k, u64 L) {
        r_1_block(buf, k, L);
        r_2_block(buf, k, L);
        return (r1 + mod * r2);
    }

    u64 r(std::vector<char> buf, u64 k, u64 L) {
        r_1(buf, k, L);
        r_2(buf, k, L);
        return (r1 + mod * r2);
    }

    u64 h(char* buf, u64 L) { return XXH64(buf, L, seed); }
};
