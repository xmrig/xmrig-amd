#ifndef __OCLCRYPTONIGHTR_GEN_H__
#define __OCLCRYPTONIGHTR_GEN_H__

#include "amd/GpuContext.h"

cl_program CryptonightR_get_program(GpuContext* ctx, xmrig::Variant variant, uint64_t height, bool background = false, cl_kernel old_kernel = nullptr);

#endif
