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
#include "nv50.h"

#include <subdev/bios.h>
#include <subdev/bios/dcb.h>
#include <subdev/bios/disp.h>
#include <subdev/bios/init.h>
#include <subdev/bios/pll.h>
#include <subdev/clk/pll.h>
#include <subdev/ibus.h>
#include <subdev/vga.h>

int
nv50_devinit_pll_set(struct nvkm_devinit *init, u32 type, u32 freq)
{
	struct nvkm_subdev *subdev = &init->subdev;
	struct nvkm_device *device = subdev->device;
	struct nvkm_bios *bios = device->bios;
	struct nvbios_pll info;
	int N1, M1, N2, M2, P;
	int ret;

	ret = nvbios_pll_parse(bios, type, &info);
	if (ret) {
		nvkm_error(subdev, "failed to retrieve pll data, %d\n", ret);
		return ret;
	}

	ret = nv04_pll_calc(subdev, &info, freq, &N1, &M1, &N2, &M2, &P);
	if (!ret) {
		nvkm_error(subdev, "failed pll calculation\n");
		return ret;
	}

	switch (info.type) {
	case PLL_VPLL0:
	case PLL_VPLL1:
		nvkm_wr32(device, info.reg + 0, 0x10000611);
		nvkm_mask(device, info.reg + 4, 0x00ff00ff, (M1 << 16) | N1);
		nvkm_mask(device, info.reg + 8, 0x7fff00ff, (P  << 28) |
							    (M2 << 16) | N2);
		break;
	case PLL_MEMORY:
		nvkm_mask(device, info.reg + 0, 0x01ff0000,
					        (P << 22) |
						(info.bias_p << 19) |
						(P << 16));
		nvkm_wr32(device, info.reg + 4, (N1 << 8) | M1);
		break;
	default:
		nvkm_mask(device, info.reg + 0, 0x00070000, (P << 16));
		nvkm_wr32(device, info.reg + 4, (N1 << 8) | M1);
		break;
	}

	return 0;
}

static u64
nv50_devinit_disable(struct nvkm_devinit *init)
{
	struct nvkm_device *device = init->subdev.device;
	u32 r001540 = nvkm_rd32(device, 0x001540);
	u64 disable = 0ULL;

	if (!(r001540 & 0x40000000))
		disable |= (1ULL << NVDEV_ENGINE_MPEG);

	return disable;
}

int
nv50_devinit_init(struct nvkm_object *object)
{
	struct nv50_devinit *init = (void *)object;
	struct nvkm_subdev *subdev = &init->base.subdev;
	struct nvkm_device *device = subdev->device;
	struct nvkm_bios *bios = device->bios;
	struct nvkm_subdev *ibus = device->ibus;
	struct nvbios_outp info;
	struct dcb_output outp;
	u8  ver = 0xff, hdr, cnt, len;
	int ret, i = 0;

	if (!init->base.post) {
		if (!nv_rdvgac(init, 0, 0x00) &&
		    !nv_rdvgac(init, 0, 0x1a)) {
			nvkm_debug(subdev, "adaptor not initialised\n");
			init->base.post = true;
		}
	}

	/* some boards appear to require certain init register timeouts
	 * to be bumped before runing devinit scripts.  not a clue why
	 * the vbios engineers didn't make the scripts just work...
	 */
	if (init->base.post && ibus)
		nv_ofuncs(ibus)->init(nv_object(ibus));

	ret = nvkm_devinit_init(&init->base);
	if (ret)
		return ret;

	/* if we ran the init tables, we have to execute the first script
	 * pointer of each dcb entry's display encoder table in order
	 * to properly initialise each encoder.
	 */
	while (init->base.post && dcb_outp_parse(bios, i, &ver, &hdr, &outp)) {
		if (nvbios_outp_match(bios, outp.hasht, outp.hashm,
				      &ver, &hdr, &cnt, &len, &info)) {
			struct nvbios_init exec = {
				.subdev = nv_subdev(init),
				.bios = bios,
				.offset = info.script[0],
				.outp = &outp,
				.crtc = -1,
				.execute = 1,
			};

			nvbios_exec(&exec);
		}
		i++;
	}

	return 0;
}

int
nv50_devinit_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
		  struct nvkm_oclass *oclass, void *data, u32 size,
		  struct nvkm_object **pobject)
{
	struct nv50_devinit *init;
	int ret;

	ret = nvkm_devinit_create(parent, engine, oclass, &init);
	*pobject = nv_object(init);
	if (ret)
		return ret;

	return 0;
}

struct nvkm_oclass *
nv50_devinit_oclass = &(struct nvkm_devinit_impl) {
	.base.handle = NV_SUBDEV(DEVINIT, 0x50),
	.base.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nv50_devinit_ctor,
		.dtor = _nvkm_devinit_dtor,
		.init = nv50_devinit_init,
		.fini = _nvkm_devinit_fini,
	},
	.pll_set = nv50_devinit_pll_set,
	.disable = nv50_devinit_disable,
	.post = nvbios_init,
}.base;
