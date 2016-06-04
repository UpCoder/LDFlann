//
//  lsh_table.h
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#ifndef lsh_table_h
#define lsh_table_h
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits.h>
// TODO as soon as we use C++0x, use the code in USE_UNORDERED_MAP
#if USE_UNORDERED_MAP
#include <unordered_map>
#else
#include <map>
#endif
#include <math.h>
#include <stddef.h>

#include "dynamic_bitset.h"
#include "matrix.h"
using namespace std;
namespace LDFlann
{
    
    namespace lsh
    {
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        /** What is stored in an LSH bucket
         */
        typedef uint32_t FeatureIndex;
        /** The id from which we can get a bucket back in an LSH table
         */
        typedef unsigned int BucketKey;
        
        /** A bucket in an LSH table
         */
        typedef std::vector<FeatureIndex> Bucket;
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        /** POD for stats about an LSH table
         */
        struct LshStats
        {
            std::vector<unsigned int> bucket_sizes_;
            size_t n_buckets_;
            size_t bucket_size_mean_;
            size_t bucket_size_median_;
            size_t bucket_size_min_;
            size_t bucket_size_max_;
            size_t bucket_size_std_dev;
            /** Each contained vector contains three value: beginning/end for interval, number of elements in the bin
             */
            std::vector<std::vector<unsigned int> > size_histogram_;
        };
        
        /** Overload the << operator for LshStats
         * @param out the streams
         * @param stats the stats to display
         * @return the streams
         */
        inline std::ostream& operator <<(std::ostream& out, const LshStats& stats)
        {
            size_t w = 20;
            out << "Lsh Table Stats:\n" << std::setw(w) << std::setiosflags(std::ios::right) << "N buckets : "
            << stats.n_buckets_ << "\n" << std::setw(w) << std::setiosflags(std::ios::right) << "mean size : "
            << std::setiosflags(std::ios::left) << stats.bucket_size_mean_ << "\n" << std::setw(w)
            << std::setiosflags(std::ios::right) << "median size : " << stats.bucket_size_median_ << "\n" << std::setw(w)
            << std::setiosflags(std::ios::right) << "min size : " << std::setiosflags(std::ios::left)
            << stats.bucket_size_min_ << "\n" << std::setw(w) << std::setiosflags(std::ios::right) << "max size : "
            << std::setiosflags(std::ios::left) << stats.bucket_size_max_;
            
            // Display the histogram
            out << std::endl << std::setw(w) << std::setiosflags(std::ios::right) << "histogram : "
            << std::setiosflags(std::ios::left);
            for (std::vector<std::vector<unsigned int> >::const_iterator iterator = stats.size_histogram_.begin(), end =
                 stats.size_histogram_.end(); iterator != end; ++iterator) out << (*iterator)[0] << "-" << (*iterator)[1] << ": " << (*iterator)[2] << ",  ";
            
            return out;
        }
        
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        /** Lsh hash table. As its key is a sub-feature, and as usually
         * the size of it is pretty small, we keep it as a continuous memory array.
         * The value is an index in the corpus of features (we keep it as an unsigned
         * int for pure memory reasons, it could be a size_t)
         */
        template<typename ElementType>
        class LshTable
        {
        public:
            /** A container of all the feature indices. Optimized for space
             */
#if USE_UNORDERED_MAP
            typedef std::unordered_map<BucketKey, Bucket> BucketsSpace;
#else
            typedef std::map<BucketKey, Bucket> BucketsSpace;
#endif
            
            /** A container of all the feature indices. Optimized for speed
             */
            typedef std::vector<Bucket> BucketsSpeed;
            
            /** Default constructor
             */
            LshTable()
            {
            }
            
            /** Default constructor
             * Create the mask and allocate the memory
             * @param feature_size is the size of the feature (considered as a ElementType[])
             * @param key_size is the number of bits that are turned on in the feature
             */
            LshTable(unsigned int feature_size, unsigned int key_size)
            {
                
                std::cerr << "LSH is not implemented for that type" << feature_size<<" | "<<key_size <<std::endl;
                throw;
            }
            
