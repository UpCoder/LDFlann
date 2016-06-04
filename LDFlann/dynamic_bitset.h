//
//  dynamic_bitset.h
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#ifndef dynamic_bitset_h
#define dynamic_bitset_h

#if FLANN_USE_BOOST
#include <boost/dynamic_bitset.hpp>
typedef boost::dynamic_bitset<> DynamicBitset;
#else

#include <limits.h>

namespace LDFlann {
    
    /** Class re-implementing the boost version of it
     * This helps not depending on boost, it also does not do the bound checks
     * and has a way to reset a block for speed
     */
    class DynamicBitset
    {
    public:
        /** @param default constructor
         */
        DynamicBitset() : size_(0)
        {
        }
        
        /** @param only constructor we use in our code
         * @param the size of the bitset (in bits)
         */
        DynamicBitset(size_t size)
        {
            resize(size);
            reset();
        }
        
        /** Sets all the bits to 0
         */
        void clear()
        {
            std::fill(bitset_.begin(), bitset_.end(), 0);
        }
        
        /** @brief checks if the bitset is empty
         * @return true if the bitset is empty
         */
        bool empty() const
        {
            return bitset_.empty();
        }
        
        /** @param set all the bits to 0
         */
        void reset()
        {
            std::fill(bitset_.begin(), bitset_.end(), 0);
        }
        
        /** @brief set one bit to 0
         * @param
         */
        void reset(size_t index)
        {
            bitset_[index / cell_bit_size_] &= ~(size_t(1) << (index % cell_bit_size_));
        }
        
        /** @brief sets a specific bit to 0, and more bits too
         * This function is useful when resetting a given set of bits so that the
         * whole bitset ends up being 0: if that's the case, we don't care about setting
         * other bits to 0
         * @param
         */
        void reset_block(size_t index)
        {
            bitset_[index / cell_bit_size_] = 0;
        }
        
        /** @param resize the bitset so that it contains at least size bits
         * @param size
         */
        void resize(size_t size)
        {
            size_ = size;
            bitset_.resize(size / cell_bit_size_ + 1);
        }
        
        /** @param set a bit to true
         * @param index the index of the bit to set to 1
         */
        void set(size_t index)
        {
            bitset_[index / cell_bit_size_] |= size_t(1) << (index % cell_bit_size_);
        }
        
        /** @param gives the number of contained bits
         */
        size_t size() const
        {
            return size_;
        }
        
        /** @param check if a bit is set
         * @param index the index of the bit to check
         * @return true if the bit is set
         */
        bool test(size_t index) const
        {
            return (bitset_[index / cell_bit_size_] & (size_t(1) << (index % cell_bit_size_))) != 0;
        }
        
    private:
        template <typename Archive>
        void serialize(Archive& ar)
        {
            ar & size_;
            ar & bitset_;
        }
        friend struct serialization::access;
        
    private:
        std::vector<size_t> bitset_;
        size_t size_;
        static const unsigned int cell_bit_size_ = CHAR_BIT * sizeof(size_t);
    };
    
} // namespace flann

#endif


#endif /* dynamic_bitset_h */
