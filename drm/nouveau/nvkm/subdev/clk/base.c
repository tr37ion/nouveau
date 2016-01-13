/*
 * Copyright 2013 Red Hat Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */
#include "priv.h"

#include <subdev/bios.h>
#include <subdev/bios/boost.h>
#include <subdev/bios/cstep.h>
#include <subdev/bios/perf.h>
#include <subdev/fb.h>
#include <subdev/pmu.h>
#include <subdev/pmu/fuc/os.h>
#include <subdev/therm.h>
#include <subdev/volt.h>

#include <core/option.h>

/******************************************************************************
 * misc
 *****************************************************************************/
static u32
nvkm_clk_adjust(struct nvkm_clk *clk, bool adjust,
		u8 pstate, u8 domain, u32 input)
{
	struct nvkm_bios *bios = clk->subdev.device->bios;
	struct nvbios_boostE boostE;
	u8  ver, hdr, cnt, len;
	u16 data;

	data = nvbios_boostEm(bios, pstate, &ver, &hdr, &cnt, &len, &boostE);
	if (data) {
		struct nvbios_boostS boostS;
		u8  idx = 0, sver, shdr;
		u16 subd;

		input = max(boostE.min, input);
		input = min(boostE.max, input);
		do {
			sver = ver;
			shdr = hdr;
			subd = nvbios_boostSp(bios, idx++, data, &sver, &shdr,
					      cnt, len, &boostS);
			if (subd && boostS.domain == domain) {
				if (adjust)
					input = input * boostS.percent / 100;
				input = max(boostS.min, input);
				input = min(boostS.max, input);
				break;
			}
		} while (subd);
	}

	return input;
}

/******************************************************************************
 * C-States
 *****************************************************************************/
int
nvkm_cstate_prog(struct nvkm_clk *clk, struct nvkm_pstate *pstate, int cstatei)
{
	struct nvkm_subdev *subdev = &clk->subdev;
	struct nvkm_device *device = subdev->device;
	struct nvkm_therm *therm = device->therm;
	struct nvkm_volt *volt = device->volt;
	struct nvkm_cstate *cstate;
	int ret;

	if (!clk->allow_eng_reclock)
		return 0;

	if (!list_empty(&pstate->list)) {
		if (cstatei == -1)
			cstate = list_entry(pstate->list.prev, typeof(*cstate), head);
		else {
			list_for_each_entry(cstate, &pstate->list, head) {
				if (cstate->cstate == cstatei)
					break;
			}
		}
	} else {
		cstate = &pstate->base;
	}

	if (therm) {
		ret = nvkm_therm_cstate(therm, pstate->fanspeed, +1);
		if (ret && ret != -ENODEV) {
			nvkm_error(subdev, "failed to raise fan speed: %d\n", ret);
			return ret;
		}
	}

	if (volt) {
		ret = nvkm_volt_set_id(volt, cstate->voltage, +1);
		if (ret && ret != -ENODEV) {
			nvkm_error(subdev, "failed to raise voltage: %d\n", ret);
			return ret;
		}
	}

	ret = clk->func->calc(clk, cstate);
	if (ret == 0) {
		ret = clk->func->prog(clk);
		clk->func->tidy(clk);
	}

	if (volt) {
		ret = nvkm_volt_set_id(volt, cstate->voltage, -1);
		if (ret && ret != -ENODEV)
			nvkm_error(subdev, "failed to lower voltage: %d\n", ret);
	}

	if (therm) {
		ret = nvkm_therm_cstate(therm, pstate->fanspeed, -1);
		if (ret && ret != -ENODEV)
			nvkm_error(subdev, "failed to lower fan speed: %d\n", ret);
	}

	return ret;
}

static void
nvkm_cstate_del(struct nvkm_cstate *cstate)
{
	list_del(&cstate->head);
	kfree(cstate);
}

static int
nvkm_cstate_new(struct nvkm_clk *clk, int idx, struct nvkm_pstate *pstate)
{
	struct nvkm_bios *bios = clk->subdev.device->bios;
	const struct nvkm_domain *domain = clk->domains;
	struct nvkm_cstate *cstate = NULL;
	struct nvbios_cstepX cstepX;
	u8  ver, hdr;
	u16 data;

	data = nvbios_cstepXp(bios, idx, &ver, &hdr, &cstepX);
	if (!data)
		return -ENOENT;

	cstate = kzalloc(sizeof(*cstate), GFP_KERNEL);
	if (!cstate)
		return -ENOMEM;

	*cstate = pstate->base;
	cstate->voltage = cstepX.voltage;
	cstate->cstate = idx;

	while (domain && domain->name != nv_clk_src_max) {
		if (domain->flags & NVKM_CLK_DOM_FLAG_CORE) {
			u32 freq = nvkm_clk_adjust(clk, true, pstate->pstate,
						   domain->bios, cstepX.freq);
			cstate->domain[domain->name] = freq;
		}
		domain++;
	}

	list_add(&cstate->head, &pstate->list);
	return 0;
}

