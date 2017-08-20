#include <stdio.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <thrust/device_vector.h>
#include <thrust/reduce.h>
#include <thrust/sequence.h>
#include <arrayfire.h>
# include <af/cuda.h>

template<typename T>
struct Fun
{
    __device__ T operator()(T t1, T t2)  {
        auto result = t1+t2;
        return result;
    }
};

__global__
void
increment_kernel( const int num, const float * d_x, float * d_y )
{
    const int id = blockIdx.x * blockDim.x + threadIdx.x;

    if ( id < num )
        d_y[id] = -d_x[id];
}

void
increment( af::array& a )
{
    a.eval(); // Ensure any JIT kernels have executed

    
    // Determine ArrayFire's CUDA stream
    int afid = af::getDevice();
    int cudaid = afcu::getNativeId( afid );
    cudaStream_t af_cuda_stream = afcu::getStream( cudaid );

    af::array b = af::constant( 0, a.dims(0), a.dims(1), f32 );
    
    const float * d_a = a.device< float >();
    float * d_b = b.device< float >();

    increment_kernel <<< 6, 1, 0, af_cuda_stream >>>( 6, d_a, d_b );

    cudaDeviceSynchronize();

    a.unlock();
    af::print( "b", b );
}

