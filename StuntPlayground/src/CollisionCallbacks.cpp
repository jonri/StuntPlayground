#include "StuntPlaygroundApplication.h"
#include "CollisionCallbacks.h"


#include <OgreNewt.h>
#include <Ogre.h>
#include "DummyBody.h"
#include "vehicle.h"
#include "Prop.h"
#include "SoundManager.h"


namespace StuntPlayground
{


	int DummyCollisionCallback::userBegin()
	{
		StuntPlayground::DummyBody* dummy;
		Ogre::Real mass;
		Ogre::Vector3 inertia;

		if (m_body0->getType() == StuntPlayground::BT_DUMMY)
		{
			m_body1->getMassMatrix( mass, inertia );
			if (mass == 0.0)
			{
				//this is a static piece, so set the possible attach early!
				dummy = (DummyBody*)m_body0->getUserData();
				if (dummy)
				{
					if (!dummy->getPossibleAttach())
					{
						dummy->setPossibleAttach(m_body1);
						return 0;
					}
				}
			}
		}
		else
		{
			m_body0->getMassMatrix( mass, inertia );
			if (mass == 0.0)
			{
				//this is a static piece, so set the possible attach early!
				dummy = (DummyBody*)m_body1->getUserData();
				if (dummy)
				{
					if (!dummy->getPossibleAttach())
					{
						dummy->setPossibleAttach(m_body0);
						return 0;
					}
				}
			}
		}
				
		return 1;

	}


	int DummyCollisionCallback::userProcess()
	{
		StuntPlayground::DummyBody* dummy;

		// this is the main function, where we save the potential body.
		if (m_body0->getType() == StuntPlayground::BT_DUMMY)
		{
			// m_body1 is the other body.
			dummy = (DummyBody*)m_body0->getUserData();
			if (dummy)
			{
					dummy->setPossibleAttach( m_body1 );
			}
		}
		else
		{
			// m_body0 is the other body.
			dummy = (DummyBody*)m_body1->getUserData();
			if (dummy)
			{
					dummy->setPossibleAttach( m_body0 );
			}
			else
			{
				// this is an error!!
				return 0;
			}
		}

		// never allow collision to actually happen
		return 0;
	}


	int CarArenaCallback::userProcess()
	{
		OgreNewt::Body* car = NULL;
		OgreNewt::Body* arena = NULL;
		OgreNewt::Body* worldlimit = NULL;
		
		//find the bodies.
		if (m_body0->getType() == StuntPlayground::BT_CAR)
			car = m_body0;
		else if (m_body1->getType() == StuntPlayground::BT_CAR)
			car = m_body1;

		if (m_body0->getType() == StuntPlayground::BT_ARENA)
			arena = m_body0;
		else if (m_body1->getType() == StuntPlayground::BT_ARENA)
			arena = m_body1;

		if (m_body0->getType() == StuntPlayground::BT_WORLD_LIMIT)
			worldlimit = m_body0;
		else if (m_body1->getType() == StuntPlayground::BT_WORLD_LIMIT)
			worldlimit = m_body1;

		
		if (!car)
		{
			return 0;
		}

		Car* thecar = (Car*)car->getUserData();
		thecar->incCollisions();
		

		if ((car) && (worldlimit))
		{
			Ogre::Quaternion orient;
			Ogre::Matrix3 rot;
			rot.FromEulerAnglesXYZ(Ogre::Degree(0), Ogre::Degree(90), Ogre::Degree(0));
			orient.FromRotationMatrix( rot );
			thecar->getChassisBody()->setPositionOrientation( Ogre::Vector3(-100,-2,45), orient );
			thecar->getChassisBody()->setVelocity( Ogre::Vector3( Ogre::Vector3::ZERO ) );
			thecar->getChassisBody()->setOmega( Ogre::Vector3( Ogre::Vector3::ZERO ) );
			thecar->reset();
			return 0;
		}

		unsigned int ID = getContactFaceAttribute();

		Ogre::Vector3 pos, norm;
		getContactPositionAndNormal( pos, norm );
		Ogre::Real normspeed = getContactNormalSpeed();

		// hide particles
		Application::getSingleton().getDirtParticles()->getEmitter(0)->setEmissionRate(0.0);
		Application::getSingleton().getSparkParticles()->getEmitter(0)->setEmissionRate(0.0);

		// now check which part of the car is hitting.
		if (getBodyCollisionID( car ) == StuntPlayground::CT_CHASSIS)
		{
				// sparks!
			if (ID != mGroundID)
			{
				//sparks!
				Application::getSingleton().getSparkParticlesNode()->setPosition(pos);
				Application::getSingleton().getSparkParticlesNode()->lookAt(pos+(norm*2.0),Ogre::Node::TS_WORLD);
				Application::getSingleton().getSparkParticles()->getEmitter(0)->setEmissionRate( 50.0*Application::getSingleton().mCar->getChassisBody()->getVelocity().length() );
			}

			//chassis is colliding, so set the friction, and make the collision as soft as possible.
			setContactKineticFrictionCoef( 0.5, 0 );
			setContactKineticFrictionCoef( 0.5, 1 );
			setContactSoftness( 0.95 );
			setContactElasticity( 0.03 );

			
			Ogre::Real vol = (normspeed * 10) - 8.0;
			if (mCarSnd)
			{
				Ogre::Real time = mCarSnd->getTimeSinceLastPlay();
				if (time > 0.1)
					mCarSnd->play( (int)vol );
			}
		}
		else
		{
			//tire is colliding.
			setContactSoftness( 0.5 );
			setContactElasticity( 0.5 );			
				
			// do a quick check for what part of the arena we're driving on.
			if ((ID == mRoadID) || (ID == mRoadCheckerID))
			{
				//on the road, good friction.
				setContactStaticFrictionCoef( 1.7, 0 );
				setContactStaticFrictionCoef( 1.7, 1 );

				setContactKineticFrictionCoef( 1.0, 0 );
				setContactKineticFrictionCoef( 1.0, 1 );

				// car is on the ground!
				thecar->setTireState( Car::TS_ONROAD );
			}
			else
			{
				if (getContactFaceAttribute() == mGroundID)
				{
					Ogre::Vector3 pos, norm;

					getContactPositionAndNormal( pos, norm );
					Application::getSingleton().getDirtParticlesNode()->setPosition( pos );

					Ogre::Vector3 vel = Application::getSingleton().mCar->getChassisBody()->getVelocity();
					if (vel.length() > 3.0)
					{
						Application::getSingleton().getDirtParticles()->getEmitter(0)->setEmissionRate( vel.length() * 1.0 );
					}

					// on the main ground.
					setContactStaticFrictionCoef( 1.0, 0 );
					setContactStaticFrictionCoef( 1.0, 1 );

					setContactKineticFrictionCoef( 0.7, 0 );
					setContactKineticFrictionCoef( 0.7, 1 );

					if (thecar->getTireState() != Car::TS_ONROAD)
						thecar->setTireState( Car::TS_ONDIRT );
				}
				else
				{
					// somewhere else!
					setContactStaticFrictionCoef( 1.2, 0 );
					setContactStaticFrictionCoef( 1.2, 1 );

					setContactKineticFrictionCoef( 0.9, 0 );
					setContactKineticFrictionCoef( 0.9, 1 );

					//if (thecar->getTireState() != Car::TS_ONROAD)
					//	thecar->setTireState( Car::TS_ONPROP );
				}
			}
			
		}

		return 1;
	}