/******************************************************************************
 * P-States
 *****************************************************************************/
static int
nvkm_pstate_prog(struct nvkm_clk *clk, int pstatei)
{
	struct nvkm_subdev *subdev = &clk->subdev;
	struct nvkm_ram *ram = subdev->device->fb->ram;
	struct nvkm_pci *pci = subdev->device->pci;
	struct nvkm_pstate *pstate;
	int ret, idx = 0;

	list_for_each_entry(pstate, &clk->states, head) {
		if (idx++ == pstatei)
			break;
	}

	nvkm_debug(subdev, "setting performance state %d\n", pstatei);
	clk->pstate = pstatei;

	nvkm_pcie_set_link(pci, pstate->pcie_speed, pstate->pcie_width);

	if (clk->allow_mem_reclock && ram && ram->func->calc) {
		int khz = pstate->base.domain[nv_clk_src_mem];
		do {
			ret = ram->func->calc(ram, khz);
			if (ret == 0)
				ret = ram->func->prog(ram);
		} while (ret > 0);
		ram->func->tidy(ram);
	}

	return nvkm_cstate_prog(clk, pstate, clk->ucstate);
}

static void
nvkm_pstate_work(struct work_struct *work)
{
	struct nvkm_clk *clk = container_of(work, typeof(*clk), work);
	struct nvkm_subdev *subdev = &clk->subdev;
	int pstate;

	if (!atomic_xchg(&clk->waiting, 0))
		return;
	clk->pwrsrc = power_supply_is_system_supplied();

	nvkm_trace(subdev, "P %d PWR %d U(AC) %d U(DC) %d A %d T %d D %d\n",
		   clk->pstate, clk->pwrsrc, clk->ustate_ac, clk->ustate_dc,
		   clk->astate, clk->tstate, clk->dstate);

	pstate = clk->pwrsrc ? clk->ustate_ac : clk->ustate_dc;
	if (clk->state_nr && pstate != -1) {
		pstate = (pstate < 0) ? clk->astate : pstate;
		pstate = min(pstate, clk->state_nr - 1 + clk->tstate);
		pstate = max(pstate, clk->dstate);
	} else {
		pstate = clk->pstate = -1;
	}

	nvkm_trace(subdev, "-> %d\n", pstate);
	if (pstate != clk->pstate) {
		int ret = nvkm_pstate_prog(clk, pstate);
		if (ret) {
			nvkm_error(subdev, "error setting pstate %d: %d\n",
				   pstate, ret);
		}
	}

	wake_up_all(&clk->wait);
	nvkm_notify_get(&clk->pwrsrc_ntfy);
}

static int
nvkm_pstate_calc(struct nvkm_clk *clk, bool wait)
{
	atomic_set(&clk->waiting, 1);
	schedule_work(&clk->work);
	if (wait)
		wait_event(clk->wait, !atomic_read(&clk->waiting));
	return 0;
}

static void
nvkm_pstate_info(struct nvkm_clk *clk, struct nvkm_pstate *pstate)
{
	const struct nvkm_domain *clock = clk->domains - 1;
	struct nvkm_cstate *cstate;
	struct nvkm_subdev *subdev = &clk->subdev;
	char info[3][32] = { "", "", "" };
	char name[4] = "--";
	int i = -1;

	if (pstate->pstate != 0xff)
		snprintf(name, sizeof(name), "%02x", pstate->pstate);

	while ((++clock)->name != nv_clk_src_max) {
		u32 lo = pstate->base.domain[clock->name];
		u32 hi = lo;
		if (hi == 0)
			continue;

		nvkm_debug(subdev, "%02x: %10d KHz\n", clock->name, lo);
		list_for_each_entry(cstate, &pstate->list, head) {
			u32 freq = cstate->domain[clock->name];
			lo = min(lo, freq);
			hi = max(hi, freq);
			nvkm_debug(subdev, "%10d KHz\n", freq);
		}

		if (clock->mname && ++i < ARRAY_SIZE(info)) {
			lo /= clock->mdiv;
			hi /= clock->mdiv;
			if (lo == hi) {
				snprintf(info[i], sizeof(info[i]), "%s %d MHz",
					 clock->mname, lo);
			} else {
				snprintf(info[i], sizeof(info[i]),
					 "%s %d-%d MHz", clock->mname, lo, hi);
			}
		}
	}

	nvkm_debug(subdev, "%s: %s %s %s\n", name, info[0], info[1], info[2]);
}

