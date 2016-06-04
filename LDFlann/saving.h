//
//  saving.h
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#ifndef saving_h
#define saving_h

#include <cstring>
#include <vector>
#include <stdio.h>

#include "general.h"
#include "serialization.h"


#ifdef FLANN_SIGNATURE_
#undef FLANN_SIGNATURE_
#endif
#define FLANN_SIGNATURE_ "FLANN_INDEX"

namespace LDFlann
{
    
    /**
     * Structure representing the index header.
     */
    struct IndexHeader
    {
        char signature[16];
        char version[16];
        flann_datatype_t data_type;
        flann_algorithm_t index_type;
        flann_distance_t distance_type;
        size_t rows;
        size_t cols;
        
        
        IndexHeader()
        {
            memset(signature, 0, sizeof(signature));
            strcpy(signature, FLANN_SIGNATURE_);
            memset(version, 0, sizeof(version));
            strcpy(version, FLANN_VERSION_);
        }
        
    private:
        template<typename Archive>
        void serialize(Archive& ar)
        {
            ar & signature;
            ar & version;
            ar & data_type;
            ar & index_type;
            ar & rows;
            ar & cols;
        }
        friend struct serialization::access;
    };
    
    /**
     * Saves index header to stream
     *
     * @param stream - Stream to save to
     * @param index - The index to save
     */
    template<typename Index>
    void save_header(FILE* stream, const Index& index)
    {
        IndexHeader header;
        header.data_type = flann_datatype_value<typename Index::ElementType>::value;
        header.index_type = index.getType();
        header.rows = index.size();
        header.cols = index.veclen();
        
        fwrite(&header, sizeof(header),1,stream);
    }
    
    
    /**
     *
     * @param stream - Stream to load from
     * @return Index header
     */
    inline IndexHeader load_header(FILE* stream)
    {
        IndexHeader header;
        int read_size = fread(&header,sizeof(header),1,stream);
        
        if (read_size!=1) {
            throw FLANNException("Invalid index file, cannot read");
        }
        
        if (strcmp(header.signature,FLANN_SIGNATURE_)!=0) {
            throw FLANNException("Invalid index file, wrong signature");
        }
        
        return header;
    }
    
    
    namespace serialization
    {
        ENUM_SERIALIZER(flann_algorithm_t);
        ENUM_SERIALIZER(flann_centers_init_t);
        ENUM_SERIALIZER(flann_log_level_t);
        ENUM_SERIALIZER(flann_datatype_t);
    }
    
}
#endif /* saving_h */
