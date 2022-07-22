#ifndef __GMAN_DRM_H__
#define __GMAN_DRM_H__

#include <drm/drm.h>

/*
 * GEM buffers:
 */

#define GMAN_BO_CACHE_MASK   0x000f0000
/* cache modes */
#define GMAN_BO_CACHED       0x00010000
#define GMAN_BO_WC           0x00020000
#define GMAN_BO_UNCACHED     0x00040000

struct drm_gman_gem_new {
	__u64 size;           /* in */
	__u32 flags;          /* in, mask of DNX_BO_x */
	__u32 handle;         /* out */
	__u64 paddr;          /* out, physical address of contiguous buffer */
};

struct drm_gman_gem_info {
	__u32 handle;         /* in */
	__u32 pad;
	__u64 offset;         /* out, offset to pass to mmap() */
};

struct drm_gman_gem_user {
	__u32 handle;         /* in */
	__u32 pad;
	__u64 paddr;          /* out, physical address of contiguous buffer */
};

#define DRM_GMAN_GEM_NEW         0x00
#define DRM_GMAN_GEM_INFO        0x01
#define DRM_GMAN_GEM_USER        0x02
#define DRM_GMAN_NUM_IOCTLS      0x03

#define DRM_IOCTL_GMAN_GEM_NEW           DRM_IOWR(DRM_COMMAND_BASE + DRM_GMAN_GEM_NEW,  struct drm_gman_gem_new)
#define DRM_IOCTL_GMAN_GEM_INFO          DRM_IOWR(DRM_COMMAND_BASE + DRM_GMAN_GEM_INFO, struct drm_gman_gem_info)
#define DRM_IOCTL_GMAN_GEM_USER          DRM_IOWR(DRM_COMMAND_BASE + DRM_GMAN_GEM_USER, struct drm_gman_gem_user)

#endif