	int PropCollision::userBegin()
	{ 
		// clear the values
		mNormSpeed = mTangSpeed = 0.0;
		mPosition = Ogre::Vector3::ZERO;
			
		return 1; 
	}

	int PropCollision::userProcess()
	{ 
		//is this this collision bigger than the previou one?
		Ogre::Real norm, tang0, tang1;
		Ogre::Vector3 pos, normal;

		norm = getContactNormalSpeed();
		tang0 = getContactTangentSpeed(0);
		tang1 = getContactTangentSpeed(1);
		getContactPositionAndNormal( pos, normal );
			
		//check.
		if (tang0 > mTangSpeed) { mTangSpeed = tang0; mPosition = pos; }
		if (tang1 > mTangSpeed) { mTangSpeed = tang1; mPosition = pos; }

		if (norm > mNormSpeed) { mNormSpeed = norm; mPosition = pos; }

		return 1;		
	}

	void PropCollision::userEnd()
	{
		
		// true = straight hit.  false = sliding collision.
		bool hit = true;
		Ogre::Real speed = mNormSpeed;

		if (mNormSpeed < mTangSpeed) { hit = false; speed = mTangSpeed; }

		// collision actually happened?
		if (speed == 0.0) { return; }

		// play sound based on materials!
		Prop* prop0 = NULL;
		Prop* prop1 = NULL;

		Sound* snd1 = NULL;
		Sound* snd2 = NULL;

		if (m_body0->getType() == StuntPlayground::BT_PROP)
		{

			prop0 = (Prop*)m_body0->getUserData();
			snd1 = SoundManager::getSingleton().getSound( prop0->getHitSound() );
		}

		if (m_body1->getType() == StuntPlayground::BT_PROP)
		{
			prop1 = (Prop*)m_body1->getUserData();
            snd2 = SoundManager::getSingleton().getSound( prop1->getHitSound() );
		}

		// PLAY THE SOUND, volume based on contact power.
		Ogre::Real vol = (speed * 10) - 8.0;

/*
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -S-  speed: "+Ogre::StringConverter::toString(speed)+
				" calculated vol: "+Ogre::StringConverter::toString(vol)+
				" position: "+Ogre::StringConverter::toString(mPosition));
#endif	// _SP_DEBUG_LOG
*/				
			
		if ((snd1) || (snd2))
		{
			if (vol > 0)
			{
				if (snd1)
				{
					Ogre::Real time = snd1->getTimeSinceLastPlay();
					if (time > 0.1)
					{
						snd1->play( (int)vol );
					}

				}

				if (snd2)
				{
					if (snd2 == snd1)
						return;
					
					Ogre::Real time = snd2->getTimeSinceLastPlay();
					if (time > 0.1)
					{
						snd2->play( (int)vol );
					}
				}
				
			}
		}
	
	}




