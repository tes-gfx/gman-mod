#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_bridge.h>

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
    DRM_INFO("%s", __func__);
    return MODE_OK;
}

static int gman_pipe_check(struct drm_simple_display_pipe *pipe,
			     struct drm_plane_state *plane_state,
			     struct drm_crtc_state *crtc_state)
{
    DRM_INFO("%s", __func__);
    return 0;
}

static void gman_pipe_enable(struct drm_simple_display_pipe *pipe,
			       struct drm_crtc_state *crtc_state,
			       struct drm_plane_state *plane_state)
{
    DRM_INFO("%s", __func__);
}

static void gman_pipe_update(struct drm_simple_display_pipe *pipe,
			       struct drm_plane_state *old_state)
{
    DRM_INFO("%s", __func__);
}

static const struct drm_simple_display_pipe_funcs gman_pipe_funcs = {
	.mode_valid = gman_pipe_mode_valid,
	.check	    = gman_pipe_check,
	.enable	    = gman_pipe_enable,
	.update	    = gman_pipe_update,
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