static void
nvkm_pstate_del(struct nvkm_pstate *pstate)
{
	struct nvkm_cstate *cstate, *temp;

	list_for_each_entry_safe(cstate, temp, &pstate->list, head) {
		nvkm_cstate_del(cstate);
	}

	list_del(&pstate->head);
	kfree(pstate);
}

static int
nvkm_pstate_new(struct nvkm_clk *clk, int idx)
{
	struct nvkm_bios *bios = clk->subdev.device->bios;
	const struct nvkm_domain *domain = clk->domains - 1;
	struct nvkm_pstate *pstate;
	struct nvkm_cstate *cstate;
	struct nvbios_cstepE cstepE;
	struct nvbios_perfE perfE;
	u8  ver, hdr, cnt, len;
	u16 data;

	data = nvbios_perfEp(bios, idx, &ver, &hdr, &cnt, &len, &perfE);
	if (!data)
		return -EINVAL;
	if (perfE.pstate == 0xff)
		return 0;

	pstate = kzalloc(sizeof(*pstate), GFP_KERNEL);
	cstate = &pstate->base;
	if (!pstate)
		return -ENOMEM;

	INIT_LIST_HEAD(&pstate->list);

	pstate->pstate = perfE.pstate;
	pstate->fanspeed = perfE.fanspeed;
	pstate->pcie_speed = perfE.pcie_speed;
	pstate->pcie_width = perfE.pcie_width;
	cstate->voltage = perfE.voltage;
	cstate->domain[nv_clk_src_core] = perfE.core;
	cstate->domain[nv_clk_src_shader] = perfE.shader;
	cstate->domain[nv_clk_src_mem] = perfE.memory;
	cstate->domain[nv_clk_src_vdec] = perfE.vdec;
	cstate->domain[nv_clk_src_dom6] = perfE.disp;

	while (ver >= 0x40 && (++domain)->name != nv_clk_src_max) {
		struct nvbios_perfS perfS;
		u8  sver = ver, shdr = hdr;
		u32 perfSe = nvbios_perfSp(bios, data, domain->bios,
					  &sver, &shdr, cnt, len, &perfS);
		if (perfSe == 0 || sver != 0x40)
			continue;

		if (domain->flags & NVKM_CLK_DOM_FLAG_CORE) {
			perfS.v40.freq = nvkm_clk_adjust(clk, false,
							 pstate->pstate,
							 domain->bios,
							 perfS.v40.freq);
		}

		cstate->domain[domain->name] = perfS.v40.freq;
	}

	data = nvbios_cstepEm(bios, pstate->pstate, &ver, &hdr, &cstepE);
	if (data) {
		int idx = cstepE.index;
		do {
			nvkm_cstate_new(clk, idx, pstate);
		} while(idx--);
	}

	nvkm_pstate_info(clk, pstate);
	list_add_tail(&pstate->head, &clk->states);
	clk->state_nr++;
	return 0;
}

/******************************************************************************
 * Adjustment triggers
 *****************************************************************************/
static int
nvkm_clk_ustate_update(struct nvkm_clk *clk, int req)
{
	struct nvkm_pstate *pstate;
	int i = 0;

	if (!(clk->allow_eng_reclock || clk->allow_mem_reclock))
		return -ENOSYS;

	if (req != -1 && req != -2) {
		list_for_each_entry(pstate, &clk->states, head) {
			if (pstate->pstate == req)
				break;
			i++;
		}

		if (pstate->pstate != req)
			return -EINVAL;
		req = i;
	}

	return req + 2;
}

static int
nvkm_clk_nstate(struct nvkm_clk *clk, const char *mode, int arglen)
{
	int ret = 1;

	if (clk->allow_eng_reclock && !strncasecmpz(mode, "auto", arglen))
		return -2;

	if (strncasecmpz(mode, "disabled", arglen)) {
		char save = mode[arglen];
		long v;

		((char *)mode)[arglen] = '\0';
		if (!kstrtol(mode, 0, &v)) {
			ret = nvkm_clk_ustate_update(clk, v);
			if (ret < 0)
				ret = 1;
		}
		((char *)mode)[arglen] = save;
	}

	return ret - 2;
}

