#ifndef __OCLGPU_H__
#define __OCLGPU_H__


#include <vector>


#include "amd/GpuContext.h"


#define OCL_ERR_SUCCESS    (0)
#define OCL_ERR_API        (2)
#define OCL_ERR_BAD_PARAMS (1)


uint32_t getNumPlatforms();
void printPlatforms();
int getAMDPlatformIdx();
std::vector<GpuContext> getAMDDevices(int index);

size_t InitOpenCL(GpuContext* ctx, size_t num_gpus, size_t platform_idx);
size_t XMRSetJob(GpuContext* ctx, uint8_t* input, size_t input_len, uint64_t target);
size_t XMRRunJob(GpuContext* ctx, cl_uint* HashOutput);

#endif /* __OCLGPU_H__ */
