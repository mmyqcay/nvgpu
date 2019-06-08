/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/safe_ops.h>
#include <nvgpu/gr/config.h>

#include "gr_config_gm20b.h"

#include <nvgpu/hw/gm20b/hw_gr_gm20b.h>

int gm20b_gr_config_init_sm_id_table(struct gk20a *g,
		struct nvgpu_gr_config *gr_config)
{
	u32 gpc, tpc;
	u32 sm_id = 0;

	for (tpc = 0;
	     tpc < nvgpu_gr_config_get_max_tpc_per_gpc_count(gr_config);
	     tpc++) {
		for (gpc = 0; gpc < nvgpu_gr_config_get_gpc_count(gr_config); gpc++) {

			if (tpc < nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc)) {
				struct nvgpu_sm_info *sm_info =
					nvgpu_gr_config_get_sm_info(gr_config, sm_id);
				nvgpu_gr_config_set_sm_info_tpc_index(sm_info, tpc);
				nvgpu_gr_config_set_sm_info_gpc_index(sm_info, gpc);
				nvgpu_gr_config_set_sm_info_sm_index(sm_info, 0);
				nvgpu_gr_config_set_sm_info_global_tpc_index(sm_info, sm_id);
				sm_id = nvgpu_safe_add_u32(sm_id, 1U);
			}
		}
	}
	nvgpu_gr_config_set_no_of_sm(gr_config, sm_id);
	return 0;
}

u32 gm20b_gr_config_get_gpc_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 val;
	u32 tpc_cnt = nvgpu_gr_config_get_max_tpc_per_gpc_count(config);

	/* Toggle the bits of NV_FUSE_STATUS_OPT_TPC_GPC */
	val = g->ops.fuse.fuse_status_opt_tpc_gpc(g, gpc_index);

	return (~val) & nvgpu_safe_sub_u32(BIT32(tpc_cnt), 1U);
}

u32 gm20b_gr_config_get_tpc_count_in_gpc(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	tmp1 = nvgpu_safe_mult_u32(gpc_stride, gpc_index);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_fs_gpc_r(), tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_fs_gpc_num_available_tpcs_v(tmp);
}

#ifdef NVGPU_GRAPHICS
u32 gm20b_gr_config_get_zcull_count_in_gpc(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	tmp1 = nvgpu_safe_mult_u32(gpc_stride, gpc_index);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_fs_gpc_r(), tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_fs_gpc_num_available_zculls_v(tmp);
}
#endif

u32 gm20b_gr_config_get_pes_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index, u32 pes_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	tmp1 = nvgpu_safe_mult_u32(gpc_index, gpc_stride);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_gpm_pd_pes_tpc_id_mask_r(pes_index),
					tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_gpm_pd_pes_tpc_id_mask_mask_v(tmp);
}

u32 gm20b_gr_config_get_pd_dist_skip_table_size(void)
{
	return gr_pd_dist_skip_table__size_1_v();
}

u32 gm20b_gr_config_get_gpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config)
{
	u32 val;
	u32 tpc_cnt = nvgpu_gr_config_get_max_gpc_count(config);

	/*
	 * For register NV_FUSE_STATUS_OPT_GPC a set bit with index i indicates
	 * corresponding GPC is floorswept
	 * But for s/w mask a set bit means GPC is enabled and it is disabled
	 * otherwise
	 * Hence toggle the bits of register value to get s/w mask
	 */
	val = g->ops.fuse.fuse_status_opt_gpc(g);

	return (~val) & nvgpu_safe_sub_u32(BIT32(tpc_cnt), 1U);
}