int
nvkm_clk_ustate(struct nvkm_clk *clk, int req, int pwr)
{
	int ret = nvkm_clk_ustate_update(clk, req);
	if (ret >= 0) {
		if (ret -= 2, pwr) clk->ustate_ac = ret;
		else		   clk->ustate_dc = ret;
		return nvkm_pstate_calc(clk, true);
	}
	return ret;
}

int
nvkm_clk_astate(struct nvkm_clk *clk, int req, int rel, bool wait)
{
	if (!rel) clk->astate  = req;
	if ( rel) clk->astate += rel;
	clk->astate = min(clk->astate, clk->state_nr - 1);
	clk->astate = max(clk->astate, 0);
	return nvkm_pstate_calc(clk, wait);
}

int
nvkm_clk_tstate(struct nvkm_clk *clk, int req, int rel)
{
	if (!rel) clk->tstate  = req;
	if ( rel) clk->tstate += rel;
	clk->tstate = min(clk->tstate, 0);
	clk->tstate = max(clk->tstate, -(clk->state_nr - 1));
	return nvkm_pstate_calc(clk, true);
}

int
nvkm_clk_dstate(struct nvkm_clk *clk, int req, int rel)
{
	if (!rel) clk->dstate  = req;
	if ( rel) clk->dstate += rel;
	clk->dstate = min(clk->dstate, clk->state_nr - 1);
	clk->dstate = max(clk->dstate, 0);
	return nvkm_pstate_calc(clk, true);
}

static int
nvkm_clk_pwrsrc(struct nvkm_notify *notify)
{
	struct nvkm_clk *clk =
		container_of(notify, typeof(*clk), pwrsrc_ntfy);
	nvkm_pstate_calc(clk, false);
	return NVKM_NOTIFY_DROP;
}

static int is_state_better(u8 cur_load, u8 target_load, int cur, int check, int margin)
{
	int needed_change = (cur_load * 0xff) / (target_load + margin * (cur_load > target_load ? 1 : -1));
	int speed_change = (check * 0xff) / cur;
	return speed_change - needed_change;
}

static int pcie_enum_to_speed[] = {
	25,
	50,
	80
};

#define __tmp(i, score, pstate) if ((last_pstate_scores[i] < 0 && last_pstate_scores[i] < pcie_score) || (last_pstate_scores[i] >= 0 && last_pstate_scores[i] > pcie_score && pcie_score > 0)) { \
	last_pstate[i] = pstate; \
	last_pstate_scores[i] = score; \
}


static int calc_needed_pstate(struct nvkm_clk *clk, u8 mem_load, u8 vid_load, u8 pcie_load)
{
	struct nvkm_subdev *subdev = &clk->subdev;
	struct nvkm_pstate *pstate;
	struct nvkm_pstate *old = NULL;
	int i = 0;
	int last_pstate[3] = { 0 };
	int last_pstate_scores[3] = { 0 };

	list_for_each_entry(pstate, &clk->states, head) {
		if (i == clk->pstate)
			old = pstate;
		++i;
	}
	i = 0;

	if (old == NULL)
		return -EINVAL;

	list_for_each_entry(pstate, &clk->states, head) {
		/* pcie needs a much lower target */
		int pcie_score = is_state_better(pcie_load, 0x5f, pcie_enum_to_speed[old->pcie_speed], pcie_enum_to_speed[pstate->pcie_speed], PERF_TARGET_SAFETY * 0.5);
		int mem_score = is_state_better(mem_load, PERF_TARGET_LOAD, old->base.domain[nv_clk_src_mem], pstate->base.domain[nv_clk_src_mem], PERF_TARGET_SAFETY);

		nvkm_trace(subdev, "dyn reclock pcie score: %i for pstate %i and speed: %i\n", pcie_score, i, pcie_enum_to_speed[pstate->pcie_speed]);
		nvkm_trace(subdev, "dyn reclock mem  score: %i for pstate %i and speed: %i\n", mem_score, i, pstate->base.domain[nv_clk_src_mem]);

		if (i == 0) {
			last_pstate[0] = i;
			last_pstate[1] = i;
			last_pstate_scores[0] = pcie_score;
			last_pstate_scores[1] = mem_score;
		} else {
			__tmp(0, pcie_score, i);
			__tmp(1, mem_score, i);
		}

		i += 1;
	}

	return max(last_pstate[0], last_pstate[1]);
}