            /** Add a feature to the table
             * @param value the value to store for that feature
             * @param feature the feature itself
             */
            void add(unsigned int value, const ElementType* feature)
            {
                // Add the value to the corresponding bucket
                BucketKey key = getKey(feature);
                
                switch (speed_level_) {
                    case kArray:
                        // That means we get the buckets from an array
                        buckets_speed_[key].push_back(value);
                        break;
                    case kBitsetHash:
                        // That means we can check the bitset for the presence of a key
                        key_bitset_.set(key);
                        buckets_space_[key].push_back(value);
                        break;
                    case kHash:
                    {
                        // That means we have to check for the hash table for the presence of a key
                        buckets_space_[key].push_back(value);
                        break;
                    }
                }
            }
            
            /** Add a set of features to the table
             * @param dataset the values to store
             */
            void add(const std::vector< std::pair<size_t, ElementType*> >& features)
            {
#if USE_UNORDERED_MAP
                buckets_space_.rehash((buckets_space_.size() + features.size()) * 1.2);
#endif
                //计算出index
                // Add the features to the table
                for (size_t i = 0; i < features.size(); ++i) {
                    add(features[i].first, features[i].second);
                }
                // Now that the table is full, optimize it for speed/space
                optimize();
            }
            
            /** Get a bucket given the key
             * @param key
             * @return
             */
            inline const Bucket* getBucketFromKey(BucketKey key) const
            {
                // Generate other buckets
                switch (speed_level_) {
                    case kArray:
                        // That means we get the buckets from an array
                        return &buckets_speed_[key];
                        break;
                    case kBitsetHash:
                        // That means we can check the bitset for the presence of a key
                        if (key_bitset_.test(key)) return &buckets_space_.find(key)->second;
                        else return 0;
                        break;
                    case kHash:
                    {
                        // That means we have to check for the hash table for the presence of a key
                        BucketsSpace::const_iterator bucket_it, bucket_end = buckets_space_.end();
                        bucket_it = buckets_space_.find(key);
                        // Stop here if that bucket does not exist
                        if (bucket_it == bucket_end) return 0;
                        else return &bucket_it->second;
                        break;
                    }
                }
                return 0;
            }
            
            /** Compute the sub-signature of a feature
             */
            size_t getKey(const ElementType* /*feature*/) const
            {
                std::cerr << "LSH is not implemented for that type" << std::endl;
                throw;
                return 1;
            }
            
            /** Get statistics about the table
             * @return
             */
            LshStats getStats() const;
            
        private:
            /** defines the speed fo the implementation
             * kArray uses a vector for storing data
             * kBitsetHash uses a hash map but checks for the validity of a key with a bitset
             * kHash uses a hash map only
             */
            enum SpeedLevel
            {
                kArray, kBitsetHash, kHash
            };
            
            /** Initialize some variables
             */
            void initialize(size_t key_size)
            {
                speed_level_ = kHash;
                key_size_ = key_size;
            }
            
            /** Optimize the table for speed/space
             */
            void optimize()
            {
                // If we are already using the fast storage, no need to do anything
                if (speed_level_ == kArray) return;
                
                // Use an array if it will be more than half full
                if (buckets_space_.size() > (unsigned int)((1 << key_size_) / 2)) {
                    speed_level_ = kArray;
                    // Fill the array version of it
                    buckets_speed_.resize(1 << key_size_);
                    for (BucketsSpace::const_iterator key_bucket = buckets_space_.begin(); key_bucket != buckets_space_.end(); ++key_bucket) buckets_speed_[key_bucket->first] = key_bucket->second;
                    
                    // Empty the hash table
                    buckets_space_.clear();
                    return;
                }
                
                // If the bitset is going to use less than 10% of the RAM of the hash map (at least 1 size_t for the key and two
                // for the vector) or less than 512MB (key_size_ <= 30)
                if (((std::max(buckets_space_.size(), buckets_speed_.size()) * CHAR_BIT * 3 * sizeof(BucketKey)) / 10
                     >= size_t(1 << key_size_)) || (key_size_ <= 32)) {
                    speed_level_ = kBitsetHash;
                    key_bitset_.resize(1 << key_size_);
                    key_bitset_.reset();
                    // Try with the BucketsSpace
                    for (BucketsSpace::const_iterator key_bucket = buckets_space_.begin(); key_bucket != buckets_space_.end(); ++key_bucket) key_bitset_.set(key_bucket->first);
                }
                else {
                    speed_level_ = kHash;
                    key_bitset_.clear();
                }
            }
            
