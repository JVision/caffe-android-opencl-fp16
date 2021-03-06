#include <algorithm>
#include <vector>

#include "caffe/layers/elu_layer.hpp"

namespace caffe {

template <typename Dtype>
void ELULayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  const Dtype* bottom_data = bottom[0]->cpu_data();
  Dtype* top_data = top[0]->mutable_cpu_data();
  const int count = bottom[0]->count();
  Dtype alpha = this->layer_param_.elu_param().alpha();
  for (int i = 0; i < count; ++i) {
    top_data[i] = std::max(bottom_data[i], Dtype(0))
        + alpha * (exp(std::min(bottom_data[i], Dtype(0))) - Dtype(1));
  }
}

#ifdef CPU_ONLY
STUB_GPU(ELULayer);
#elif USE_OPENCL

template <>
void ELULayer<half>::Forward_gpu(const vector<Blob<half>*>& bottom,
    const vector<Blob<half>*>& top) {

  const half* bottom_data = bottom[0]->gpu_data();
  half* top_data = top[0]->mutable_gpu_data();
  const int count = bottom[0]->count();
  half alpha = this->layer_param_.elu_param().alpha();

  cl_int ret;

  cl_kernel kernel = clCreateKernel(Caffe::Get().math_program, "ELUForward", &ret);
  OPENCL_CHECK(ret);

  // Set arguments for kernel
  half_b alpha_half = float2half_impl(alpha);
  OPENCL_CHECK(clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&bottom_data));  
  OPENCL_CHECK(clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&top_data));  
  OPENCL_CHECK(clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&count));
  OPENCL_CHECK(clSetKernelArg(kernel, 3, sizeof(cl_half), (void *)&alpha_half));

  size_t global_size = CAFFE_GET_BLOCKS(count);
  
  OPENCL_CHECK(clEnqueueNDRangeKernel(Caffe::Get().commandQueue, kernel, 1, NULL, &global_size, &CAFFE_CUDA_NUM_THREADS, 0, NULL, NULL));  

}


template <>
void ELULayer<float>::Forward_gpu(const vector<Blob<float>*>& bottom,
    const vector<Blob<float>*>& top) {

  const float* bottom_data = bottom[0]->gpu_data();
  float* top_data = top[0]->mutable_gpu_data();
  const int count = bottom[0]->count();
  float alpha = this->layer_param_.elu_param().alpha();

  cl_int ret;

  cl_kernel kernel = clCreateKernel(Caffe::Get().math_program, "ELUForward", &ret);
  OPENCL_CHECK(ret);

  // Set arguments for kernel
  OPENCL_CHECK(clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&bottom_data));  
  OPENCL_CHECK(clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&top_data));  
  OPENCL_CHECK(clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&count));
  OPENCL_CHECK(clSetKernelArg(kernel, 3, sizeof(cl_float), (void *)&alpha));

  size_t global_size = CAFFE_GET_BLOCKS(count);
  
  OPENCL_CHECK(clEnqueueNDRangeKernel(Caffe::Get().commandQueue, kernel, 1, NULL, &global_size, &CAFFE_CUDA_NUM_THREADS, 0, NULL, NULL));  

}



#endif




INSTANTIATE_CLASS(ELULayer);
REGISTER_LAYER_CLASS(ELU);

}  // namespace caffe
