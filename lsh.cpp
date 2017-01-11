//
//  lsh.cpp
//  LSH
//
//  Created by Roberto Perdisci on 1/7/17.
//  Copyright © 2017 Roberto Perdisci. All rights reserved.
//

#include "lsh.hpp"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>

namespace rp {
    
    using std::set;
    using std::string;
    using std::vector;

    
    LSH::LSH(const unsigned minhash_sig_len, const unsigned lsh_bands,
             const unsigned ngram_len, const unsigned seed) {
        this->minhash_sig_len = minhash_sig_len;
        this->lsh_bands = lsh_bands;
        this->ngram_len = ngram_len;
        this->seed = seed;
        
        mh = MinHash(minhash_sig_len,seed);
    }
    
    const CorpusNGramsMap& LSH::get_corpus_ngrams() {
        return ng_map;
    }
    
    const MinHashMap& LSH::get_minhash_signatures() {
        return mh_map;
    }
    
    const BandsHashMap& LSH::get_lsh_bands() {
        return bh_map;
    }
    
    const std::vector<CandidatePair>& LSH::get_candidate_pairs() {
        return cp_vect;
    }
    
    const std::vector<float>& LSH::get_minhash_similarities() {
        return mh_sim_vect;
    }
    
    const std::vector<float>& LSH::get_ngram_similarities() {
        return ng_sim_vect;
    }
    
    void LSH::minhash_corpus(const std::string dir_name, bool keep_ngrams, bool progress) {
        
        boost::filesystem::path dir_path(dir_name);
        std::cerr << "Dir = " << dir_path.string() << std::endl;
        
        if(is_directory(dir_path)) {
            for(auto& file_name : boost::make_iterator_range(boost::filesystem::directory_iterator(dir_path), {})) {
                if(progress) std::cerr << "Processing file : " << file_name.path().filename().string() << std::endl;
                boost::timer t; // for profiling
                NGramSet ngrams = load_ngrams_from_file(file_name.path().string(), ngram_len);
                VUint32 sig = mh.minhash_universal(ngrams);
                // VUint32 sig = mh.minhash_xor(ngrams);
                if(progress) std::cerr << "Ngrams size: " << ngrams.size() << std::endl;
                if(progress) std::cerr << "Time: " << t.elapsed() << std::endl;
                /*
                std::cout << "MH sig for: " << file_name.path().filename().string() << std::endl;
                for(auto s : sig)
                    std::cout << s << " ";
                std::cout << std::endl;
                */
                mh_map.insert(std::move(std::make_pair(file_name.path().filename().string(), sig)));
                if(keep_ngrams)
                    ng_map.insert(std::move(std::make_pair(file_name.path().filename().string(),std::move(ngrams))));
            }
        }
    }
    
    void LSH::compute_bands() {
        bh_map = std::move(compute_bands(mh_map,minhash_sig_len,lsh_bands));
    }
    
