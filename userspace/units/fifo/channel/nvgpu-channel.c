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
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <unit/io.h>
#include <unit/unit.h>

#include <nvgpu/channel.h>
#include <nvgpu/engines.h>
#include <nvgpu/tsg.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>

#include <nvgpu/posix/posix-fault-injection.h>

#include "../nvgpu-fifo.h"

#define MAX_STUB	2

struct stub_ctx {
	u32 chid;
};

struct stub_ctx stub[MAX_STUB];

struct channel_unit_ctx {
	u32 branches;
	struct stub_ctx stub[MAX_STUB];
};

static struct channel_unit_ctx unit_ctx;

static void subtest_setup(u32 branches)
{
	u32 i;

	unit_ctx.branches = branches;
	memset(stub, 0, sizeof(stub));
	for (i = 0; i < MAX_STUB; i++) {
		stub[i].chid = NVGPU_INVALID_CHANNEL_ID;
	}
}

#define subtest_pruned	test_fifo_subtest_pruned
#define branches_str	test_fifo_flags_str

#define assert(cond)	unit_assert(cond, goto done)

#define F_CHANNEL_SETUP_SW_VZALLOC_FAIL				BIT(0)
#define F_CHANNEL_SETUP_SW_LAST					BIT(1)

/* TODO: nvgpu_cond_init failure, not testable yet */
#define F_CHANNEL_SETUP_SW_INIT_SUPPORT_FAIL_COND_INIT

static const char *f_channel_setup_sw[] = {
	"vzalloc_fail",
};

static u32 stub_channel_count(struct gk20a *g)
{
	return 32;
}

static int test_channel_setup_sw(struct unit_module *m,
		struct gk20a *g, void *args)
{
	struct gpu_ops gops = g->ops;
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_posix_fault_inj *kmem_fi;
	u32 branches;
	int rc = UNIT_FAIL;
	int err;
	u32 fail = F_CHANNEL_SETUP_SW_VZALLOC_FAIL;

	u32 prune = fail;

	kmem_fi = nvgpu_kmem_get_fault_injection();

	g->ops.channel.count = stub_channel_count;

	for (branches = 0U; branches < F_CHANNEL_SETUP_SW_LAST; branches++) {

		if (subtest_pruned(branches, prune)) {
			unit_verbose(m, "%s branches=%s (pruned)\n",
				__func__,
				branches_str(branches, f_channel_setup_sw));
			continue;
		}
		subtest_setup(branches);

		nvgpu_posix_enable_fault_injection(kmem_fi,
			branches & F_CHANNEL_SETUP_SW_VZALLOC_FAIL ?
			true : false, 0);

		unit_verbose(m, "%s branches=%s\n", __func__,
			branches_str(branches, f_channel_setup_sw));

		err = nvgpu_channel_setup_sw(g);

		if (branches & fail) {
			assert(err != 0);
			assert(f->channel == NULL);
		} else {
			assert(err == 0);
			nvgpu_channel_cleanup_sw(g);
		}
	}

	rc = UNIT_SUCCESS;
done:
	nvgpu_posix_enable_fault_injection(kmem_fi, false, 0);
	if (rc != UNIT_SUCCESS) {
		unit_err(m, "%s branches=%s\n", __func__,
			branches_str(branches, f_channel_setup_sw));
	}
	g->ops = gops;
	return rc;
}

#define F_CHANNEL_OPEN_ENGINE_NOT_VALID		BIT(0)
#define F_CHANNEL_OPEN_PRIVILEGED		BIT(1)
#define F_CHANNEL_OPEN_ALLOC_CH_FAIL		BIT(2)
#define F_CHANNEL_OPEN_ALLOC_CH_WARN0		BIT(3)
#define F_CHANNEL_OPEN_ALLOC_CH_WARN1		BIT(4)
#define F_CHANNEL_OPEN_ALLOC_CH_AGGRESSIVE	BIT(5)
#define F_CHANNEL_OPEN_BUG_ON			BIT(6)
#define F_CHANNEL_OPEN_ALLOC_INST_FAIL		BIT(7)
#define F_CHANNEL_OPEN_OS			BIT(8)
#define F_CHANNEL_OPEN_LAST			BIT(9)


/* TODO: cover nvgpu_cond_init failures */
#define F_CHANNEL_OPEN_COND0_INIT_FAIL
#define F_CHANNEL_OPEN_COND1_INIT_FAIL

static const char *f_channel_open[] = {
	"engine_not_valid",
	"privileged",
	"alloc_ch_fail",
	"alloc_ch_warn0",
	"alloc_ch_warn1",
	"aggressive_destroy",
	"bug_on",
	"alloc_inst_fail",
	"cond0_init_fail",
	"cond1_init_fail",
	"hal",
};

static int stub_channel_alloc_inst_ENOMEM(struct gk20a *g,
		struct nvgpu_channel *ch)
{
	return -ENOMEM;
}

