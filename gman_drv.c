#include <linux/module.h>

#include <drm/drm_drv.h>
#include <drm/drm_gem_cma_helper.h>

#include "gman_drv.h"

static struct class *gman_class;
struct gman_device *gman_device;

DEFINE_DRM_GEM_CMA_FOPS(gman_fops);

static struct drm_driver gman_driver = {
	.driver_features	= DRIVER_GEM | DRIVER_ATOMIC,
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
	.fops			= &gman_fops,
	.name			= "gman",
	.desc			= "TES Graphics Manager",
	.date			= "20220721",
	.major			= 1,
	.minor			= 0,
};

static int gman_remove(struct gman_device *gdev)
{
	struct drm_device *ddev = gdev->ddev;

	drm_dev_unregister(ddev);
	drm_dev_put(ddev);

	return 0;
}

static int gman_init_internal(struct device *dev)
{
	struct gman_device *gdev;
	struct drm_device *ddev;
	int ret;

	gdev = devm_kzalloc(dev, sizeof(*gdev), GFP_KERNEL);
	if (gdev == NULL)
		return -ENOMEM;

	gdev->dev = dev;

	ddev = drm_dev_alloc(&gman_driver, dev);
	if (IS_ERR(ddev))
		return PTR_ERR(ddev);

	gdev->ddev = ddev;
	ddev->dev_private = gdev;

	ret = drm_dev_register(ddev, 0);
	if (ret)
		goto error;

	DRM_INFO("%s initialized\n", dev_name(dev));

	gman_device = gdev;

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
