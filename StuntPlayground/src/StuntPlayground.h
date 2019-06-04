#pragma once

#ifndef _STUNTPLAYGROUND_
#define _STUNTPLAYGROUND_

namespace StuntPlayground
{

	// ProgState is used to track the applications general state
	enum ProgState { PS_LOGO, PS_WAITING, PS_PLACING, PS_CHOOSING, PS_PLAYING, PS_REPLAYING };

	// various general types for rigid bodies.
	enum BodyType { BT_ARENA, BT_PROP, BT_CAR, BT_DUMMY, BT_WORLD_LIMIT };

	enum CollisionType { CT_CHASSIS, CT_TIRE };

	enum PrimitiveType
	{
		BOX, ELLIPSOID, CYLINDER, CAPSULE, CONE, CHAMFER_CYLINDER, CONVEX_HULL, TREE
	};

	enum CameraView
	{
		CV_FOLLOW, CV_FRONT, CV_LDOOR, CV_RDOOR, CV_LSIDE, CV_RSIDE, CV_WIDE_FOLLOW, CV_WIDE_LSIDE, CV_WIDE_RSIDE, CV_FREE, CV_SIZE
	};

	enum RotAxis { AXIS_X, AXIS_Y, AXIS_Z };


}	// end NAMESPACE StuntPlayground


#endif	/// _MAINFRAMELISTENER_ guard