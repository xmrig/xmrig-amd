/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <uv.h>


#include "amd/OclError.h"
#include "amd/OclLib.h"
#include "common/log/Log.h"


static uv_lib_t oclLib;

static const char *kErrorTemplate           = "Error %s when calling %s.";

static const char *kBuildProgram            = "clBuildProgram";
static const char *kCreateProgramWithBinary = "clCreateProgramWithBinary";
static const char *kCreateProgramWithSource = "clCreateProgramWithSource";
static const char *kGetDeviceInfo           = "clGetDeviceInfo";
static const char *kGetProgramBuildInfo     = "clGetProgramBuildInfo";
static const char *kGetProgramInfo          = "clGetProgramInfo";

static cl_int (*pBuildProgram)(cl_program, cl_uint, const cl_device_id *, const char *, void (CL_CALLBACK *pfn_notify)(cl_program, void *), void *)  = nullptr;
static cl_int (*pGetDeviceInfo)(cl_device_id, cl_device_info, size_t, void *, size_t *)                                                              = nullptr;
static cl_int (*pGetProgramBuildInfo)(cl_program, cl_device_id, cl_program_build_info, size_t, void *, size_t *)                                     = nullptr;
static cl_int (*pGetProgramInfo)(cl_program, cl_program_info, size_t, void *, size_t *)                                                              = nullptr;
static cl_program (*pCreateProgramWithBinary)(cl_context, cl_uint, const cl_device_id *, const size_t *, const unsigned char **, cl_int *, cl_int *) = nullptr;
static cl_program (*pCreateProgramWithSource)(cl_context, cl_uint, const char **, const size_t *, cl_int *)                                          = nullptr;


#define DLSYM(x) if (uv_dlsym(&oclLib, k##x, reinterpret_cast<void**>(&p##x)) == -1) { return false; }


bool OclLib::init(const char *fileName)
{
    if (uv_dlopen(fileName, &oclLib) == -1 || !load()) {
        LOG_ERR("Failed to load OpenCL runtime: %s", uv_dlerror(&oclLib));
        return false;
    }

    return true;
}


bool OclLib::load()
{
    DLSYM(BuildProgram);
    DLSYM(GetDeviceInfo);
    DLSYM(GetProgramBuildInfo);
    DLSYM(GetProgramInfo);
    DLSYM(CreateProgramWithBinary);
    DLSYM(CreateProgramWithSource);

    return true;
}


cl_int OclLib::buildProgram(cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void *user_data)
{
    assert(pBuildProgram != nullptr);

    const cl_int ret = pBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, OclError::toString(ret), kBuildProgram);
    }

    return ret;
}


cl_int OclLib::getDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    assert(pGetDeviceInfo != nullptr);

    const cl_int ret = pGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, OclError::toString(ret), kGetDeviceInfo);
    }

    return ret;
}


cl_int OclLib::getDeviceInfoSilent(cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    assert(pGetDeviceInfo != nullptr);

    return pGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
}


cl_int OclLib::getProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    assert(pGetProgramBuildInfo != nullptr);

    const cl_int ret = pGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, OclError::toString(ret), kGetProgramBuildInfo);
    }

    return ret;
}


cl_int OclLib::getProgramInfo(cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    assert(pGetProgramInfo != nullptr);

    const cl_int ret = pGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, OclError::toString(ret), kGetProgramInfo);
    }

    return ret;
}


cl_program OclLib::createProgramWithBinary(cl_context context, cl_uint num_devices, const cl_device_id *device_list, const size_t *lengths, const unsigned char **binaries, cl_int *binary_status, cl_int *errcode_ret)
{
    assert(pCreateProgramWithBinary != nullptr);

    auto result = pCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, OclError::toString(*errcode_ret), kCreateProgramWithBinary);
    }

    return result;
}


cl_program OclLib::createProgramWithSource(cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret)
{
    assert(pCreateProgramWithSource != nullptr);

    auto result = pCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
    if (*errcode_ret != CL_SUCCESS) {
        LOG_ERR(kErrorTemplate, OclError::toString(*errcode_ret), kCreateProgramWithSource);
    }

    return result;
}
