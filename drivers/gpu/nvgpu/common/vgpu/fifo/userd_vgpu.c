/*
 * Virtualized GPU USERD
 *
 * Copyright (c) 2014-2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifdef CONFIG_NVGPU_TRACE
#include <trace/events/gk20a.h>
#endif

#include <nvgpu/gk20a.h>
#include <nvgpu/fifo/userd.h>

#include "userd_vgpu.h"

int vgpu_userd_setup_sw(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_USERD
	struct nvgpu_fifo *f = &g->fifo;

	f->userd_entry_size = g->ops.userd.entry_size(g);

	return nvgpu_userd_init_slabs(g);
#else
	return 0;
#endif
}

void vgpu_userd_cleanup_sw(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_USERD
	nvgpu_userd_free_slabs(g);
#endif
}
