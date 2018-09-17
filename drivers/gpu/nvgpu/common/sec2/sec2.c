/*
 * Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
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
#include <nvgpu/log.h>
#include <nvgpu/timers.h>
#include <nvgpu/sec2.h>
#include <nvgpu/sec2if/sec2_if_sec2.h>
#include <nvgpu/sec2if/sec2_if_cmn.h>

/* sec2 falcon queue init */
int nvgpu_sec2_queue_init(struct nvgpu_sec2 *sec2, u32 id,
	struct sec2_init_msg_sec2_init *init)
{
	struct gk20a *g = sec2->g;
	struct nvgpu_falcon_queue *queue = NULL;
	u32 queue_log_id = 0;
	u32 oflag = 0;
	int err = 0;

	if (id == SEC2_NV_CMDQ_LOG_ID) {
		/*
		 * set OFLAG_WRITE for command queue
		 * i.e, push from nvgpu &
		 * pop form falcon ucode
		 */
		oflag = OFLAG_WRITE;
	} else if (id == SEC2_NV_MSGQ_LOG_ID) {
		/*
		 * set OFLAG_READ for message queue
		 * i.e, push from falcon ucode &
		 * pop form nvgpu
		 */
		oflag = OFLAG_READ;
	} else {
		nvgpu_err(g, "invalid queue-id %d", id);
		err = -EINVAL;
		goto exit;
	}

	/* init queue parameters */
	queue_log_id = init->q_info[id].queue_log_id;

	queue = &sec2->queue[queue_log_id];
	queue->id = queue_log_id;
	queue->index = init->q_info[id].queue_phy_id;
	queue->offset = init->q_info[id].queue_offset;
	queue->position = init->q_info[id].queue_offset;
	queue->size = init->q_info[id].queue_size;
	queue->oflag = oflag;
	queue->queue_type = QUEUE_TYPE_EMEM;

	err = nvgpu_flcn_queue_init(sec2->flcn, queue);
	if (err != 0) {
		nvgpu_err(g, "queue-%d init failed", queue->id);
	}

exit:
	return err;
}

static void sec2_seq_init(struct nvgpu_sec2 *sec2)
{
	u32 i = 0;

	nvgpu_log_fn(sec2->g, " ");

	memset(sec2->seq, 0,
		sizeof(struct sec2_sequence) * SEC2_MAX_NUM_SEQUENCES);

	memset(sec2->sec2_seq_tbl, 0, sizeof(sec2->sec2_seq_tbl));

	for (i = 0; i < SEC2_MAX_NUM_SEQUENCES; i++) {
		sec2->seq[i].id = (u8)i;
	}
}

static void nvgpu_remove_sec2_support(struct nvgpu_sec2 *sec2)
{
	struct gk20a *g = sec2->g;

	nvgpu_log_fn(g, " ");

	nvgpu_kfree(g, sec2->seq);
	nvgpu_mutex_destroy(&sec2->sec2_seq_lock);
	nvgpu_mutex_destroy(&sec2->isr_mutex);
}

static int nvgpu_init_sec2_setup_sw(struct gk20a *g, struct nvgpu_sec2 *sec2)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	sec2->seq = nvgpu_kzalloc(g, SEC2_MAX_NUM_SEQUENCES *
		sizeof(struct sec2_sequence));
	if (sec2->seq == NULL) {
		err = -ENOMEM;
		goto exit;
	}

	err = nvgpu_mutex_init(&sec2->sec2_seq_lock);
	if (err != 0) {
		goto free_seq_alloc;
	}

	sec2_seq_init(sec2);

	err = nvgpu_mutex_init(&sec2->isr_mutex);
	if (err != 0) {
		goto free_seq_mutex;
	}

	sec2->remove_support = nvgpu_remove_sec2_support;

	goto exit;

free_seq_mutex:
	nvgpu_mutex_destroy(&sec2->sec2_seq_lock);
free_seq_alloc:
	nvgpu_kfree(g, sec2->seq);

exit:
	return err;
}

int nvgpu_init_sec2_support(struct gk20a *g)
{
	struct nvgpu_sec2 *sec2 = &g->sec2;
	int err = 0;

	nvgpu_log_fn(g, " ");

	err = nvgpu_init_sec2_setup_sw(g, sec2);
	if (err != 0) {
		goto exit;
	}

	/* Enable irq*/
	nvgpu_mutex_acquire(&sec2->isr_mutex);
	g->ops.sec2.enable_irq(sec2, true);
	sec2->isr_enabled = true;
	nvgpu_mutex_release(&sec2->isr_mutex);

	/* TBD - call SEC2 in secure mode to boot RTOS */

exit:
	return err;
}

int nvgpu_sec2_destroy(struct gk20a *g)
{
	struct nvgpu_sec2 *sec2 = &g->sec2;
	u32 i = 0;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&sec2->isr_mutex);
	sec2->isr_enabled = false;
	nvgpu_mutex_release(&sec2->isr_mutex);

	for (i = 0; i < SEC2_QUEUE_NUM; i++) {
		nvgpu_flcn_queue_free(sec2->flcn, &sec2->queue[i]);
	}

	sec2->sec2_ready = false;

	return 0;
}
