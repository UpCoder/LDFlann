//
//  hdf5.h
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#ifndef hdf5_h
#define hdf5_h
#include <hdf5.h>
#include "matrix.h"
#define CHECK_ERROR(x,y) if ((x)<0) throw FLANNException((y));

namespace LDFlann{
    namespace{
        template<typename T>
        hid_t get_hdf5_type()
        {
            throw FLANNException("Unsupported type for IO operations");
        }
        template<>
        hid_t get_hdf5_type<char>() { return H5T_NATIVE_CHAR; }
        template<>
        hid_t get_hdf5_type<unsigned char>() { return H5T_NATIVE_UCHAR; }
        template<>
        hid_t get_hdf5_type<short int>() { return H5T_NATIVE_SHORT; }
        template<>
        hid_t get_hdf5_type<unsigned short int>() { return H5T_NATIVE_USHORT; }
        template<>
        hid_t get_hdf5_type<int>() { return H5T_NATIVE_INT; }
        template<>
        hid_t get_hdf5_type<unsigned int>() { return H5T_NATIVE_UINT; }
        template<>
        hid_t get_hdf5_type<long>() { return H5T_NATIVE_LONG; }
        template<>
        hid_t get_hdf5_type<unsigned long>() { return H5T_NATIVE_ULONG; }
        template<>
        hid_t get_hdf5_type<float>() { return H5T_NATIVE_FLOAT; }
        template<>
        hid_t get_hdf5_type<double>() { return H5T_NATIVE_DOUBLE; }
        template<>
        hid_t get_hdf5_type<long double>() { return H5T_NATIVE_LDOUBLE; }
    }
        template<typename T>
        void save_to_file(const LDFlann::Matrix<T>& dataset, const std::string& filename, const std::string& name)
        {
            
#if H5Eset_auto_vers == 2
            H5Eset_auto( H5E_DEFAULT, NULL, NULL );
#else
            H5Eset_auto( NULL, NULL );
#endif
            
            herr_t status;
            hid_t file_id;
            file_id = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
            if (file_id < 0) {
                file_id = H5Fcreate(filename.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
            }
            CHECK_ERROR(file_id,"Error creating hdf5 file.");
            
            hsize_t     dimsf[2];              // dataset dimensions
            dimsf[0] = dataset.rows;
            dimsf[1] = dataset.cols;
            
            hid_t space_id = H5Screate_simple(2, dimsf, NULL);
            hid_t memspace_id = H5Screate_simple(2, dimsf, NULL);
            
            hid_t dataset_id;
#if H5Dcreate_vers == 2
            dataset_id = H5Dcreate2(file_id, name.c_str(), get_hdf5_type<T>(), space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
#else
            dataset_id = H5Dcreate(file_id, name.c_str(), get_hdf5_type<T>(), space_id, H5P_DEFAULT);
#endif
            
            if (dataset_id<0) {
#if H5Dopen_vers == 2
                dataset_id = H5Dopen2(file_id, name.c_str(), H5P_DEFAULT);
#else
                dataset_id = H5Dopen(file_id, name.c_str());
#endif
            }
            CHECK_ERROR(dataset_id,"Error creating or opening dataset in file.");
            
            status = H5Dwrite(dataset_id, get_hdf5_type<T>(), memspace_id, space_id, H5P_DEFAULT, dataset.ptr() );
            CHECK_ERROR(status, "Error writing to dataset");
            
            H5Sclose(memspace_id);
            H5Sclose(space_id);
            H5Dclose(dataset_id);
            H5Fclose(file_id);
            
        }
        template<typename T>
        void load_from_file(LDFlann::Matrix<T>& dataset, const std::string& filename, const std::string& name)
        {
            herr_t status;
            hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
            CHECK_ERROR(file_id,"Error opening hdf5 file.");
            
            hid_t dataset_id;
#if H5Dopen_vers == 2
            dataset_id = H5Dopen2(file_id, name.c_str(), H5P_DEFAULT);
#else
            dataset_id = H5Dopen(file_id, name.c_str());
#endif
            CHECK_ERROR(dataset_id,"Error opening dataset in file.");
            
            hid_t space_id = H5Dget_space(dataset_id);
            
            hsize_t dims_out[2];
            H5Sget_simple_extent_dims(space_id, dims_out, NULL);
            
            dataset = LDFlann::Matrix<T>(new T[dims_out[0]*dims_out[1]], dims_out[0], dims_out[1]);
            
            status = H5Dread(dataset_id, get_hdf5_type<T>(), H5S_ALL, H5S_ALL, H5P_DEFAULT, dataset[0]);
            CHECK_ERROR(status, "Error reading dataset");
            
            H5Sclose(space_id);
            H5Dclose(dataset_id);
            H5Fclose(file_id);
        }

}
#endif /* hdf5_h */
