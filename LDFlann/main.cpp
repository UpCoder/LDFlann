//
//  main.cpp
//  LDFlann
//
//  Created by 梁栋 on 16/6/3.
//  Copyright © 2016年 梁栋. All rights reserved.
//

#include <iostream>
#include "hdf5.h"
#include "matrix.h"
#include "flann.cpp"
#include <sys/time.h>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <string>
using namespace LDFlann;

using namespace std;
int main(int argc, const char * argv[]) {
    int nn = 3;
    //freopen("/Users/Liang/Desktop/FlannTest1/FlannTest1/show.txt", "w", stdout);
    struct timeval start, end;
    gettimeofday( &start, NULL );
    Matrix<float> dataset;
    Matrix<float> query;
    load_from_file(dataset, "/Users/Liang/Desktop/FlannTest1/FlannTest1/dataset.hdf5","dataset");
    load_from_file(query, "/Users/Liang/Desktop/FlannTest1/FlannTest1/dataset.hdf5","query");
    cout<<query.rows<<endl;
    Matrix<int> indices(new int[query.rows*nn], query.rows, nn);
    Matrix<float> dists(new float[query.rows*nn], query.rows, nn);
    Matrix<unsigned int> dists1(new unsigned int[query.rows*nn],query.rows,nn);
    
    Index<Hamming<float>> indexLSH(dataset,LshIndexParams(4));
    gettimeofday( &start, NULL );
    indexLSH.buildIndex();
    gettimeofday( &end, NULL );
    int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec;
    printf("time: %d us\n", timeuse);
    gettimeofday( &start, NULL );
    int res = indexLSH.knnSearch(query, indices, dists1, nn, SearchParams(128));
    cout<<res<<endl;

    delete[] dataset.ptr();
    delete[] query.ptr();
    delete[] indices.ptr();
    delete[] dists.ptr();
    delete [] dists1.ptr();
    gettimeofday( &end, NULL );
    timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec;
    printf("time: %d us\n", timeuse);
    return 0;
}
