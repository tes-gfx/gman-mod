#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_bridge.h>
#include <drm/drm_vblank.h>

#include "gman_drv.h"
#include "gman_kms.h"

#define to_gman_device(x) \
	container_of(x, struct gman_device, connector)

#define to_gman_connector(x) \
	container_of(x, struct gman_drm_connector, connector)

#define to_gman_encoder(x) \
	container_of(x, struct gman_drm_encoder, encoder)

static const struct drm_mode_config_funcs mode_config_funcs = {
	.fb_create = drm_gem_fb_create,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

/*******************************************************************************
 Pipe
*/

static enum drm_mode_status gman_pipe_mode_valid(struct drm_crtc *crtc,
						   const struct drm_display_mode *mode)
{
	/* Clock is fixed to 65 MHz */
	if (mode->clock > 65000) {
		return MODE_CLOCK_HIGH;
	}
	if (mode->clock < 65000) {
		return MODE_CLOCK_LOW;
	}

    return MODE_OK;
}

static void gman_pipe_update(struct drm_simple_display_pipe *pipe,
			       struct drm_plane_state *old_state)
{
	struct drm_crtc *crtc = &pipe->crtc;

	if (crtc->state->event) {
		spin_lock_irq(&crtc->dev->event_lock);
		drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
		spin_unlock_irq(&crtc->dev->event_lock);
	}
}

static const struct drm_simple_display_pipe_funcs gman_pipe_funcs = {
	.mode_valid = gman_pipe_mode_valid,
	.update = gman_pipe_update,
};

static const uint32_t gman_formats[] = {
	DRM_FORMAT_RGB888,
	DRM_FORMAT_XRGB8888,
};

static const uint64_t gman_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID
};

int gman_modeset_init(struct gman_device *gdev, struct drm_bridge *bridge)
{
    struct drm_device *ddev = gdev->ddev;
	struct drm_mode_config *mode_config;
	int ret = 0;

	drm_mode_config_init(ddev);
	mode_config = &ddev->mode_config;
	mode_config->funcs = &mode_config_funcs;
	mode_config->min_width = 1;
	mode_config->max_width = 1920;
	mode_config->min_height = 1;
	mode_config->max_height = 1200;

    ret = drm_simple_display_pipe_init(ddev, &gdev->pipe, &gman_pipe_funcs,
        gman_formats, ARRAY_SIZE(gman_formats), gman_modifiers, NULL);
    if (ret) {
        DRM_ERROR("Could not initialize pipe.");
        return ret;
    }

    ret = drm_simple_display_pipe_attach_bridge(&gdev->pipe, bridge);
    if (ret) {
        DRM_ERROR("Could not attach bridge.");
        return ret;
    }

    drm_mode_config_reset(ddev);
    drm_kms_helper_poll_init(ddev);

    return 0;
}