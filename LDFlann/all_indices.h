//
//  all_indices.h
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#ifndef all_indices_h
#define all_indices_h
#include "general.h"

#include "nn_index.h"
#include "lsh_index.h"

namespace LDFlann
{
    
    /**
     * enable_if sfinae helper
     */
    template<bool, typename T = void> struct enable_if{};
    template<typename T> struct enable_if<true,T> { typedef T type; };
    
    /**
     * disable_if sfinae helper
     */
    template<bool, typename T> struct disable_if{ typedef T type; };
    template<typename T> struct disable_if<true,T> { };
    
    /**
     * Check if two type are the same
     */
    template <typename T, typename U>
    struct same_type
    {
        enum {value = false};
    };
    
    template<typename T>
    struct same_type<T,T>
    {
        enum {value = true};
    };
    
#define HAS_MEMBER(member) \
template<typename T> \
struct member { \
typedef char No; \
typedef long Yes; \
template<typename C> static Yes test( typename C::member* ); \
template<typename C> static No test( ... ); \
enum { value = sizeof (test<T>(0))==sizeof(Yes) }; \
};
    
    HAS_MEMBER(needs_kdtree_distance)
    HAS_MEMBER(needs_vector_space_distance)
    HAS_MEMBER(is_kdtree_distance)
    HAS_MEMBER(is_vector_space_distance)
    
    struct DummyDistance
    {
        typedef float ElementType;
        typedef float ResultType;
        
        template <typename Iterator1, typename Iterator2>
        ResultType operator()(Iterator1 a, Iterator2 b, size_t size, ResultType /*worst_dist*/ = -1) const
        {
            return ResultType(0);
        }
        
        template <typename U, typename V>
        inline ResultType accum_dist(const U& a, const V& b, int) const
        {
            return ResultType(0);
        }
    };
    
    /**
     * Checks if an index and a distance can be used together
     */
    template<template <typename> class Index, typename Distance, typename ElemType>
    struct valid_combination
    {
        static const bool value = same_type<ElemType,typename Distance::ElementType>::value &&
    				(!needs_kdtree_distance<Index<DummyDistance> >::value || is_kdtree_distance<Distance>::value) &&
    				(!needs_vector_space_distance<Index<DummyDistance> >::value || is_kdtree_distance<Distance>::value || is_vector_space_distance<Distance>::value);
        
    };
    
    
    /*********************************************************
     * Create index
     **********************************************************/
    //here the index 赋值给了对应的index，因为构造函数参数的不同
    template <template<typename> class Index, typename Distance, typename T>
    inline NNIndex<Distance>* create_index_(LDFlann::Matrix<T> data, const LDFlann::IndexParams& params, const Distance& distance,
                                            typename enable_if<valid_combination<Index,Distance,T>::value,void>::type* = 0)
    {
        return new Index<Distance>(data, params, distance);
    }
    
    template <template<typename> class Index, typename Distance, typename T>
    inline NNIndex<Distance>* create_index_(LDFlann::Matrix<T> data, const LDFlann::IndexParams& params, const Distance& distance,
                                            typename disable_if<valid_combination<Index,Distance,T>::value,void>::type* = 0)
    {
        return NULL;
    }
    
    template<typename Distance>
    inline NNIndex<Distance>*
    create_index_by_type(const flann_algorithm_t index_type,
                         const Matrix<typename Distance::ElementType>& dataset, const IndexParams& params, const Distance& distance)
    {
        typedef typename Distance::ElementType ElementType;
        
        NNIndex<Distance>* nnIndex;
        
        switch (index_type) {
                
            case FLANN_INDEX_LSH:
                nnIndex = create_index_<LshIndex,Distance,ElementType>(dataset, params, distance);
                break;
            default:
                throw FLANNException("Unknown index type");
        }
        
        if (nnIndex==NULL) {
            throw FLANNException("Unsupported index/distance combination");
        }
        return nnIndex;
    }
    
}

#endif /* all_indices_h */
