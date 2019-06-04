#include "StuntPlaygroundApplication.h"

#include "DummyBody.h"



namespace StuntPlayground
{

	DummyBody::DummyBody( OgreNewt::World* world, const OgreNewt::MaterialID* mat, Ogre::Vector3& size, Ogre::SceneManager* mgr ) : m_Size(size)
	{
		mGoalPos = Ogre::Vector3::ZERO;
		mGoalOrient = Ogre::Quaternion::IDENTITY;

		mSpringConst = mSpringDamp = 0.0;

		//temp collision
		OgreNewt::Collision* col = new OgreNewt::CollisionPrimitives::Box( world, size );
		m_Body = new OgreNewt::Body( world, col );
		delete col;
		m_Body->setUserData( this );
		m_Body->setContinuousCollisionMode( 1 );

		//setup basic visual stuff
		Ogre::SceneNode* node = mgr->getRootSceneNode()->createChildSceneNode();
		Ogre::Entity* ent = mgr->createEntity("DummyEntity","box.mesh");
		node->attachObject( ent );
		node->setScale( size );
		ent->Entity::setMaterialName("Dummy/OK");
				
		m_Body->attachToNode( node );

		Ogre::Real mass = 10.0;
		Ogre::Vector3 inertia = OgreNewt::MomentOfInertia::CalcBoxSolid( mass, size );
		m_Body->setMassMatrix( mass, inertia );

		mSpringConst = 0.5;
		m_Body->setLinearDamping( 0.5 );
		m_Body->setAngularDamping( Ogre::Vector3(0.9,0.9,0.9) );

		m_Body->setMaterialGroupID( mat );
		m_MyMat = mat;

		m_Body->setType( (int)StuntPlayground::BT_DUMMY );

		// rotation ring.
		mRotNode = node->createChildSceneNode();

		ent = mgr->createEntity("RotRingEntity", "rot_ring.mesh");
		ent->setCastShadows(false);
		mRotNode->attachObject( ent );


		m_Attached = NULL;
		m_AttachedMat = NULL;
		m_PossibleAttach = NULL;
	
	}

	void DummyBody::setCollision( const OgreNewt::Collision* newcollision, Ogre::Vector3 nodescale )
	{
		m_Body->setCollision( newcollision );
		Ogre::Real mass;
		Ogre::Vector3 inertia;
		m_Body->getMassMatrix( mass, inertia );
			
		Ogre::AxisAlignedBox box = newcollision->getAABB();
		Ogre::Vector3 rotscale = box.getMaximum() - box.getMinimum();
		if ((rotscale.x > rotscale.y) && (rotscale.x > rotscale.z)) { rotscale.y = rotscale.x; rotscale.z = rotscale.x; }
		if ((rotscale.y > rotscale.x) && (rotscale.y > rotscale.z)) { rotscale.x = rotscale.y; rotscale.z = rotscale.y; }
		if ((rotscale.z > rotscale.y) && (rotscale.z > rotscale.y)) { rotscale.x = rotscale.z; rotscale.y = rotscale.z; }
		rotscale.y = 1.0;

		mRotNode->setScale( rotscale );
		//m_Body->setMassMatrix( mass, OgreNewt::MomentOfInertia::CalcBoxSolid( mass, Ogre::Vector3( box.getMaximum()-box.getMinimum())) );

		m_Body->getOgreNode()->setScale( nodescale );
	}

