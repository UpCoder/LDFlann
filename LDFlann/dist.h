//
//  dist.h
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#ifndef dist_h
#define dist_h

#include <cmath>
#include <cstdlib>
#include <string.h>
#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#include "defines.h"


namespace LDFlann
{
    
    template<typename T>
    struct Accumulator { typedef T Type; };
    template<>
    struct Accumulator<unsigned char>  { typedef float Type; };
    template<>
    struct Accumulator<unsigned short> { typedef float Type; };
    template<>
    struct Accumulator<unsigned int> { typedef float Type; };
    template<>
    struct Accumulator<char>   { typedef float Type; };
    template<>
    struct Accumulator<short>  { typedef float Type; };
    template<>
    struct Accumulator<int> { typedef float Type; };
    
    
    
       ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * Hamming distance functor - counts the bit differences between two strings - useful for the Brief descriptor
     * bit count of A exclusive XOR'ed with B
     */
    struct HammingLUT
    {
        typedef unsigned char ElementType;
        typedef int ResultType;
        
        /** this will count the bits in a ^ b
         */
        ResultType operator()(const unsigned char* a, const unsigned char* b, int size) const
        {
            ResultType result = 0;
            for (int i = 0; i < size; i++) {
                result += byteBitsLookUp(a[i] ^ b[i]);
            }
            return result;
        }
        
        
        /** \brief given a byte, count the bits using a look up table
         *  \param b the byte to count bits.  The look up table has an entry for all
         *  values of b, where that entry is the number of bits.
         *  \return the number of bits in byte b
         */
        static unsigned char byteBitsLookUp(unsigned char b)
        {
            static const unsigned char table[256]  = {
                /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
                /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
                /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
                /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4,
                /* 10 */ 1, /* 11 */ 2, /* 12 */ 2, /* 13 */ 3,
                /* 14 */ 2, /* 15 */ 3, /* 16 */ 3, /* 17 */ 4,
                /* 18 */ 2, /* 19 */ 3, /* 1a */ 3, /* 1b */ 4,
                /* 1c */ 3, /* 1d */ 4, /* 1e */ 4, /* 1f */ 5,
                /* 20 */ 1, /* 21 */ 2, /* 22 */ 2, /* 23 */ 3,
                /* 24 */ 2, /* 25 */ 3, /* 26 */ 3, /* 27 */ 4,
                /* 28 */ 2, /* 29 */ 3, /* 2a */ 3, /* 2b */ 4,
                /* 2c */ 3, /* 2d */ 4, /* 2e */ 4, /* 2f */ 5,
                /* 30 */ 2, /* 31 */ 3, /* 32 */ 3, /* 33 */ 4,
                /* 34 */ 3, /* 35 */ 4, /* 36 */ 4, /* 37 */ 5,
                /* 38 */ 3, /* 39 */ 4, /* 3a */ 4, /* 3b */ 5,
                /* 3c */ 4, /* 3d */ 5, /* 3e */ 5, /* 3f */ 6,
                /* 40 */ 1, /* 41 */ 2, /* 42 */ 2, /* 43 */ 3,
                /* 44 */ 2, /* 45 */ 3, /* 46 */ 3, /* 47 */ 4,
                /* 48 */ 2, /* 49 */ 3, /* 4a */ 3, /* 4b */ 4,
                /* 4c */ 3, /* 4d */ 4, /* 4e */ 4, /* 4f */ 5,
                /* 50 */ 2, /* 51 */ 3, /* 52 */ 3, /* 53 */ 4,
                /* 54 */ 3, /* 55 */ 4, /* 56 */ 4, /* 57 */ 5,
                /* 58 */ 3, /* 59 */ 4, /* 5a */ 4, /* 5b */ 5,
                /* 5c */ 4, /* 5d */ 5, /* 5e */ 5, /* 5f */ 6,
                /* 60 */ 2, /* 61 */ 3, /* 62 */ 3, /* 63 */ 4,
                /* 64 */ 3, /* 65 */ 4, /* 66 */ 4, /* 67 */ 5,
                /* 68 */ 3, /* 69 */ 4, /* 6a */ 4, /* 6b */ 5,
                /* 6c */ 4, /* 6d */ 5, /* 6e */ 5, /* 6f */ 6,
                /* 70 */ 3, /* 71 */ 4, /* 72 */ 4, /* 73 */ 5,
                /* 74 */ 4, /* 75 */ 5, /* 76 */ 5, /* 77 */ 6,
                /* 78 */ 4, /* 79 */ 5, /* 7a */ 5, /* 7b */ 6,
                /* 7c */ 5, /* 7d */ 6, /* 7e */ 6, /* 7f */ 7,
                /* 80 */ 1, /* 81 */ 2, /* 82 */ 2, /* 83 */ 3,
                /* 84 */ 2, /* 85 */ 3, /* 86 */ 3, /* 87 */ 4,
                /* 88 */ 2, /* 89 */ 3, /* 8a */ 3, /* 8b */ 4,
                /* 8c */ 3, /* 8d */ 4, /* 8e */ 4, /* 8f */ 5,
                /* 90 */ 2, /* 91 */ 3, /* 92 */ 3, /* 93 */ 4,
                /* 94 */ 3, /* 95 */ 4, /* 96 */ 4, /* 97 */ 5,
                /* 98 */ 3, /* 99 */ 4, /* 9a */ 4, /* 9b */ 5,
                /* 9c */ 4, /* 9d */ 5, /* 9e */ 5, /* 9f */ 6,
                /* a0 */ 2, /* a1 */ 3, /* a2 */ 3, /* a3 */ 4,
                /* a4 */ 3, /* a5 */ 4, /* a6 */ 4, /* a7 */ 5,
                /* a8 */ 3, /* a9 */ 4, /* aa */ 4, /* ab */ 5,
                /* ac */ 4, /* ad */ 5, /* ae */ 5, /* af */ 6,
                /* b0 */ 3, /* b1 */ 4, /* b2 */ 4, /* b3 */ 5,
                /* b4 */ 4, /* b5 */ 5, /* b6 */ 5, /* b7 */ 6,
                /* b8 */ 4, /* b9 */ 5, /* ba */ 5, /* bb */ 6,
                /* bc */ 5, /* bd */ 6, /* be */ 6, /* bf */ 7,
                /* c0 */ 2, /* c1 */ 3, /* c2 */ 3, /* c3 */ 4,
                /* c4 */ 3, /* c5 */ 4, /* c6 */ 4, /* c7 */ 5,
                /* c8 */ 3, /* c9 */ 4, /* ca */ 4, /* cb */ 5,
                /* cc */ 4, /* cd */ 5, /* ce */ 5, /* cf */ 6,
                /* d0 */ 3, /* d1 */ 4, /* d2 */ 4, /* d3 */ 5,
                /* d4 */ 4, /* d5 */ 5, /* d6 */ 5, /* d7 */ 6,
                /* d8 */ 4, /* d9 */ 5, /* da */ 5, /* db */ 6,
                /* dc */ 5, /* dd */ 6, /* de */ 6, /* df */ 7,
                /* e0 */ 3, /* e1 */ 4, /* e2 */ 4, /* e3 */ 5,
                /* e4 */ 4, /* e5 */ 5, /* e6 */ 5, /* e7 */ 6,
                /* e8 */ 4, /* e9 */ 5, /* ea */ 5, /* eb */ 6,
                /* ec */ 5, /* ed */ 6, /* ee */ 6, /* ef */ 7,
                /* f0 */ 4, /* f1 */ 5, /* f2 */ 5, /* f3 */ 6,
                /* f4 */ 5, /* f5 */ 6, /* f6 */ 6, /* f7 */ 7,
                /* f8 */ 5, /* f9 */ 6, /* fa */ 6, /* fb */ 7,
                /* fc */ 6, /* fd */ 7, /* fe */ 7, /* ff */ 8
            };
            return table[b];
        }
    };
    
