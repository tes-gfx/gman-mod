#ifndef __GMAN_DRV_H__
#define __GMAN_DRV_H__

#include <linux/kernel.h>
#include <linux/wait.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_bridge.h>

#include "gman_kms.h"

struct clk;
struct device;
struct drm_device;
struct drm_property;
struct gman_device;
struct gman_encoder;

struct gman_device {
	struct device *dev;
	struct drm_device *ddev;
	struct drm_simple_display_pipe pipe;
	struct drm_bridge *bridge;
};

#endif /* __GMAN_DRV_H__ */