	void DummyBody::attachBody( OgreNewt::Body* body, const OgreNewt::MaterialID* tempMat )
	{
		m_Attached = body;

		//remove any existing callbacks (force)
		m_Attached->removeForceAndTorqueCallback();

		//get attached body' material, and use that!
		m_AttachedMat = m_Attached->getMaterialGroupID();
			
		//then make the attached our material!
		m_Attached->setMaterialGroupID( m_MyMat );

		// set the material for this body to the new material.
		m_Body->setMaterialGroupID( tempMat );
			

		// check the mass value of the body, if > 0, use the actual body for the shape!  if not, use bounding box.
		Ogre::Real mass;
		Ogre::Vector3 inertia;

		m_Attached->getMassMatrix( mass, inertia );
		if (mass == 0.0)
		{
			//this body is static, so use the bounding box for placement!
			Ogre::AxisAlignedBox box = m_Attached->getCollision()->getAABB();

			//make the collision, a custom convex hull.
			Ogre::Vector3 min, max;
			Ogre::Vector3* verts = new Ogre::Vector3[8];

			min = box.getMinimum() + Ogre::Vector3(0.1,0.1,0.1);
			max = box.getMaximum() - Ogre::Vector3(0.1,0.1,0.1);

			Ogre::Vector3 pos;
			pos = max - ((max-min)/2.0);

			OgreNewt::Collision* newcol = new OgreNewt::CollisionPrimitives::Box(body->getWorld(), max-min, Ogre::Quaternion::IDENTITY, pos  );
			setCollision( newcol );

			delete newcol;
		}
		else
		{
			//this body is dynamic, so we can use the collision data for the body as our shape for the dummy!
			setCollision( m_Attached->getCollision() );
		}

			
	}


	void DummyBody::detach()
	{
		if (!m_Attached)
			return;

		// detach from the existing body, and set to a default size.
		// set the forcecallback for the attached body.
		m_Attached->setStandardForceCallback();

		// make sure it's not frozen.
		m_Attached->unFreeze();

		// remove any existing forces!
		m_Attached->setVelocity( Ogre::Vector3(0,0,0) );
		m_Attached->setOmega( Ogre::Vector3(0,0,0) );

		//restore material for attached body
		m_Attached->setMaterialGroupID( m_AttachedMat );

		// restore our own material
		m_Body->setMaterialGroupID( m_MyMat );

		m_Attached = NULL;
		m_AttachedMat = NULL;

		setSize( Ogre::Vector3(1,1,1) );
	}

	void DummyBody::setSize( Ogre::Vector3& size )
	{
		//re-do the collision!
		OgreNewt::Collision* col = new OgreNewt::CollisionPrimitives::Box( m_Body->getWorld(), size );
		setCollision( col, size );
		delete col;
	}

	void DummyBody::setGoalPositionOrientation( Ogre::Vector3& pos, Ogre::Quaternion& orient )
	{
		mGoalPos = pos;
		mGoalOrient = orient;
	}

	void DummyBody::getGoalPositionOrientation( Ogre::Vector3& pos, Ogre::Quaternion& orient )
	{
		pos = mGoalPos;
		orient = mGoalOrient;
		//make sure the body isn't frozen:
		m_Body->unFreeze();
	}

	void DummyBody::attachToCurrentPossible( const OgreNewt::MaterialID* material ) 
	{ 
		attachBody( m_PossibleAttach, material ); 
		setPossibleAttach(NULL); 
		
		//now line up with the body.
		Ogre::Vector3 attachpos;
		Ogre::Quaternion attachorient;

		m_Attached->getPositionOrientation( attachpos, attachorient );

		// align the body...
		m_Body->Body::setPositionOrientation( attachpos, attachorient );
		setGoalPositionOrientation( attachpos, attachorient );

	}

	void DummyBody::setRotRingOrient( StuntPlayground::RotAxis axis )
	{
		Ogre::Quaternion orient = Ogre::Quaternion::IDENTITY;
		Ogre::Vector3 theaxis;

		if (axis == StuntPlayground::AXIS_X)
		{
			orient.FromAngleAxis( Ogre::Degree(90), Ogre::Vector3::UNIT_Z );
		}

		if (axis == StuntPlayground::AXIS_Y)
		{
			orient.FromAngleAxis( Ogre::Degree(0), Ogre::Vector3::UNIT_Y );
		}

		if (axis == StuntPlayground::AXIS_Z)
		{
			orient.FromAngleAxis( Ogre::Degree(90), Ogre::Vector3::UNIT_X );
		}

		mRotNode->setOrientation( orient );
	}






}	// end NAMESPACE StuntPlayground