static int calc_needed_cstate(struct nvkm_clk *clk, u8 core_load, int *ps)
{
	struct nvkm_subdev *subdev = &clk->subdev;
	struct nvkm_pstate *pstate = NULL;
	struct nvkm_cstate *cstate = NULL;
	struct nvkm_cstate *old = NULL;
	int i = 0, last_score = 0, last_cstate = 0, last_pstate = 0;

	list_for_each_entry(pstate, &clk->states, head) {
		if (i == clk->pstate) {
			list_for_each_entry(cstate, &pstate->list, head) {
				if (cstate->cstate == clk->ucstate)
					old = cstate;
			}
		}
		i += 1;
	}
	i = 0;

	if (pstate == NULL)
		return -EINVAL;

	if (old == NULL)
		old = &pstate->base;

	list_for_each_entry(pstate, &clk->states, head) {
		list_for_each_entry(cstate, &pstate->list, head) {

			int score = is_state_better(core_load, PERF_TARGET_LOAD, old->domain[nv_clk_src_gpc], cstate->domain[nv_clk_src_gpc], PERF_TARGET_SAFETY);
			nvkm_trace(subdev, "dyn reclock core score: %i for cstate %i old_speed: %i speed: %i\n", score, cstate->cstate, old->domain[nv_clk_src_gpc], cstate->domain[nv_clk_src_gpc]);
			if (cstate->cstate == 0 && i == 0) {
				last_score = score;
				last_cstate = cstate->cstate;
				last_pstate = i;
			} else if ((last_score < 0 && last_score < score) || (last_score >= 0 && last_score > score && score > 0)) {
				if (last_cstate != cstate->cstate) {
					last_score = score;
					last_cstate = cstate->cstate;
					last_pstate = i;
					nvkm_trace(subdev, "newest best: %i", last_cstate);
				}
			}
		}
		i += 1;
	}

	if (last_pstate > *ps)
		*ps = last_pstate;

	return last_cstate;
}

int nvkm_clk_pmu_reclk_request(struct nvkm_clk *clk, int data)
{
	struct nvkm_subdev *subdev = &clk->subdev;
	struct nvkm_pmu *pmu = clk->subdev.device->pmu;
	u8 core_load =  data & 0x000000ff;
	u8 vid_load  = (data & 0x0000ff00) >>  8;
	u8 mem_load  = (data & 0x00ff0000) >> 16;
	u8 pcie_load = (data & 0xff000000) >> 24;
	int ps, cs;

	nvkm_debug(subdev, "reclock request from PMU (core: %02x mem: %02x vid: %02x pcie: %02x)\n", core_load, mem_load, vid_load, pcie_load);

	ps = calc_needed_pstate(clk, mem_load, vid_load, pcie_load);
	cs = calc_needed_cstate(clk, core_load, &ps);

	nvkm_debug(subdev, "reclocking to pstate: %i cstate: %i\n", ps, cs);

	if (clk->ustate_ac != ps || clk->ucstate != cs)
		nvkm_pmu_send(pmu, NULL, PROC_PERF, PERF_MSG_ACK_RECLOCK, 0, 0);

	clk->ucstate = cs;

	if (clk->ustate_ac == ps) {
		struct nvkm_pstate *pstate;
		int i = 0;

		list_for_each_entry(pstate, &clk->states, head) {
			if (i == ps)
				break;
			++i;
		}

		return nvkm_cstate_prog(clk, pstate, cs);
	} else {
		clk->ustate_ac = ps;
		clk->ustate_dc = ps;
		return nvkm_pstate_calc(clk, true);
	}

	return 0;
}

/******************************************************************************
 * subdev base class implementation
 *****************************************************************************/

int
nvkm_clk_read(struct nvkm_clk *clk, enum nv_clk_src src)
{
	return clk->func->read(clk, src);
}

static int
nvkm_clk_fini(struct nvkm_subdev *subdev, bool suspend)
{
	struct nvkm_clk *clk = nvkm_clk(subdev);
	nvkm_notify_put(&clk->pwrsrc_ntfy);
	flush_work(&clk->work);
	if (clk->func->fini)
		clk->func->fini(clk);
	return 0;
}

