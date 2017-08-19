/**
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/**
 * Vector addition: C = A + B.
 *
 * This sample is a very basic sample that implements element by element
 * vector addition. It is the same as the sample illustrating Chapter 2
 * of the programming guide with some additions like error checking.
 */

#include <stdio.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <thrust/device_vector.h>
#include <thrust/reduce.h>
#include <thrust/sequence.h>

template<typename T>
struct Fun
{
    __device__ T operator()(T t1, T t2)  {
        auto result = t1+t2;
        return result;
    }
};

int
run()
{
    const int N = 100;
    thrust::device_vector<int> vec(N);
    thrust::sequence(vec.begin(),vec.end());
    auto op = Fun<int>();

    return thrust::reduce(vec.begin(),vec.end(),0,op);
}