    /**
     * Hamming distance functor (pop count between two binary vectors, i.e. xor them and count the number of bits set)
     * That code was taken from brief.cpp in OpenCV
     */
    template<class T>
    struct HammingPopcnt
    {
        typedef T ElementType;
        typedef int ResultType;
        
        template<typename Iterator1, typename Iterator2>
        ResultType operator()(Iterator1 a, Iterator2 b, size_t size, ResultType /*worst_dist*/ = -1) const
        {
            ResultType result = 0;
#if __GNUC__
#if ANDROID && HAVE_NEON
            static uint64_t features = android_getCpuFeatures();
            if ((features& ANDROID_CPU_ARM_FEATURE_NEON)) {
                for (size_t i = 0; i < size; i += 16) {
                    uint8x16_t A_vec = vld1q_u8 (a + i);
                    uint8x16_t B_vec = vld1q_u8 (b + i);
                    //uint8x16_t veorq_u8 (uint8x16_t, uint8x16_t)
                    uint8x16_t AxorB = veorq_u8 (A_vec, B_vec);
                    
                    uint8x16_t bitsSet += vcntq_u8 (AxorB);
                    //uint16x8_t vpadalq_u8 (uint16x8_t, uint8x16_t)
                    uint16x8_t bitSet8 = vpaddlq_u8 (bitsSet);
                    uint32x4_t bitSet4 = vpaddlq_u16 (bitSet8);
                    
                    uint64x2_t bitSet2 = vpaddlq_u32 (bitSet4);
                    result += vgetq_lane_u64 (bitSet2,0);
                    result += vgetq_lane_u64 (bitSet2,1);
                }
            }
            else
#endif
                //for portability just use unsigned long -- and use the __builtin_popcountll (see docs for __builtin_popcountll)
                typedef unsigned long long pop_t;
            const size_t modulo = size % sizeof(pop_t);
            const pop_t* a2 = reinterpret_cast<const pop_t*> (a);
            const pop_t* b2 = reinterpret_cast<const pop_t*> (b);
            const pop_t* a2_end = a2 + (size / sizeof(pop_t));
            
            for (; a2 != a2_end; ++a2, ++b2) result += __builtin_popcountll((*a2) ^ (*b2));
            
            if (modulo) {
                //in the case where size is not dividable by sizeof(size_t)
                //need to mask off the bits at the end
                pop_t a_final = 0, b_final = 0;
                memcpy(&a_final, a2, modulo);
                memcpy(&b_final, b2, modulo);
                result += __builtin_popcountll(a_final ^ b_final);
            }
#else
            HammingLUT lut;
            result = lut(reinterpret_cast<const unsigned char*> (a),
                         reinterpret_cast<const unsigned char*> (b), size * sizeof(pop_t));
#endif
            return result;
        }
    };
    