    BandsHashMap LSH::compute_bands(const MinHashMap& mh_map, const unsigned minhash_sig_len, const unsigned lsh_bands) {
        
        assert(minhash_sig_len % lsh_bands == 0);
        
        BandsHashMap local_bh_map;
        for(auto& i : mh_map) {
            std::string s;
            VUint32 bh;
            unsigned index = 1;
            // std::cout << "=========== " << i.first << std::endl;
            for(auto mh : i.second) {
                // s += "|" + std::to_string(mh) + "|"; // for debugging
                s += std::to_string(mh);
                if(index % (minhash_sig_len/lsh_bands) == 0) {
                    bh.push_back(MinHash::shash32(s));
                    // std::cout << "bh = " << s << " - " << MinHash::shash32(s) << std::endl;
                    s = std::string();
                }
                ++index;
            }
            // std::cout << "==== bh size = " << bh.size() << std::endl;
            local_bh_map.insert(std::make_pair(i.first,bh));
        }
        
        return std::move(local_bh_map);
    }
    
    
    void LSH::compute_candidate_pairs() {
        cp_vect = std::move(compute_candidate_pairs(bh_map));
    }
    
    
    std::vector<CandidatePair> LSH::compute_candidate_pairs(const BandsHashMap& bh_map) {

        std::vector<CandidatePair> cpv;
        
        std::vector<string> keys;
        for(auto b : bh_map)
            keys.push_back(b.first);
        
        for(int i=0; i<keys.size()-1; i++) {
            for(int j = i+1; j<keys.size(); j++) {
                for(int k=0; k<bh_map.at(keys[i]).size(); k++) {
                    if(bh_map.at(keys[i])[k] == bh_map.at(keys[j])[k]) {
                        cpv.push_back({keys[i],keys[j]});
                        break;
                    }
                }
            }
        }
        return std::move(cpv);
    }
    
    
    void LSH::compute_minhash_similarities() {
        mh_sim_vect = compute_minhash_similarities(mh_map, cp_vect);
    }
    
    
    std::vector<float> LSH::compute_minhash_similarities(const MinHashMap& mh_map, const std::vector<CandidatePair>& cpv) {
        
        std::vector<float> js_vect;
        
        for(CandidatePair cp : cpv) {
            std::set<std::uint32_t> v1(mh_map.at(cp.first).begin(),mh_map.at(cp.first).end());
            std::set<std::uint32_t> v2(mh_map.at(cp.second).begin(),mh_map.at(cp.second).end());
            
            unsigned common = 0;
            for(int i=0; i<mh_map.at(cp.first).size(); i++)
                if(mh_map.at(cp.first)[i] == mh_map.at(cp.second)[i])
                    common++;
            
            
            float js = float(common)/mh_map.at(cp.first).size(); // minhash-based Jaccard similarity
            js_vect.push_back(js);
        }
        
        return std::move(js_vect);
    }
    
    void LSH::compute_ngram_similarities() {
        ng_sim_vect = compute_ngram_similarities(ng_map, cp_vect);
    }
    
    std::vector<float> LSH::compute_ngram_similarities(const CorpusNGramsMap& ng_map, const std::vector<CandidatePair>& cpv) {
        
        std::vector<float> js_vect;
        
        for(CandidatePair cp : cpv) {
            std::set<std::string> v1 = ng_map.at(cp.first);
            std::set<std::string> v2 = ng_map.at(cp.second);
            
            std::vector<std::string> v_int;
            std::vector<std::string> v_uni;
            std::set_intersection(v1.begin(), v1.end(),
                                  v2.begin(), v2.end(),
                                  std::back_inserter(v_int));
            std::set_union(v1.begin(), v1.end(),
                           v2.begin(), v2.end(),
                           std::back_inserter(v_uni));
            
            float js = float(v_int.size())/v_uni.size(); // Jaccard similarity
            js_vect.push_back(js);
        }
        
        return std::move(js_vect);
    }
    
    
    float LSH::candidate_probability(unsigned minhash_sig_len, unsigned lsh_bands, float jaccard_sim) {
        unsigned r = minhash_sig_len/lsh_bands;
        unsigned b = lsh_bands;
        float s = jaccard_sim;
        return 1 - std::pow((1 - std::pow(s,r)),b);
    }
    
    
    
    set<string> LSH::load_ngrams_from_file(const string file_name, const unsigned ngram_len) {
        
        const unsigned n = ngram_len;
        set<string> ngrams_set;
        
        std::ifstream f(file_name);
        string line;
        string prev_line;
        
        
        vector<string> last_ngram;
        while(std::getline(f,line)) {
            
            boost::trim(line);
            if(line.length()==0)
                continue;
            
            if(n>1 && !last_ngram.empty()) {
                string last_ngram_str;
                for(int i=1; i<last_ngram.size(); i++)
                    last_ngram_str += last_ngram[i] + " ";
                line = last_ngram_str + line;
            }
            else if(!prev_line.empty())
                line = prev_line + " " + line;
            
            vector<string> tokens;
            boost::algorithm::split(tokens,line,boost::algorithm::is_space());
            
            if(tokens.size() >= n) {
                for(int i=0; i<tokens.size()-n+1; i++) {
                    string ngram;
                    for(int j=i; j<(i+n); j++)
                        ngram += tokens[j] + " ";
                    boost::trim(ngram);
                    ngrams_set.insert(ngram);
                }
                
                vector<string>::const_iterator last = tokens.end();
                vector<string>::const_iterator first = tokens.end() - n;
                last_ngram = vector<string>(first,last);
                prev_line = string();
            }
            else {
                prev_line = line;
                last_ngram = vector<string>();
            }
            
        }
        
        return std::move(ngrams_set);
    }
    
} // namespace rp
