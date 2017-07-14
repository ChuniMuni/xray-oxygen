#pragma once

#include "../../3rd party/ode/include/ode/common.h"
#include "../ExtendedGeom.h"
#include "dTriColliderMath.h"

extern xr_vector< flags8 >			gl_cl_tries_state	;

#define CONTACT(Ptr, Stride) ((dContactGeom*) (((char*)Ptr) + (Stride)))
#ifdef _M_X64
#define SURFACE(Ptr, Stride) ((dSurfaceParameters*) (((char*)Ptr) + (Stride+sizeof(dContactGeom))))
#else
#define SURFACE(Ptr, Stride) ((dSurfaceParameters*) (((char*)Ptr) + (Stride - offsetof(dContact, geom) + offsetof(dContact, surface))))
#endif
#define NUMC_MASK (0xffff)

#define M_SIN_PI_3		REAL(0.8660254037844386467637231707529362)
#define M_COS_PI_3		REAL(0.5000000000000000000000000000000000)