            template<typename Archive>
            void serialize(Archive& ar)
            {
                int val;
                if (Archive::is_saving::value) {
                    val = (int)speed_level_;
                }
                ar & val;
                if (Archive::is_loading::value) {
                    speed_level_ = (SpeedLevel) val;
                }
                
                ar & key_size_;
                ar & mask_;
                
                if (speed_level_==kArray) {
                    ar & buckets_speed_;
                }
                if (speed_level_==kBitsetHash || speed_level_==kHash) {
                    ar & buckets_space_;
                }
                if (speed_level_==kBitsetHash) {
                    ar & key_bitset_;
                }
            }
            friend struct serialization::access;
            
            /** The vector of all the buckets if they are held for speed
             */
            BucketsSpeed buckets_speed_;
            
            /** The hash table of all the buckets in case we cannot use the speed version
             */
            BucketsSpace buckets_space_;
            
            /** What is used to store the data */
            SpeedLevel speed_level_;
            
            /** If the subkey is small enough, it will keep track of which subkeys are set through that bitset
             * That is just a speedup so that we don't look in the hash table (which can be mush slower that checking a bitset)
             */
            DynamicBitset key_bitset_;
            
            /** The size of the sub-signature in bits
             */
            unsigned int key_size_;
            
            // Members only used for the unsigned char specialization
            /** The mask to apply to a feature to get the hash key
             * Only used in the unsigned char case
             */
            std::vector<size_t> mask_;
        };
        
        
        
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Specialization for float
        template<>
        inline LshTable<float>::LshTable(unsigned int feature_size,unsigned int subsignature_size)
        {
            initialize(subsignature_size);
            mask_ = std::vector<size_t>((size_t)ceil((float)(feature_size * sizeof(float)) / (float)sizeof(size_t)), 0);
            
            // A bit brutal but fast to code
            std::vector<size_t> indices(feature_size * CHAR_BIT * sizeof(float));
            for (size_t i = 0; i < feature_size * CHAR_BIT * sizeof(float);++i) indices[i] = i;
            std::random_shuffle(indices.begin(), indices.end());//打乱顺序
            
            // Generate a random set of order of subsignature_size_ bits
            for (unsigned int i = 0; i < key_size_; ++i) {
                size_t index = indices[i];
                
                // Set that bit in the mask
                size_t divisor = CHAR_BIT * sizeof(size_t);
                size_t idx = index / divisor; //pick the right size_t index 第几个size_t
                mask_[idx] |= size_t(1) << (index % divisor); //use modulo to find the bit offset  //第i个size_t里面的第多少位（index％divisor）
            }
        }
        inline unsigned char* translate(float input)
        {
            int res = 0;
            unsigned char  charArray[sizeof(float)];
            unsigned char* pData = (unsigned char*)& input;
            for(int i=0;i<sizeof(float);i++)
            {
                charArray[i] = *pData;
                pData++;
               //cout<<(unsigned int)charArray[i]<<endl;
            }
//            float temp = 0.0;
//            void* pf = &temp;
//            unsigned char* px = charArray;
//            for(int i=0;i<sizeof(float);i++)
//            {
//                *((unsigned char*)pf+i) = *(px+i);
//            }
//            cout<<temp<<endl;
            return charArray;
        }
        /** Return the Subsignature of a feature
         * @param feature the feature to analyze
         */
        template<>
        inline size_t LshTable<float>::getKey(const float* feature) const
        {
            // no need to check if T is dividable by sizeof(size_t) like in the Hamming
            // distance computation as we have a mask
            unsigned char* featureChar = (unsigned char*)feature;
            const size_t* feature_block_ptr = reinterpret_cast<const size_t*> (featureChar);//将任意类型指针转化为size_t
            
            // Figure out the subsignature of the feature
            // Given the feature ABCDEF, and the mask 001011, the output will be
            // 000CEF
            size_t subsignature = 0;
            size_t bit_index = 1;
            
            for (std::vector<size_t>::const_iterator pmask_block = mask_.begin(); pmask_block != mask_.end(); ++pmask_block) {
                // get the mask and signature blocks
                size_t feature_block = *feature_block_ptr;
                size_t mask_block = *pmask_block;
                while (mask_block) {
                    // Get the lowest set bit in the mask block
                    size_t lowest_bit = mask_block & (-(ptrdiff_t)mask_block);//计算mask_block中有多少个1就有多少的循环 lowest_bit就是从低位开始到出现第一个1时的大小
                    // Add it to the current subsignature if necessary
                    subsignature += (feature_block & lowest_bit) ? bit_index : 0;
                    // Reset the bit in the mask block
                    mask_block ^= lowest_bit;
                    // increment the bit index for the subsignature
                    bit_index <<= 1;
                }
                // Check the next feature block
                ++feature_block_ptr;
            }
            return subsignature;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Specialization for unsigned char
        //feature_size is colums
        template<>
        inline LshTable<unsigned char>::LshTable(unsigned int feature_size, unsigned int subsignature_size)
        {
            initialize(subsignature_size);
            // Allocate the mask
            mask_ = std::vector<size_t>((size_t)ceil((float)(feature_size * sizeof(char)) / (float)sizeof(size_t)), 0);
            
            // A bit brutal but fast to code
            std::vector<size_t> indices(feature_size * CHAR_BIT);
            for (size_t i = 0; i < feature_size * CHAR_BIT; ++i) indices[i] = i;
            std::random_shuffle(indices.begin(), indices.end());//打乱顺序
            
            // Generate a random set of order of subsignature_size_ bits
            for (unsigned int i = 0; i < key_size_; ++i) {
                size_t index = indices[i];
                
                // Set that bit in the mask
                size_t divisor = CHAR_BIT * sizeof(size_t);
                size_t idx = index / divisor; //pick the right size_t index 第几个size_t
                mask_[idx] |= size_t(1) << (index % divisor); //use modulo to find the bit offset  //第i个size_t里面的第多少位（index％divisor）
            }
           
            // Set to 1 if you want to display the mask for debug
#if 0
            {
                size_t bcount = 0;
                BOOST_FOREACH(size_t mask_block, mask_){
                    out << std::setw(sizeof(size_t) * CHAR_BIT / 4) << std::setfill('0') << std::hex << mask_block
                    << std::endl;
                    bcount += __builtin_popcountll(mask_block);
                }
                out << "bit count : " << std::dec << bcount << std::endl;
                out << "mask size : " << mask_.size() << std::endl;
                return out;
            }
#endif
        }
        
        /** Return the Subsignature of a feature
         * @param feature the feature to analyze
         */
        template<>
        inline size_t LshTable<unsigned char>::getKey(const unsigned char* feature) const
        {
            // no need to check if T is dividable by sizeof(size_t) like in the Hamming
            // distance computation as we have a mask
            const size_t* feature_block_ptr = reinterpret_cast<const size_t*> (feature);//将任意类型指针转化为size_t
            
            // Figure out the subsignature of the feature
            // Given the feature ABCDEF, and the mask 001011, the output will be
            // 000CEF
            size_t subsignature = 0;
            size_t bit_index = 1;
            
            for (std::vector<size_t>::const_iterator pmask_block = mask_.begin(); pmask_block != mask_.end(); ++pmask_block) {
                // get the mask and signature blocks
                size_t feature_block = *feature_block_ptr;
                size_t mask_block = *pmask_block;
                while (mask_block) {
                    // Get the lowest set bit in the mask block
                    size_t lowest_bit = mask_block & (-(ptrdiff_t)mask_block);//计算mask_block中有多少个1就有多少的循环 lowest_bit就是从低位开始到出现第一个1时的大小
                    // Add it to the current subsignature if necessary
                    subsignature += (feature_block & lowest_bit) ? bit_index : 0;
                    // Reset the bit in the mask block
                    mask_block ^= lowest_bit;
                    // increment the bit index for the subsignature
                    bit_index <<= 1;
                }
                // Check the next feature block
                ++feature_block_ptr;
            }
            return subsignature;
        }
        // End the two namespaces
    }
}

#endif /* lsh_table_h */
