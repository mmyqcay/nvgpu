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

#ifndef NVGPU_LOG_COMMON_H
#define NVGPU_LOG_COMMON_H

enum nvgpu_log_type {
	NVGPU_ERROR = 0,
	NVGPU_WARNING,
	NVGPU_DEBUG,
	NVGPU_INFO,
};

/*
 * Use this define to set a default mask.
 */
#define NVGPU_DEFAULT_DBG_MASK		(0)

#define	gpu_dbg_info 		BIT(0) 	/* Lightly verbose info. */
#define	gpu_dbg_fn		BIT(1) 	/* Function name tracing. */
#define	gpu_dbg_reg		BIT(2) 	/* Register accesses; very verbose. */
#define	gpu_dbg_pte		BIT(3) 	/* GMMU PTEs. */
#define	gpu_dbg_intr		BIT(4) 	/* Interrupts. */
#define	gpu_dbg_pmu		BIT(5) 	/* gk20a pmu. */
#define	gpu_dbg_clk		BIT(6) 	/* gk20a clk. */
#define	gpu_dbg_map		BIT(7) 	/* Memory mappings. */
#define	gpu_dbg_map_v		BIT(8) 	/* Verbose mem mappings. */
#define	gpu_dbg_gpu_dbg		BIT(9) 	/* GPU debugger/profiler. */
#define	gpu_dbg_cde		BIT(10) /* cde info messages. */
#define	gpu_dbg_cde_ctx		BIT(11) /* cde context usage messages. */
#define	gpu_dbg_ctxsw		BIT(12) /* ctxsw tracing. */
#define	gpu_dbg_sched		BIT(13) /* Sched control tracing. */
#define	gpu_dbg_sema		BIT(14) /* Semaphore debugging. */
#define	gpu_dbg_sema_v		BIT(15) /* Verbose semaphore debugging. */
#define	gpu_dbg_pmu_pstate	BIT(16) /* p state controlled by pmu. */
#define	gpu_dbg_xv		BIT(17) /* XVE debugging. */
#define	gpu_dbg_shutdown	BIT(18) /* GPU shutdown tracing. */
#define	gpu_dbg_kmem		BIT(19) /* Kmem tracking debugging. */
#define	gpu_dbg_pd_cache	BIT(20) /* PD cache traces. */
#define	gpu_dbg_alloc		BIT(21) /* Allocator debugging. */
#define	gpu_dbg_dma		BIT(22) /* DMA allocation prints. */
#define	gpu_dbg_sgl		BIT(23) /* SGL related traces. */
#ifdef CONFIG_NVGPU_DGPU
#define	gpu_dbg_vidmem		BIT(24) /* VIDMEM tracing. */
#endif
#define	gpu_dbg_nvlink		BIT(25) /* nvlink Operation tracing. */
#define	gpu_dbg_clk_arb		BIT(26) /* Clk arbiter debugging. */
#define	gpu_dbg_event		BIT(27) /* Events to User debugging. */
#define	gpu_dbg_vsrv		BIT(28) /* server debugging. */
#define	gpu_dbg_mem		BIT(31) /* memory accesses; very verbose. */

#endif