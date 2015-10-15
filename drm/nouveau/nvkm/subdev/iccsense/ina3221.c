/*
 * Copyright 2015 Martin Peres
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
 * Authors: Martin Peres
 */
#include "priv.h"

#include <subdev/i2c.h>
#include <subdev/bios.h>
#include <subdev/bios/extdev.h>
#include <subdev/bios/iccsense.h>

struct iccsense_ina3221_priv {
	struct nvkm_iccsense base;

	struct nvkm_i2c_bus *i2c_bus;
	u8 rails_mohm[3];
};

static int
ina3221_poll_power_lane(struct i2c_adapter *adap, uint8_t id, uint8_t shunt)
{
	int vbus, vshunt;

	if (id > 2)
		return -EINVAL;

	shunt = 5;
	if (shunt == 0)
		return 0;

	vshunt = nv_rd16i2cr(adap, 0x40, 1 + (id * 2));
	vbus = nv_rd16i2cr(adap, 0x40, 2 + (id * 2));

	if (vshunt < 0 || vbus < 0)
		return -EINVAL;

	return vbus * vshunt * 5 / shunt;
}


static int
ina3221_pwr_get(struct nvkm_iccsense *iccsense)
{
	struct iccsense_ina3221_priv *priv;
	uint32_t pwr = 0;
	int ret, i;

	priv = (struct iccsense_ina3221_priv*) iccsense;

	for(i = 0; i < 3; i++) {
		int lane = ina3221_poll_power_lane(&priv->i2c_bus->i2c, i,
						   priv->rails_mohm[i]);
		if (lane >= 0)
			pwr += lane;
	}

	return pwr;
}

static int
ina3221_init(struct nvkm_subdev *subdev)
{
	return 0;
}

static void *
ina3221_dtor(struct nvkm_subdev *subdev)
{
	return nvkm_iccsense(subdev);
}

static const struct nvkm_subdev_func
ina3221_iccsense = {
	.dtor = ina3221_dtor,
	.init = ina3221_init,
};

static const struct nvkm_iccsense_driver
iccsense_ina3221_driver = {
	.name = "INA3221",
	.pwr_get = ina3221_pwr_get,
};

int
nvkm_iccsense_ina3221_new(struct nvkm_device *device, int index,
			struct nvkm_iccsense **piccsense)
{
	struct nvkm_bios *bios = device->bios;
	struct nvbios_iccsense iccsense_tbl;
	struct iccsense_ina3221_priv *priv;
	struct nvkm_i2c *i2c = device->i2c;
	struct nvbios_extdev_func extdev;
	struct nvkm_i2c_bus *i2c_bus;
	int i;

	/* Look for an INA3221 */
	if (nvbios_extdev_find(bios, NVBIOS_EXTDEV_INA3221, &extdev))
		return -EINVAL;

	/* check if the bus exists */
	/* TODO: Actually check if the device is present and if not, iterate
	 * to the next bus!
	 */
	i2c_bus = nvkm_i2c_bus_find(i2c, 2 /*extdev.bus*/);
	if (!i2c_bus)
		return -EINVAL;

	/* check that we do have a iccsense table */
	if (nvbios_iccsense_parse(bios, &iccsense_tbl))
		return -EINVAL;

	/* parse the iccsense table */
	if (!(*piccsense = kzalloc(sizeof(struct iccsense_ina3221_priv), GFP_KERNEL)))
		return -ENOMEM;
	priv = (struct iccsense_ina3221_priv*) *piccsense;
	nvkm_iccsense_ctor(&ina3221_iccsense, &iccsense_ina3221_driver, device, index, *piccsense);

	priv->i2c_bus = i2c_bus;

	/* copy the shunt resistors' values to the right lane index */
	for (i = 0; i < iccsense_tbl.nr_entry; i++) {
		struct pwr_rail_t r = iccsense_tbl.rail[i];
		priv->rails_mohm[r.extdev_id] = r.resistor_mohm;
		nvkm_debug(&priv->base.subdev, "rail[%i] = %i mOhm\n", r.extdev_id, r.resistor_mohm);
	}

	return 0;
}
