#include <linux/module.h>
#include <linux/device.h>

#include <drm/drm_drv.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_encoder_slave.h>
#include <drm/drm_fb_helper.h>

#include "gman_drv.h"
#include "gman_gem.h"
#include "gman_drm.h"
#include "gman_kms.h"

static struct class *gman_class;
static struct gman_device *gman_device;

static int gman_ioctl_gem_new(struct drm_device *dev, void *data,
	struct drm_file *file)
{
	struct drm_gman_gem_new *args = data;
	struct drm_gem_object *bo = NULL;
	dma_addr_t paddr;
	int ret;

	dev_dbg(dev->dev, "New bo:\n");
	dev_dbg(dev->dev, " size=0x%08llx\n", args->size);
	dev_dbg(dev->dev, " flags=0x%08x\n", args->flags);

	if (args->flags & ~(GMAN_BO_CACHED | GMAN_BO_WC | GMAN_BO_UNCACHED))
			return -EINVAL;

	bo = gman_gem_new(dev, args->size, &paddr);
	if(IS_ERR(bo))
		return PTR_ERR(bo);

	args->paddr = paddr;
	dev_dbg(dev->dev, " paddr=0x%08llx\n", args->paddr);

	ret = drm_gem_handle_create(file, bo, &args->handle);
	drm_gem_object_put_unlocked(bo);

	dev_dbg(dev->dev, " handle=0x%08x\n", args->handle);
	dev_dbg(dev->dev, " obj=0x%p\n", bo);

	return ret;
}

static int gman_ioctl_gem_info(struct drm_device *dev, void *data,
		struct drm_file *file)
{
	struct drm_gman_gem_info *args = data;
	struct drm_gem_object *obj;
	int ret;

	if (args->pad)
		return -EINVAL;

	obj = drm_gem_object_lookup(file, args->handle);
	if (!obj)
		return -ENOENT;

	ret = gman_gem_mmap_offset(obj, &args->offset);
	drm_gem_object_put_unlocked(obj);

	return ret;
}

static int gman_ioctl_gem_user(struct drm_device *dev, void *data,
		struct drm_file *file)
{
	struct drm_gman_gem_user *args = data;
	struct drm_gem_object *obj;
	struct drm_gem_cma_object *obj_cma;

	if (args->pad)
		return -EINVAL;

	obj = drm_gem_object_lookup(file, args->handle);
	if (!obj)
		return -ENOENT;

	obj_cma = to_drm_gem_cma_obj(obj);
	args->paddr = obj_cma->paddr;

	drm_gem_object_put_unlocked(obj);

	return 0;
}

static const struct drm_ioctl_desc gman_ioctls[] = {
#define GMAN_IOCTL(n, func, flags) \
	DRM_IOCTL_DEF_DRV(GMAN_##n, gman_ioctl_##func, flags)
	GMAN_IOCTL(GEM_NEW,       gem_new,       DRM_AUTH|DRM_RENDER_ALLOW),
	GMAN_IOCTL(GEM_INFO,      gem_info,      DRM_AUTH|DRM_RENDER_ALLOW),
	GMAN_IOCTL(GEM_USER,      gem_user,      DRM_AUTH|DRM_RENDER_ALLOW),
};

DEFINE_DRM_GEM_CMA_FOPS(gman_fops);

static struct drm_driver gman_driver = {
	.driver_features	= DRIVER_GEM | DRIVER_ATOMIC | DRIVER_MODESET,
	.gem_free_object_unlocked = drm_gem_cma_free_object,
	.gem_vm_ops		= &drm_gem_cma_vm_ops,
	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,
	.gem_prime_get_sg_table	= drm_gem_cma_prime_get_sg_table,
	.gem_prime_import_sg_table = drm_gem_cma_prime_import_sg_table,
	.gem_prime_vmap		= drm_gem_cma_prime_vmap,
	.gem_prime_vunmap	= drm_gem_cma_prime_vunmap,
	.gem_prime_mmap		= drm_gem_cma_prime_mmap,
	.dumb_create		= drm_gem_cma_dumb_create,
	.ioctls         = gman_ioctls,
	.num_ioctls     = DRM_GMAN_NUM_IOCTLS,
	.fops			= &gman_fops,
	.name			= "tes-gman",
	.desc			= "TES Graphics Manager",
	.date			= "20220721",
	.major			= 1,
	.minor			= 0,
};

static int match_encoder_slave(struct device *dev, const void *data)
{
	const char *name = data;
	return dev->driver && strcmp(dev->driver->name, name) == 0;
}

static struct i2c_client *find_encoder_slave(const char *driver_name)
{
	struct device *dev;
	struct i2c_client *i2c_client;

	dev = bus_find_device(&i2c_bus_type, NULL, driver_name, match_encoder_slave);

	if(!dev)
		return NULL;

	i2c_client = to_i2c_client(dev);
	return i2c_client;
}

static int gman_remove(struct gman_device *gdev)
{
	struct drm_device *ddev = gdev->ddev;

	drm_dev_unregister(ddev);
	drm_mode_config_cleanup(ddev);
	drm_dev_put(ddev);

	return 0;
}

static int gman_init_internal(struct device *dev)
{
	struct gman_device *gdev;
	struct drm_device *ddev;
	struct i2c_client *i2c_bridge;
	int ret;

	ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;

	gdev = devm_kzalloc(dev, sizeof(*gdev), GFP_KERNEL);
	if (gdev == NULL)
		return -ENOMEM;

	gdev->dev = dev;

	ddev = drm_dev_alloc(&gman_driver, dev);
	if (IS_ERR(ddev))
		return PTR_ERR(ddev);

	gdev->ddev = ddev;
	ddev->dev_private = gdev;

	i2c_bridge = find_encoder_slave("adv7511");
	if(i2c_bridge) {
		DRM_INFO("gman: Found DRM bridge device %s\n", i2c_bridge->name);
		gdev->bridge = of_drm_find_bridge(i2c_bridge->dev.of_node);

		ret = gman_modeset_init(gdev);
		if (ret != 0)
			goto error;
	}

	ret = drm_dev_register(ddev, 0);
	if (ret)
		goto error;

	gman_device = gdev;

	/* Establish a dummy client to enable the bridge. We simply use fbdev and don't
	 * care about the wasted memory for the frame buffer.
	 */
	if (gdev->bridge)
		drm_fbdev_generic_setup(ddev, 0);

	return 0;

error:
	gman_remove(gdev);
	return ret;
}

static int __init gman_init(void)
{
	int res;
	struct device *dev;

	gman_class = class_create(THIS_MODULE, "gman");
	if(IS_ERR(gman_class)) {
		return PTR_ERR(gman_class);
	}

	dev = device_create(gman_class, NULL, MKDEV(0,0), NULL, "gman");
	if(IS_ERR(dev)) {
		res = PTR_ERR(dev);
		goto err_device;
	}

	res = gman_init_internal(dev);
	if(res < 0)
		goto err_init;

	return 0;
err_init:
	device_destroy(gman_class, MKDEV(0,0));
err_device:
	class_destroy(gman_class);
	return res;
}
module_init(gman_init);

static void __exit gman_exit(void)
{
	gman_remove(gman_device);
	device_destroy(gman_class, MKDEV(0,0));
	class_destroy(gman_class);
}
module_exit(gman_exit);

MODULE_AUTHOR("TES Electronic Solutions GmbH");
MODULE_DESCRIPTION("GMAN Graphics Manager for TES EvalKit");
MODULE_LICENSE("GPL");