static int test_channel_open(struct unit_module *m,
		struct gk20a *g, void *args)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_fifo fifo = g->fifo;
	struct gpu_ops gops = g->ops;
	struct nvgpu_channel *ch, *next_ch;
	struct nvgpu_posix_fault_inj *kmem_fi;
	u32 branches;
	int rc = UNIT_FAIL;
	u32 fail =
		F_CHANNEL_OPEN_ALLOC_CH_FAIL |
		F_CHANNEL_OPEN_BUG_ON |
		F_CHANNEL_OPEN_ALLOC_INST_FAIL;
	u32 prune = fail |
		F_CHANNEL_OPEN_ALLOC_CH_WARN0 |
		F_CHANNEL_OPEN_ALLOC_CH_WARN1;
	u32 runlist_id;
	bool privileged;
	int err;
	void (*os_channel_open)(struct nvgpu_channel *ch) =
		g->os_channel.open;

	kmem_fi = nvgpu_kmem_get_fault_injection();

	for (branches = 0U; branches < F_CHANNEL_OPEN_LAST; branches++) {

		if (subtest_pruned(branches, prune)) {
			unit_verbose(m, "%s branches=%s (pruned)\n", __func__,
				branches_str(branches, f_channel_open));
			continue;
		}
		subtest_setup(branches);
		unit_verbose(m, "%s branches=%s\n", __func__,
			branches_str(branches, f_channel_open));

		nvgpu_posix_enable_fault_injection(kmem_fi, false, 0);

		next_ch =
			nvgpu_list_empty(&f->free_chs) ? NULL :
			nvgpu_list_first_entry(&f->free_chs,
				nvgpu_channel, free_chs);
		assert(next_ch != NULL);

		runlist_id =
			branches & F_CHANNEL_OPEN_ENGINE_NOT_VALID ?
			NVGPU_INVALID_RUNLIST_ID :
			NVGPU_ENGINE_GR;

		privileged =
			branches & F_CHANNEL_OPEN_PRIVILEGED ?
			true : false;

		if (branches & F_CHANNEL_OPEN_ALLOC_CH_FAIL) {
			nvgpu_init_list_node(&f->free_chs);
		}

		if (branches & F_CHANNEL_OPEN_ALLOC_CH_WARN0) {
			nvgpu_atomic_inc(&next_ch->ref_count);
		}

		if (branches & F_CHANNEL_OPEN_ALLOC_CH_WARN1) {
			next_ch->referenceable = false;
		}

		if (branches & F_CHANNEL_OPEN_ALLOC_CH_AGGRESSIVE) {
			g->aggressive_sync_destroy_thresh += 1U;
			f->used_channels += 2U;
		}

		g->ops.channel.alloc_inst =
			branches & F_CHANNEL_OPEN_ALLOC_INST_FAIL ?
			stub_channel_alloc_inst_ENOMEM :
			gops.channel.alloc_inst;

		if (branches & F_CHANNEL_OPEN_BUG_ON) {
			next_ch->g = (void *)1;
		}

		err = EXPECT_BUG(
			ch = gk20a_open_new_channel(g, runlist_id,
				privileged, getpid(), getpid());
		);

		if (branches & F_CHANNEL_OPEN_BUG_ON) {
			next_ch->g = NULL;
			assert(err != 0);
		} else {
			assert(err == 0);
		};

		if (branches & F_CHANNEL_OPEN_ALLOC_CH_WARN1) {
			next_ch->referenceable = true;
		}

		if (branches & F_CHANNEL_OPEN_ALLOC_CH_AGGRESSIVE) {
			g->aggressive_sync_destroy_thresh -= 1U;
			f->used_channels -= 2U;
			assert(g->aggressive_sync_destroy);
			g->aggressive_sync_destroy = false;
		}

		if (branches & fail) {
			if (branches & F_CHANNEL_OPEN_ALLOC_CH_FAIL) {
				f->free_chs = fifo.free_chs;
			}

			if (branches & F_CHANNEL_OPEN_ALLOC_CH_WARN0) {
				nvgpu_atomic_dec(&ch->ref_count);
			}
			assert(ch == NULL);
		} else {
			assert(ch != NULL);
			assert(ch->g == g);
			assert(nvgpu_list_empty(&ch->free_chs));

			nvgpu_channel_close(ch);
			ch = NULL;
		}
	}
	rc = UNIT_SUCCESS;

done:
	if (rc != UNIT_SUCCESS) {
		unit_err(m, "%s branches=%s\n", __func__,
			branches_str(branches, f_channel_open));
	}
	nvgpu_posix_enable_fault_injection(kmem_fi, false, 0);
	if (ch != NULL) {
		nvgpu_channel_close(ch);
	}
	g->ops = gops;
	g->os_channel.open = os_channel_open;
	return rc;
}

struct unit_module_test nvgpu_channel_tests[] = {
	UNIT_TEST(setup_sw, test_channel_setup_sw, &unit_ctx, 0),
	UNIT_TEST(init_support, test_fifo_init_support, &unit_ctx, 0),
	UNIT_TEST(open, test_channel_open, &unit_ctx, 0),
	UNIT_TEST(remove_support, test_fifo_remove_support, &unit_ctx, 0),
};

UNIT_MODULE(nvgpu_channel, nvgpu_channel_tests, UNIT_PRIO_NVGPU_TEST);
