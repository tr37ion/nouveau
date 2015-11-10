/*
 * Copyright (C) 2009 Red Hat <bskeggs@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * Authors:
 *  Ben Skeggs <bskeggs@redhat.com>
 */

#include <nvkm/subdev/pmu.h>

#include <linux/debugfs.h>
#include <nvif/ioctl.h>
#include "nouveau_debugfs.h"
#include "nouveau_drm.h"

static int
nouveau_debugfs_vbios_image(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct nouveau_drm *drm = nouveau_drm(node->minor->dev);
	int i;

	for (i = 0; i < drm->vbios.length; i++)
		seq_printf(m, "%c", drm->vbios.data[i]);
	return 0;
}

static int
nouveau_debugfs_current_load(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct nouveau_drm *drm = nouveau_drm(node->minor->dev);
	struct nvkm_pmu *pmu = nvxx_pmu(&drm->device);
	struct nvkm_pmu_load_data load_data = { 0 };

	if (!pm_runtime_suspended(drm->dev->dev)) {
		int ret = nvkm_pmu_get_perf_data(pmu, &load_data);
		if (ret < 0)
			return ret;
	}

	seq_printf(m, "core: %i\n", load_data.core);
	seq_printf(m, "mem: %i\n", load_data.mem);
	seq_printf(m, "video: %i\n", load_data.video);
	seq_printf(m, "pcie: %i\n", load_data.pcie);
	return 0;
}

static int
nouveau_debugfs_pstate_get(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct nouveau_debugfs *debugfs = nouveau_debugfs(node->minor->dev);
	struct nvif_object *ctrl;
	struct nvif_control_pstate_info_v0 info = {};
	int ret, i;

	if (!debugfs)
		return -ENODEV;

	ctrl = &debugfs->ctrl;

	ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_INFO,
			&info, sizeof(info));
	if (ret)
		return ret;

	for (i = 0; i < info.count + 1; i++) {
		const s32 state = i < info.count ? i :
			NVIF_CONTROL_PSTATE_ATTR_V0_STATE_CURRENT;
		struct nvif_control_pstate_attr_v0 attr = {
			.state = state,
			.index = 0,
		};

		ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_ATTR,
				&attr, sizeof(attr));
		if (ret == -EAGAIN) {
			ret = 0;
			continue;
		}
		if (ret)
			return ret;

		if (i < info.count)
			seq_printf(m, "%02x:", attr.state);
		else
			seq_printf(m, "%s:", info.pwrsrc == 0 ? "DC" :
						   info.pwrsrc == 1 ? "AC" :
						   "--");

		attr.index = 0;
		do {
			attr.state = state;
			ret = nvif_mthd(ctrl,
					NVIF_CONTROL_PSTATE_ATTR,
					&attr, sizeof(attr));
			if (ret)
				return ret;

			seq_printf(m, " %s %d", attr.name, attr.min);
			if (attr.min != attr.max)
				seq_printf(m, "-%d", attr.max);
			seq_printf(m, " %s", attr.unit);
		} while (attr.index);

		if (state >= 0) {
			if (info.ustate_ac == state)
				seq_printf(m, " AC");
			if (info.ustate_dc == state)
				seq_printf(m, " DC");
			if (info.pstate == state)
				seq_printf(m, " *");
		} else {
			if (info.ustate_ac < -1)
				seq_printf(m, " AC");
			if (info.ustate_dc < -1)
				seq_printf(m, " DC");
		}

		seq_printf(m, "\n");
	}

	return 0;
}

static ssize_t
nouveau_debugfs_pstate_set(struct file *file, const char __user *ubuf, size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct nouveau_debugfs *debugfs = nouveau_debugfs(node->minor->dev);
	struct nvif_object *ctrl;
	struct nvif_control_pstate_user_v0 args = { .pwrsrc = -EINVAL };
	char buf[32] = {}, *tmp, *cur = buf;
	long value, ret;

	if (!debugfs)
		return -ENODEV;

	ctrl = &debugfs->ctrl;

	if (len >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	if ((tmp = strchr(buf, '\n')))
		*tmp = '\0';

	if (!strncasecmp(cur, "dc:", 3)) {
		args.pwrsrc = 0;
		cur += 3;
	} else
	if (!strncasecmp(cur, "ac:", 3)) {
		args.pwrsrc = 1;
		cur += 3;
	}

	if (!strcasecmp(cur, "none"))
		args.ustate = NVIF_CONTROL_PSTATE_USER_V0_STATE_UNKNOWN;
	else
	if (!strcasecmp(cur, "auto"))
		args.ustate = NVIF_CONTROL_PSTATE_USER_V0_STATE_PERFMON;
	else {
		ret = kstrtol(cur, 16, &value);
		if (ret)
			return ret;
		args.ustate = value;
	}

	ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_USER,
			&args, sizeof(args));
	if (ret < 0)
		return ret;

	return len;
}

static int
nouveau_debugfs_pstate_open(struct inode *inode, struct file *file)
{
	return single_open(file, nouveau_debugfs_pstate_get, inode->i_private);
}

static const struct file_operations nouveau_pstate_fops = {
	.owner = THIS_MODULE,
	.open = nouveau_debugfs_pstate_open,
	.read = seq_read,
	.write = nouveau_debugfs_pstate_set,
};

