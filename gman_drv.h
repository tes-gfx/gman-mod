#ifndef __GMAN_DRV_H__
#define __GMAN_DRV_H__

#include <linux/kernel.h>
#include <linux/wait.h>

struct clk;
struct device;
struct drm_device;
struct drm_property;
struct gman_device;
struct gman_encoder;

struct gman_device {
	struct device *dev;

	void __iomem *mmio;

	struct drm_device *ddev;

	struct gman_crtc *crtcs;
	unsigned int num_crtcs;

	struct gman_encoder *encoders;
};

#endif /* __GMAN_DRV_H__ */
