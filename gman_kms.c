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

static void gman_connector_destroy(struct drm_connector *connector)
{
    DRM_INFO("%s", __func__);

	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}


static enum drm_connector_status gman_connector_detect(struct drm_connector
							*connector, bool force)
{
    DRM_INFO("%s", __func__);

	return connector_status_connected;
}


static int gman_connector_helper_get_modes(struct drm_connector *connector)
{
    struct drm_display_mode *mode;
    struct gman_drm_connector *gman_connector = to_gman_connector(connector);
    struct gman_device *gman_dev = to_gman_device(gman_connector);

    DRM_INFO("%s", __func__);

	mode = drm_mode_create(gman_dev->ddev);
    mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;

    mode->width_mm = 400;
    mode->height_mm = 225;
    mode->hdisplay = 1920;
    mode->vdisplay = 1200;

    drm_mode_set_name(mode);
	drm_mode_probed_add(connector, mode);

	return 1;
}


const struct drm_connector_funcs connector_funcs = {
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = gman_connector_destroy,
	.detect = gman_connector_detect,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};


const struct drm_connector_helper_funcs connector_helper_funcs = {
	.get_modes = gman_connector_helper_get_modes,
};


int gman_connector_init(struct drm_device *dev)
{
	struct gman_device *priv = dev->dev_private;
	struct gman_drm_connector *gman_connector = &priv->connector;
	struct drm_connector *connector = &gman_connector->connector;

    DRM_INFO("%s", __func__);

	drm_connector_helper_add(connector, &connector_helper_funcs);	
	drm_connector_init(dev, connector, &connector_funcs,
			   DRM_MODE_CONNECTOR_LVDS);

	return 0;
}


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

static enum drm_mode_status
drm_simple_kms_crtc_mode_valid(struct drm_crtc *crtc,
			       const struct drm_display_mode *mode)
{
    DRM_INFO("%s", __func__);
	return MODE_OK;
}

static int drm_simple_kms_crtc_check(struct drm_crtc *crtc,
				     struct drm_crtc_state *state)
{
    DRM_INFO("%s", __func__);
	return 0;
}

static void drm_simple_kms_crtc_enable(struct drm_crtc *crtc,
				       struct drm_crtc_state *old_state)
{
    DRM_INFO("%s", __func__);
}

static void drm_simple_kms_crtc_disable(struct drm_crtc *crtc,
					struct drm_crtc_state *old_state)
{
    DRM_INFO("%s", __func__);
}

static const struct drm_crtc_helper_funcs drm_simple_kms_crtc_helper_funcs = {
	.mode_valid     = drm_simple_kms_crtc_mode_valid,
	.atomic_check   = drm_simple_kms_crtc_check,
	.atomic_enable  = drm_simple_kms_crtc_enable,
	.atomic_disable = drm_simple_kms_crtc_disable,
};

static int drm_simple_kms_crtc_enable_vblank(struct drm_crtc *crtc)
{
    DRM_INFO("%s", __func__);
	return 0;
}

static void drm_simple_kms_crtc_disable_vblank(struct drm_crtc *crtc)
{
    DRM_INFO("%s", __func__);
}

static const struct drm_crtc_funcs drm_simple_kms_crtc_funcs = {
	.reset = drm_atomic_helper_crtc_reset,
	.destroy = drm_crtc_cleanup,
	.set_config = drm_atomic_helper_set_config,
	.page_flip = drm_atomic_helper_page_flip,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
	.enable_vblank = drm_simple_kms_crtc_enable_vblank,
	.disable_vblank = drm_simple_kms_crtc_disable_vblank,
};

static int drm_simple_kms_plane_atomic_check(struct drm_plane *plane,
					struct drm_plane_state *plane_state)
{
    DRM_INFO("%s", __func__);
	return 0;
}

static void drm_simple_kms_plane_atomic_update(struct drm_plane *plane,
					struct drm_plane_state *old_pstate)
{
    DRM_INFO("%s", __func__);
}

static int drm_simple_kms_plane_prepare_fb(struct drm_plane *plane,
					   struct drm_plane_state *state)
{
    DRM_INFO("%s", __func__);
	return 0;
}

static void drm_simple_kms_plane_cleanup_fb(struct drm_plane *plane,
					    struct drm_plane_state *state)
{
    DRM_INFO("%s", __func__);
}

static bool drm_simple_kms_format_mod_supported(struct drm_plane *plane,
						uint32_t format,
						uint64_t modifier)
{
	return modifier == DRM_FORMAT_MOD_LINEAR;
}

static const struct drm_plane_helper_funcs drm_simple_kms_plane_helper_funcs = {
	.prepare_fb = drm_simple_kms_plane_prepare_fb,
	.cleanup_fb = drm_simple_kms_plane_cleanup_fb,
	.atomic_check = drm_simple_kms_plane_atomic_check,
	.atomic_update = drm_simple_kms_plane_atomic_update,
};

static const struct drm_plane_funcs drm_simple_kms_plane_funcs = {
	.update_plane		    = drm_atomic_helper_update_plane,
	.disable_plane		    = drm_atomic_helper_disable_plane,
	.destroy		        = drm_plane_cleanup,
	.reset			        = drm_atomic_helper_plane_reset,
	.atomic_duplicate_state	= drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_plane_destroy_state,
	.format_mod_supported   = drm_simple_kms_format_mod_supported,
};

int gman_pipe_init(struct gman_device *gdev, struct drm_bridge *bridge)
{
    int ret;
    struct drm_device *ddev = gdev->ddev;
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

int gman_modeset_init(struct gman_device *gdev, struct drm_bridge *bridge)
{
    struct drm_device *ddev = gdev->ddev;
	struct drm_mode_config *mode_config;
	int ret = 0;

    DRM_INFO("%s", __func__);

	drm_mode_config_init(ddev);
	mode_config = &ddev->mode_config;
	mode_config->funcs = &mode_config_funcs;
	mode_config->min_width = 1;
	mode_config->max_width = 1920;
	mode_config->min_height = 1;
	mode_config->max_height = 1200;

    //gman_connector_init(ddev);
    gman_pipe_init(gdev, bridge);

    return ret;
}