static int
nvkm_clk_init(struct nvkm_subdev *subdev)
{
	struct nvkm_clk *clk = nvkm_clk(subdev);
	const struct nvkm_domain *clock = clk->domains;
	int ret;

	memset(&clk->bstate, 0x00, sizeof(clk->bstate));
	INIT_LIST_HEAD(&clk->bstate.list);
	clk->bstate.pstate = 0xff;

	while (clock->name != nv_clk_src_max) {
		ret = nvkm_clk_read(clk, clock->name);
		if (ret < 0) {
			nvkm_error(subdev, "%02x freq unknown\n", clock->name);
			return ret;
		}
		clk->bstate.base.domain[clock->name] = ret;
		clock++;
	}

	nvkm_pstate_info(clk, &clk->bstate);

	if (clk->func->init)
		return clk->func->init(clk);

	clk->astate = clk->state_nr - 1;
	clk->tstate = 0;
	clk->dstate = 0;
	clk->pstate = -1;
	nvkm_pstate_calc(clk, true);
	return 0;
}

static void *
nvkm_clk_dtor(struct nvkm_subdev *subdev)
{
	struct nvkm_clk *clk = nvkm_clk(subdev);
	struct nvkm_pstate *pstate, *temp;

	nvkm_notify_fini(&clk->pwrsrc_ntfy);

	/* Early return if the pstates have been provided statically */
	if (clk->func->pstates)
		return clk;

	list_for_each_entry_safe(pstate, temp, &clk->states, head) {
		nvkm_pstate_del(pstate);
	}

	return clk;
}

static const struct nvkm_subdev_func
nvkm_clk = {
	.dtor = nvkm_clk_dtor,
	.init = nvkm_clk_init,
	.fini = nvkm_clk_fini,
};

int
nvkm_clk_ctor(const struct nvkm_clk_func *func, struct nvkm_device *device,
	      int index, bool allow_eng_reclock, bool allow_mem_reclock,
	      struct nvkm_clk *clk)
{
	int ret, idx, arglen;
	const char *mode;

	nvkm_subdev_ctor(&nvkm_clk, device, index, 0, &clk->subdev);
	clk->func = func;
	INIT_LIST_HEAD(&clk->states);
	clk->domains = func->domains;
	clk->ustate_ac = -1;
	clk->ustate_dc = -1;
	clk->ucstate   = -1;
	clk->allow_eng_reclock = allow_eng_reclock;
	clk->allow_mem_reclock = allow_mem_reclock;

	INIT_WORK(&clk->work, nvkm_pstate_work);
	init_waitqueue_head(&clk->wait);
	atomic_set(&clk->waiting, 0);

	/* If no pstates are provided, try and fetch them from the BIOS */
	if (!func->pstates) {
		idx = 0;
		do {
			ret = nvkm_pstate_new(clk, idx++);
		} while (ret == 0);
	} else {
		for (idx = 0; idx < func->nr_pstates; idx++)
			list_add_tail(&func->pstates[idx].head, &clk->states);
		clk->state_nr = func->nr_pstates;
	}

	ret = nvkm_notify_init(NULL, &device->event, nvkm_clk_pwrsrc, true,
			       NULL, 0, 0, &clk->pwrsrc_ntfy);
	if (ret)
		return ret;

	mode = nvkm_stropt(device->cfgopt, "NvClkMode", &arglen);
	if (mode) {
		clk->ustate_ac = nvkm_clk_nstate(clk, mode, arglen);
		clk->ustate_dc = nvkm_clk_nstate(clk, mode, arglen);
	}

	mode = nvkm_stropt(device->cfgopt, "NvClkModeAC", &arglen);
	if (mode)
		clk->ustate_ac = nvkm_clk_nstate(clk, mode, arglen);

	mode = nvkm_stropt(device->cfgopt, "NvClkModeDC", &arglen);
	if (mode)
		clk->ustate_dc = nvkm_clk_nstate(clk, mode, arglen);

	return 0;
}

int
nvkm_clk_new_(const struct nvkm_clk_func *func, struct nvkm_device *device,
	      int index, bool allow_eng_reclock, bool allow_mem_reclock,
	      struct nvkm_clk **pclk)
{
	if (!(*pclk = kzalloc(sizeof(**pclk), GFP_KERNEL)))
		return -ENOMEM;
	return nvkm_clk_ctor(func, device, index, allow_eng_reclock,
			     allow_mem_reclock, *pclk);
}