    template<typename T>
    struct Hamming
    {
        typedef T ElementType;
        typedef unsigned int ResultType;
        
        /** This is popcount_3() from:
         * http://en.wikipedia.org/wiki/Hamming_weight */
        unsigned int popcnt32(uint32_t n) const
        {
            n -= ((n >> 1) & 0x55555555);
            n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
            return (((n + (n >> 4))& 0xF0F0F0F)* 0x1010101) >> 24;
        }
        
        unsigned int popcnt64(uint64_t n) const
        {
            n -= ((n >> 1) & 0x5555555555555555LL);
            n = (n & 0x3333333333333333LL) + ((n >> 2) & 0x3333333333333333LL);
            return (((n + (n >> 4))& 0x0f0f0f0f0f0f0f0fLL)* 0x0101010101010101LL) >> 56;
        }
        
        template <typename Iterator1, typename Iterator2>
        ResultType operator()(Iterator1 a, Iterator2 b, size_t size, ResultType /*worst_dist*/ = 0) const
        {
#ifdef FLANN_PLATFORM_64_BIT
            const uint64_t* pa = reinterpret_cast<const uint64_t*>(a);
            const uint64_t* pb = reinterpret_cast<const uint64_t*>(b);
            ResultType result = 0;
            size /= (sizeof(uint64_t)/sizeof(unsigned char));
            for(size_t i = 0; i < size; ++i ) {
                result += popcnt64(*pa ^ *pb);
                ++pa;
                ++pb;
            }
#else
            const uint32_t* pa = reinterpret_cast<const uint32_t*>(a);
            const uint32_t* pb = reinterpret_cast<const uint32_t*>(b);
            ResultType result = 0;
            size /= (sizeof(uint32_t)/sizeof(unsigned char));
            for(size_t i = 0; i < size; ++i ) {
                result += popcnt32(*pa ^ *pb);
                ++pa;
                ++pb;
            }
#endif
            return result;
        }
    };
}

#endif /* dist_h */
