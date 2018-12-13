/*
 * Copyright (c) 2016-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef NVGPU_PERF_H
#define NVGPU_PERF_H

#include <nvgpu/pmu/pstate.h>
#include <nvgpu/pmuif/nvgpu_gpmu_cmdif.h>

#include "vfe_equ.h"
#include "vfe_var.h"
#include "volt/volt.h"
#include "lpwr/lpwr.h"
#include "change_seq.h"

#define CTRL_PERF_VFE_VAR_TYPE_INVALID                               0x00U
#define CTRL_PERF_VFE_VAR_TYPE_DERIVED                               0x01U
#define CTRL_PERF_VFE_VAR_TYPE_DERIVED_PRODUCT                       0x02U
#define CTRL_PERF_VFE_VAR_TYPE_DERIVED_SUM                           0x03U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE                                0x04U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_FREQUENCY                      0x05U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED                         0x06U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE                    0x07U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP                    0x08U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_VOLTAGE                        0x09U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_CALLER_SPECIFIED               0x0AU

#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_NONE                  0x00U
#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_VALUE                 0x01U
#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_OFFSET                0x02U
#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_SCALE                 0x03U

#define CTRL_PERF_VFE_EQU_TYPE_INVALID                               0x00U
#define CTRL_PERF_VFE_EQU_TYPE_COMPARE                               0x01U
#define CTRL_PERF_VFE_EQU_TYPE_MINMAX                                0x02U
#define CTRL_PERF_VFE_EQU_TYPE_QUADRATIC                             0x03U
#define CTRL_PERF_VFE_EQU_TYPE_SCALAR                                0x04U

#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_UNITLESS                       0x00U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_FREQ_MHZ                       0x01U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VOLT_UV                        0x02U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VF_GAIN                        0x03U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VOLT_DELTA_UV                  0x04U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_WORK_TYPE                      0x06U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_UTIL_RATIO                     0x07U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_WORK_FB_NORM                   0x08U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_POWER_MW                       0x09U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_PWR_OVER_UTIL_SLOPE            0x0AU
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VIN_CODE                       0x0BU

#define CTRL_PERF_VFE_EQU_QUADRATIC_COEFF_COUNT                      0x03U

#define CTRL_PERF_VFE_EQU_COMPARE_FUNCTION_EQUAL                     0x00U
#define CTRL_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER_EQ                0x01U
#define CTRL_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER                   0x02U

struct gk20a;

struct nvgpu_vfe_invalidate {
	bool state_change;
	struct nvgpu_cond wq;
	struct nvgpu_thread state_task;
};

struct perf_pmupstate {
	struct vfe_vars vfe_varobjs;
	struct vfe_equs vfe_equobjs;
	struct pstates pstatesobjs;
	struct obj_volt volt;
	struct obj_lwpr lpwr;
	struct nvgpu_vfe_invalidate vfe_init;
	struct change_seq_pmu changeseq_pmu;
};

int perf_pmu_init_pmupstate(struct gk20a *g);
void perf_pmu_free_pmupstate(struct gk20a *g);
int perf_pmu_vfe_load(struct gk20a *g);

#endif /* NVGPU_PERF_H */
