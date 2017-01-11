//
//  minhash.cpp
//  
//
//  Created by Roberto Perdisci on 1/7/17.
//  Copyright © 2017 Roberto Perdisci. All rights reserved.
//

#include "minhash.hpp"

#include <iostream>
#include <cstdint>
#include <cassert>
#include <boost/functional/hash.hpp>
#include "xxHash/xxhash.h"


namespace rp {

using std::string;
using std::vector;
using std::set;
    

MinHash::MinHash(const unsigned sig_len, const unsigned seed) {
    this->sig_len = sig_len;
    this->seed = seed;
    rand_eng.seed(seed);
}
    
    
vector<uint32_t> MinHash::random_uint32_universal_hash(const uint64_t x) {
    
    assert(x > 0 && x < UINT64_MAX);
    
    static const unsigned w = sizeof(uint64_t)*8;
    static const unsigned M = sizeof(uint32_t)*8;
    
    vector<uint32_t> hv;
    hv.push_back(static_cast<uint32_t>(x));
    
    for(int i=0; i<sig_len; i++) {
        if(a.size()<sig_len) {
            a.push_back(urandom_64(rand_eng));
            b.push_back(urandom_32(rand_eng));
        }
        
        uint32_t h = static_cast<uint32_t>((a[i]*x+b[i]) >> (w-M));
        hv.push_back(h);
    }
    
    return hv;
}
    


vector<uint32_t> MinHash::minhash_universal(const set<string>& s_set) {

    vector<uint32_t> mh_sig(sig_len,UINT32_MAX);
    for(string s : s_set) {
        uint64_t xxh = static_cast<uint64_t>(XXH64(s.data(),s.size(),seed));
        vector<uint32_t> rh = random_uint32_universal_hash(xxh);
        for(int i=0; i<sig_len; i++) {
            if(rh[i] < mh_sig[i])
                mh_sig[i] = rh[i];
        }
    }
    
    return std::move(mh_sig);
}
    
    
uint32_t MinHash::shash32(const string s, const unsigned seed) {
    return static_cast<uint32_t>(XXH32(s.data(),s.size(),seed));
}
    
    
vector<uint32_t> MinHash::minhash_xor(const set<string>& s_set) {
    
    static boost::hash<std::string> boost_hash_fn;
    
    static std::default_random_engine rand_eng(seed);
    static std::uniform_int_distribution<uint32_t> srandom;
        
    static vector<uint32_t> rn;
        
    vector<uint32_t> mh_sig(sig_len,std::numeric_limits<uint32_t>::max());
    for(string s : s_set) {
        std::size_t boosth = boost_hash_fn(s);
        for(int i=0; i<sig_len; i++) {
            if(rn.size()<sig_len)
                rn.push_back(srandom(rand_eng));
            uint32_t h = boosth^rn[i];
            if(h < mh_sig[i]) {
                mh_sig[i] = h;
            }
        }
    }
        
    return mh_sig;
}


} // namespace rp


