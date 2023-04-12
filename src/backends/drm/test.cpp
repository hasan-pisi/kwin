#include "test.h"

#include <xf86drm.h>

int doDrmIoctl(int fd, unsigned long request, void *arg)
{
    return drmIoctl(fd, request, arg);
}
