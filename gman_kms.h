#ifndef __GMAN_KMS_H__
#define __GMAN_KMS_H__

#include <drm/drm_drv.h>
#include <drm/drm_encoder_slave.h>
#include <drm/drm_bridge.h>

struct gman_device;

struct gman_drm_connector {
	struct drm_connector connector;
};

struct gman_drm_encoder {
	struct drm_encoder encoder;
	struct drm_encoder_slave slave;
};

int gman_modeset_init(struct gman_device *gdev, struct drm_bridge *bridge);
int gman_encoder_init(struct gman_device *gdev);

#endif