static int
nouveau_debugfs_cstate_get(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct nouveau_debugfs *debugfs = nouveau_debugfs(node->minor->dev);
	struct nvif_object *ctrl;
	struct nvif_control_pstate_info_v0 info = {};
	struct nvif_control_pstate_attr_v0 attr = {};
	struct nvif_control_pstate_clock_info_v0 cinfo = {};
	struct nvif_control_pstate_cstate *cstate;
	int ret, i;

	if (!debugfs)
		return -ENODEV;

	ctrl = &debugfs->ctrl;

	ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_INFO,
			&info, sizeof(info));

	if (ret)
		return ret;

	attr.state = info.pstate;

	ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_ATTR,
		&attr, sizeof(attr));
	if (ret)
		return ret;

	ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_CLOCK_INFO,
		&cinfo, sizeof(cinfo));
	if (ret)
		return ret;

	seq_printf(m, "current pstate: 0x%02x\n", attr.state);
	seq_printf(m, "available clockings: %d - %d %s\n", attr.min,
		attr.max, attr.unit);

	seq_printf(m, "available cstates for current pstate:\n");
	for (i = 0; i < cinfo.states; i++) {
		cstate = &cinfo.cstates[i];
		seq_printf(m, "%d: {gpc: %d, mem: %d, voltage: %d}\n",
			cstate->cstate, cstate->gpc, cstate->mem, cstate->voltage);
	}

	seq_printf(m, "current core: %d\n", cinfo.gpc);
	seq_printf(m, "current memory: %d\n", cinfo.mem);

	return 0;
}

static ssize_t
nouveau_debugfs_cstate_set(struct file *file, const char __user *ubuf, size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct nouveau_debugfs *debugfs = nouveau_debugfs(node->minor->dev);
	struct nvif_object *ctrl;
	struct nvif_control_pstate_set_cstate_v0 args = {};
	int ret;
	char buf[10] = {};

	if (!debugfs)
		return -ENODEV;

	ctrl = &debugfs->ctrl;

	if (len >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	ret = kstrtou8(buf, 10, &args.cstate);
	if (ret)
		return ret;

	ret = nvif_mthd(ctrl, NVIF_CONTROL_PSTATE_SET_CSTATE,
		&args, sizeof(args));

	return len;
}

static int
nouveau_debugfs_cstate_open(struct inode *inode, struct file *file)
{
	return single_open(file, nouveau_debugfs_cstate_get, inode->i_private);
}

static const struct file_operations nouveau_cstate_fops = {
	.owner = THIS_MODULE,
	.open = nouveau_debugfs_cstate_open,
	.read = seq_read,
	.write = nouveau_debugfs_cstate_set,
};

static struct drm_info_list nouveau_debugfs_list[] = {
	{ "vbios.rom", nouveau_debugfs_vbios_image, 0, NULL },
	{ "current_load", nouveau_debugfs_current_load, 0, NULL },
};
#define NOUVEAU_DEBUGFS_ENTRIES ARRAY_SIZE(nouveau_debugfs_list)

static const struct nouveau_debugfs_files {
	const char *name;
	const struct file_operations *fops;
} nouveau_debugfs_files[] = {
	{"pstate", &nouveau_pstate_fops},
	{"cstate", &nouveau_cstate_fops},
};

static int
nouveau_debugfs_create_file(struct drm_minor *minor,
		const struct nouveau_debugfs_files *ndf)
{
	struct drm_info_node *node;

	node = kmalloc(sizeof(*node), GFP_KERNEL);

	if (node == NULL)
		return -ENOMEM;

	node->minor = minor;
	node->info_ent = (const void *)ndf->fops;
	node->dent = debugfs_create_file(ndf->name, S_IRUGO | S_IWUSR, minor->debugfs_root,
			node, ndf->fops);

	if (!node->dent) {
		kfree(node);
		return -ENOMEM;
	}

	mutex_lock(&minor->debugfs_lock);
	list_add(&node->list, &minor->debugfs_list);
	mutex_unlock(&minor->debugfs_lock);

	return 0;
}

int
nouveau_drm_debugfs_init(struct drm_minor *minor)
{
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(nouveau_debugfs_files); i++) {
		ret = nouveau_debugfs_create_file(minor, &nouveau_debugfs_files[i]);

		if (ret)
			return ret;
	}

	return drm_debugfs_create_files(nouveau_debugfs_list, NOUVEAU_DEBUGFS_ENTRIES,
				 minor->debugfs_root, minor);
}

void
nouveau_drm_debugfs_cleanup(struct drm_minor *minor)
{
	int i;

	drm_debugfs_remove_files(nouveau_debugfs_list, NOUVEAU_DEBUGFS_ENTRIES,
				 minor);

	for (i = 0; i < ARRAY_SIZE(nouveau_debugfs_files); i++) {
		drm_debugfs_remove_files((struct drm_info_list *)nouveau_debugfs_files[i].fops,
				1, minor);
	}
}

int
nouveau_debugfs_init(struct nouveau_drm *drm)
{
	int ret;

	drm->debugfs = kzalloc(sizeof(*drm->debugfs), GFP_KERNEL);
	if (!drm->debugfs) {
		return-ENOMEM;
	}

	ret = nvif_object_init(&drm->device.object, 0,
			NVIF_IOCTL_NEW_V0_CONTROL, NULL, 0, &drm->debugfs->ctrl);

	if (ret)
		return ret;

	return 0;
}

void
nouveau_debugfs_cleanup(struct nouveau_drm *drm)
{
	if (drm->debugfs && drm->debugfs->ctrl.priv)
		nvif_object_fini(&drm->debugfs->ctrl);

	kfree(drm->debugfs);
	drm->debugfs = NULL;
}
