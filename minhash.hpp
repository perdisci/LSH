//
//  minhash.hpp
//  
//
//  Created by Roberto Perdisci on 1/7/17.
//  Copyright © 2017 Roberto Perdisci. All rights reserved.
//

#ifndef minhash_hpp
#define minhash_hpp

#include <set>
#include <string>
#include <vector>
#include <random>

namespace rp {
    
class MinHash {
    
public:
    
    MinHash(const unsigned sig_len=100, const unsigned seed=0);
    
    std::vector<uint32_t> minhash_universal(const std::set<std::string>& s_set);
    std::vector<uint32_t> minhash_xor(const std::set<std::string>& s_set);
    
    static uint32_t shash32(const std::string s, const unsigned seed=0);
        

private:
    
    std::vector<uint64_t> a;
    std::vector<uint64_t> b;
    std::default_random_engine rand_eng;
    std::uniform_int_distribution<uint64_t> urandom_64;
    std::uniform_int_distribution<uint32_t> urandom_32;
    std::vector<uint32_t> random_uint32_universal_hash(const uint64_t x);
    
    unsigned sig_len;
    unsigned seed;

};
    
}

#endif /* minhash_hpp */
