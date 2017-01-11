//
//  lsh.hpp
//  LSH
//
//  Created by Roberto Perdisci on 1/7/17.
//  Copyright © 2017 Roberto Perdisci. All rights reserved.
//

#ifndef lsh_hpp
#define lsh_hpp

#include <set>
#include <map>
#include <string>
#include <vector>
#include "minhash.hpp"

namespace rp {
    
    typedef std::vector<uint32_t> VUint32;
    typedef std::map<std::string,VUint32> MinHashMap;
    typedef MinHashMap BandsHashMap;
    typedef std::pair<std::string,std::string> CandidatePair;
    typedef std::set<std::string> NGramSet;
    typedef std::map<std::string,NGramSet> CorpusNGramsMap;
    
    
    
    // LSH = Locality Sensitive Hashing
    // Efficient computation of (approximate) Jaccard Similarity between documents
    //
    // References:
    //   Mining Massive Datasets - Chapter 3
    //   http://infolab.stanford.edu/~ullman/mmds/book.pdf
    //
    class LSH {
        
    public:
        
        LSH(const unsigned minhash_sig_len=240, const unsigned lsh_bands=80,
            const unsigned ngram_len=1, const unsigned seed=0);
        
        // TODO: Generalize ngram tokenization
        // Currently, we only do a simple word-grams tokenization.
        static std::set<std::string> load_ngrams_from_file(const std::string file_name,
                                                           const unsigned ngram_len);
        
        // Computes MinHash signatures for each document in a directory
        void minhash_corpus(const std::string dir_name, bool keep_ngrams=false, bool progress=false);
        
        // Computes LSH bands from MinHash signatures
        void compute_bands();
        static BandsHashMap compute_bands(const MinHashMap& mh_map,
                                          const unsigned minhash_sig_len, const unsigned num_bands);
        
        // Compute pairs of documents that are likely to be similar
        void compute_candidate_pairs();
        static std::vector<CandidatePair> compute_candidate_pairs(const BandsHashMap& bh_map);
        
        // Compute approximate Jaccard Similarity between documant pairs using MinHash
        void compute_minhash_similarities();
        static std::vector<float> compute_minhash_similarities(const MinHashMap& mh_map,
                                                               const std::vector<CandidatePair>& cpv);
        
        // Compute approximate Jaccard Similarity between documant pairs using ngrams (no hashing)
        void compute_ngram_similarities();
        static std::vector<float> compute_ngram_similarities(const CorpusNGramsMap& mh_map,
                                                               const std::vector<CandidatePair>& cpv);
        
        // Available after calling minhash_corpus(...), with keep_ngrams=true
        const CorpusNGramsMap& get_corpus_ngrams();
        
        // Available after calling minhash_corpus(...)
        const MinHashMap& get_minhash_signatures();
        
        // Avilable after calling compute_candidate_pairs()
        const std::vector<CandidatePair>& get_candidate_pairs();
        
        // Avilable after calling compute_bands()
        const BandsHashMap& get_lsh_bands();
        
        // Avilable after calling compute_minhash_similarities()
        const std::vector<float>& get_minhash_similarities();
        
        // Avilable after calling compute_ngram_similarities()
        // if minhash_corpus(...) was called with with keep_ngrams=true
        const std::vector<float>& get_ngram_similarities();
        
        
        // utility function to help choose parameter values for LSH
        float candidate_probability(unsigned minhash_sig_len, unsigned lsh_bands, float jaccard_sim);
        
        
    private:
        
        unsigned minhash_sig_len;
        unsigned lsh_bands;
        unsigned ngram_len;
        unsigned seed;
        
        MinHash mh;
        MinHashMap mh_map;
        BandsHashMap bh_map;
        CorpusNGramsMap ng_map;
        std::vector<CandidatePair> cp_vect;
        std::vector<float> mh_sim_vect;
        std::vector<float> ng_sim_vect;
    };
    
}

#endif /* lhs_hpp */
