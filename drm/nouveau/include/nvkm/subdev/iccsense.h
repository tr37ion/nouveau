#ifndef __NVKM_ICCSENSE_H__
#define __NVKM_ICCSENSE_H__
#include <core/device.h>
#include <core/subdev.h>
#include <subdev/bios/iccsense.h>

struct nvkm_iccsense_driver;

struct nvkm_iccsense {
	struct nvkm_subdev subdev;
	const struct nvkm_iccsense_driver *driver;
};

int nvbios_iccsense_parse(struct nvkm_bios *, struct nvbios_iccsense *);

int gf100_iccsense_new(struct nvkm_device *, int index, struct nvkm_iccsense **);

#endif
