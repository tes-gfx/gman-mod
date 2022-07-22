#ifndef _GMAN_GEM_H_
#define _GMAN_GEM_H_


#include <drm/drmP.h>
#include <drm/drm_gem_cma_helper.h>


struct drm_gem_object *gman_gem_new(struct drm_device *dev, size_t unaligned_size, dma_addr_t *paddr);
int gman_gem_mmap_offset(struct drm_gem_object *obj, u64 *offset);


#endif /* _GMAN_GEM_H_ */