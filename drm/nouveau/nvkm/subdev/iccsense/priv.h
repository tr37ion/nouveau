#ifndef __NVKM_ICCSENSE_PRIV_H__
#define __NVKM_ICCSENSE_PRIV_H__
#define nvkm_iccsense(p) container_of((p), struct nvkm_iccsense, subdev)
#include <subdev/iccsense.h>

void
nvkm_iccsense_ctor(const struct nvkm_subdev_func *,
		   const struct nvkm_iccsense_driver *,
		   struct nvkm_device *, int , struct nvkm_iccsense *);

struct nvkm_iccsense_driver {
	const char *name;
	int (*pwr_get)(struct nvkm_iccsense *);
};

int
nvkm_iccsense_ina3221_new(struct nvkm_device *device, int index,
			struct nvkm_iccsense **piccsense);

#endif