	int CarPropCollision::userBegin()
	{ 
		// clear the values
		mNormSpeed = mTangSpeed = 0.0;
		mPosition = Ogre::Vector3::ZERO;
			
		return 1; 
	}

	int CarPropCollision::userProcess()
	{ 
		//is this this collision bigger than the previou one?
		Ogre::Real norm, tang0, tang1;
		Ogre::Vector3 pos, normal;

		norm = getContactNormalSpeed();
		tang0 = getContactTangentSpeed(0);
		tang1 = getContactTangentSpeed(1);
		getContactPositionAndNormal( pos, normal );

		// hide particles
		Application::getSingleton().getSparkParticles()->getEmitter(0)->setEmissionRate(0.0);
		
		bool body;
		OgreNewt::Body* thecar;

		if (m_body0->getType() == StuntPlayground::BT_CAR)
		{
			thecar = m_body0;
		}
		else
		{
			if (m_body1->getType() == StuntPlayground::BT_CAR)
			{
				thecar = m_body1;
			}
			else
			{
				//error!
#ifdef _SP_DEBUG_LOG
				Application::getSingleton().mDebugLog->logMessage(" -S- CarPropCollision:: cannot find car!!");
#endif	// _SP_DEBUG_LOG
				return 0;
			}
		}

		if (getBodyCollisionID( thecar ) == StuntPlayground::CT_CHASSIS)
		{
			body = true;

			Ogre::Vector3 pos, norm;
			
			getContactPositionAndNormal( pos, norm );
			Ogre::Real tang = getContactTangentSpeed(0);
			if (abs(tang) < abs(getContactTangentSpeed(1))) { tang = getContactTangentSpeed(1); }
			
			Application::getSingleton().getSparkParticlesNode()->setPosition(pos);
			Application::getSingleton().getSparkParticlesNode()->lookAt(pos+(norm*2.0),Ogre::Node::TS_WORLD);
			Application::getSingleton().getSparkParticles()->getEmitter(0)->setEmissionRate( 30.0*abs(tang) );

		}
		else
		{
			setContactStaticFrictionCoef( 1.2, 0 );
			setContactStaticFrictionCoef( 1.2, 1 );

			setContactKineticFrictionCoef( 0.9, 0 );
			setContactKineticFrictionCoef( 0.9, 1 );

			body = false;
			Car* car = (Car*)thecar->getUserData();
			car->setTireState( Car::TS_ONPROP );
		}

		//check.
		if (tang0 > mTangSpeed) { mTangSpeed = tang0; mPosition = pos; mBodyHit = body; }
		if (tang1 > mTangSpeed) { mTangSpeed = tang1; mPosition = pos; mBodyHit = body; }

		if (norm > mNormSpeed) { mNormSpeed = norm; mPosition = pos; mBodyHit = body; }

		return 1;		
	}

	void CarPropCollision::userEnd()
	{

		// true = straight hit.  false = sliding collision.
		bool hit = true;
		Ogre::Real speed = mNormSpeed;

		if (mNormSpeed < mTangSpeed) { hit = false; speed = mTangSpeed; }

		// collision actually happened?
		if (speed == 0.0) { return; }

		// play sound based on materials!
		Prop* theprop = NULL;
		Car* thecar = NULL;

		Sound* propsnd = NULL;
		Sound* carsnd = NULL;

		if (m_body0->getType() == StuntPlayground::BT_PROP)
		{
			// 0 = prop, 1 = car.
			theprop = (Prop*)m_body0->getUserData();
			thecar = (Car*)m_body1->getUserData();
		}
		else
		{
			if (m_body1->getType() == StuntPlayground::BT_PROP)
			{
				theprop = (Prop*)m_body1->getUserData();
				thecar = (Car*)m_body0->getUserData();
				thecar->incCollisions();
			}
			else
			{
				// error!
#ifdef _SP_DEBUG_LOG
				Application::getSingleton().mDebugLog->logMessage(" -S- CarPropCallback: error finding bodies!");
#endif	// _SP_DEBUG_LOG
				return;
			}
		}


		// is this a body hit?
		if (!mBodyHit)
			return;


		// get prop hit sound.
		propsnd = SoundManager::getSingleton().getSound( theprop->getHitSound() );

		Ogre::Real vol = (speed * 10) - 8.0;

		// car sound is based on speed.
		// (but not yet)
		carsnd = SoundManager::getSingleton().getSound( "CarCrash1" );

		if ((propsnd) && (vol > 0))
		{
			Ogre::Real time = propsnd->getTimeSinceLastPlay();
			if (time > 0.1)
				propsnd->play( (int)vol );
		}

		vol *= 1.5;

		if ((carsnd) && (vol > 0))
		{
			Ogre::Real time = carsnd->getTimeSinceLastPlay();
			if (time > 0.1)
				carsnd->play( (int)vol );
		}

	}



}	// end NAMESPACE StuntPlayground