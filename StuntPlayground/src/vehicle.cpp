#include "StuntPlayground.h"
#include ".\StuntPlaygroundApplication.h"
#include "vehicle.h"
#include "RigidBodyLoader.h"
#include "WorldRecorder.h"
#include "SoundManager.h"
#include <tinyxml.h>

namespace StuntPlayground
{

	Car::MyTire::MyTire(Ogre::SceneManager* mgr, Ogre::SceneNode* parentnode, OgreNewt::Vehicle* vehicle, Ogre::Quaternion localorient, Ogre::Vector3 localpos, Ogre::Vector3 pin,
				Ogre::Real mass, Ogre::Real width, Ogre::Real radius, Ogre::Real susShock, Ogre::Real susSpring, Ogre::Real susLength, int colID)
				: Tire( vehicle, localorient, localpos, pin, mass, width, radius, susShock, susSpring, susLength, colID )
	{
		mSceneMgr = mgr;

		Ogre::SceneNode* node = parentnode->createChildSceneNode();
		attachToNode( node );
		node->setScale( Ogre::Vector3( radius, radius, width ) );

		mRadius = radius;

		mRecObj = NULL;
	}


	Car::MyTire::~MyTire()
	{
		// delete the visual object for this tire.
		while (getOgreNode()->numAttachedObjects() > 0)
		{
			Ogre::MovableObject* ent = getOgreNode()->detachObject(static_cast<unsigned short>(0));
			mSceneMgr->destroyEntity( (Ogre::Entity*)ent );
		}
		getOgreNode()->getParentSceneNode()->removeAndDestroyChild( getOgreNode()->getName() );
		
	}

	Car::Car() : OgreNewt::Vehicle()
	{
	}

	Car::Car( OgreNewt::World* world, Ogre::SceneNode* parentnode, Ogre::SceneManager* mgr, Ogre::String filename, const OgreNewt::MaterialID* mat ) : OgreNewt::Vehicle(), mSceneMgr(mgr)
	{
		_init( world, parentnode, mgr, filename, mat );
	}


	Car::~Car()
	{
		std::vector<Car::MyTire*> tires;

		// delete tire entities.
		for (Car::MyTire* tire = (Car::MyTire*)getFirstTire(); tire; tire = (Car::MyTire*)getNextTire( tire ) )
		{
			tires.push_back( tire );
		}

		for (int i=0; i<tires.size(); i++)
		{
			Car::MyTire* tire = tires[i];
			delete tire;
		}

		// delete the scene node.
		while (m_chassis->getOgreNode()->numAttachedObjects() > 0)
		{
			Ogre::MovableObject* obj = m_chassis->getOgreNode()->detachObject(static_cast<unsigned short>(0));
			mSceneMgr->destroyEntity( (Ogre::Entity*)obj );
		}

		Ogre::SceneNode* todestroy = (Ogre::SceneNode*)m_chassis->getOgreNode()->getParent()->removeChild( m_chassis->getOgreNode()->getName() );
		mSceneMgr->SceneManager::destroySceneNode( todestroy->getName() );

		destroy();
		
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -VS- Car Destroyed.");
#endif	// _SP_DEBUG_LOG

	}

	void Car::_init( OgreNewt::World* world, Ogre::SceneNode* parentnode, Ogre::SceneManager* mgr, Ogre::String filename, const OgreNewt::MaterialID* mat )
	{
		mTakeOffPos = Ogre::Vector3::ZERO;
		Ogre::Vector3 comOffset = Ogre::Vector3::ZERO;
		mNumFlips = 0;
		mLastY = Ogre::Vector3::ZERO;
		mCollisions = 0;

		mTireState = TS_ONROAD;
		
		// temp stuff
		mFilename = filename;

		TiXmlDocument doc;

		doc.LoadFile( (std::string)filename );

		// find the vehicle in the file!!
		TiXmlElement* vehicle = doc.FirstChildElement( "vehicle" );
		if (!vehicle)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'vehicle' element in "+filename );
#endif	// _SP_DEBUG_LOG
			Application::getSingleton().showErrorMessage("Error loading vehicle!!", false);
			return;
		}

		// vehicle name...
		mName = (Ogre::String)vehicle->Attribute( "name" );

		TiXmlElement* elem = vehicle->FirstChildElement( "chassis" );

		// find the rigid body for the chassis!
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'chassis' element in "+filename );
#endif	// _SP_DEBUG_LOG
			return;
		}
		Ogre::String rigidfile = (Ogre::String)elem->Attribute( "filename" );

		// possible offset for center of mass.
		elem = elem->FirstChildElement("CenterOfMass");
		if (elem)
		{
			comOffset = Ogre::StringConverter::parseVector3( elem->Attribute( "offset" ) );
		}

		// setup the scene node, etc.
		Ogre::SceneNode* vehiclenode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		// make the rigid body!
		OgreNewt::Body* body = StuntPlayground::loadRigidBody( rigidfile, mName+"Entity", world, vehiclenode, mSceneMgr, StuntPlayground::CT_CHASSIS );

		// now initialize the vehicle.
		init( body, Ogre::Vector3(0,1,0) );
		m_chassis->setMaterialGroupID( mat );
		if (comOffset != Ogre::Vector3::ZERO)
			m_chassis->setCenterOfMass( comOffset );

		mLookNode = m_chassis->getOgreNode()->createChildSceneNode();

		// okay, load the torque curve.
		elem = vehicle->FirstChildElement( "torque_curve" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'torque_curve' element in "+filename );
#endif	// _SP_DEBUG_LOG
			return;
		}

		// loop through the points on the curve.
		mMinRPM = 15000.0;
		mMaxRPM = 0.0;

		TiXmlElement* point = elem->FirstChildElement( "point" );
		while (point)
		{
			TorquePoint temp;

			temp.rpm = Ogre::StringConverter::parseReal( (Ogre::String)point->Attribute("rpm") );
			temp.torque = Ogre::StringConverter::parseReal( (Ogre::String)point->Attribute("torque") );

			if (temp.rpm < mMinRPM) { mMinRPM = temp.rpm; }
			if (temp.rpm > mMaxRPM) { mMaxRPM = temp.rpm; }

			mTorqueCurve.push_back( temp );

			// get the next point.
			point = point->NextSiblingElement( "point" );
		}

		mRPMRange = mMaxRPM - mMinRPM;

		// time to get the gears.
		elem = vehicle->FirstChildElement( "gearbox" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'gearbox' element in "+filename );
#endif	// _SP_DEBUG_LOG
			return;
		}

		// loop through the gears
		TiXmlElement* gear = elem->FirstChildElement( "gear" );

		while (gear)
		{
			Ogre::Real ratio = Ogre::StringConverter::parseReal( (Ogre::String)gear->Attribute("ratio") );

			mGears.push_back( ratio );

			//get the next gear.
			gear = gear->NextSiblingElement( "gear" );
		}

		// engine settings
		mEngineMass = 100.0;
		mEngineSnd = NULL;

		elem = vehicle->FirstChildElement( "engine" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR! cannot find 'engine' in "+filename );
#endif	//_SP_DEBUG_LOG
		}
		else
		{
			mEngineMass = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("rotating_mass") );
			mEngineSnd = SoundManager::getSingleton().getSound( (std::string)elem->Attribute("sound") );
			mEngineBaseRPM = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("base_rpm") );
			mEngineScale = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("scale_factor") );

			mEngineSnd->setLoop( true );
		}

		// shift delay
		mShiftDelay = 0.05;
		elem = vehicle->FirstChildElement( "shift" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR! cannot find 'gear' in "+filename );
#endif	// _SP_DEBUG_LOK
		}
		else
		{
			mShiftDelay = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "delay" ) );
			mShiftTime = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "time" ) );
		}


		// differential ratio.
		mDifferentialRatio = 1.0f;
		elem = vehicle->FirstChildElement( "differential" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'differential ratio' in"+filename );
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			mDifferentialRatio = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "ratio" ) );
		}

		// transmission efficiency
		mTransmissionEfficiency = 1.0f;
		elem = vehicle->FirstChildElement( "transmission" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'transmission efficiency' in"+filename );
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			mDifferentialRatio = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "efficiency" ) );
		}

		// drag coefficient
		mDragCoefficient = 1.0f;
		elem = vehicle->FirstChildElement( "drag" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'drag coefficient' in"+filename );
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			mDragCoefficient = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "coefficient" ) );
		}

		// brake strength
		mBrakeStrength = 1000.0f;
		elem = vehicle->FirstChildElement( "brake" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'brake strength' in"+filename );
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			mBrakeStrength = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "strength" ) );
		}

		// steering settings.
		mSteerMaxAngle = Ogre::Radian(0.0);
		mSteerSpeed = Ogre::Radian(0.0);
		mSteerLossPercent = 0.0;
		mSteerFromSpeed = 0.0;
		mSteerToSpeed = 0.0;

		elem = vehicle->FirstChildElement( "steering" );
		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'steering' in"+filename );
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			mSteerMaxAngle = Ogre::Radian(Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("angle") ) );
			mSteerSpeed = Ogre::Radian(Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("speed") ) );
			mSteerLossPercent = (Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("percent_drop") ) / 100.0);
			mSteerFromSpeed = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("from_speed") );
			mSteerToSpeed = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute("to_speed") );
		}
		mSteerSpeedRange = mSteerToSpeed - mSteerFromSpeed;

		// finally, go through the tires.
		TiXmlElement* tire_elem = vehicle->FirstChildElement( "tire" );

		if (!tire_elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- ERROR! no `tire` found in "+filename );
#endif	// _SP_DEBUG_LOG
			return;
		}

		int tirecount = 0;

		// loop through the tires
		while (tire_elem)
		{
			Ogre::Quaternion orient;
			Ogre::Vector3 rot, pos, pin;
			Ogre::Real rad, height, mass;
			Ogre::Real damp, spring, length;
			Ogre::Real grip;
			int steer, drive;
			Ogre::SceneNode* node = mSceneMgr->getRootSceneNode();

			// get visual mesh.
			elem = tire_elem->FirstChildElement( "visual" );
			if (!elem)
			{
#ifdef _SP_DEBUG_LOG
				Application::getSingleton().mDebugLog->logMessage(" -V- ERROR! cannot find 'visual' in <tire> in "+filename );
#endif	// _SP_DEBUG_LOG
			}
			else
			{
				Car::MyTire* tire;

				// setup the mesh...
				Ogre::String meshfile = (Ogre::String)elem->Attribute( "mesh" );

				Ogre::Entity* ent = mSceneMgr->createEntity( mName+"_Tire"+Ogre::StringConverter::toString(tirecount++), meshfile );
				ent->setNormaliseNormals( true );

				// set defaults in case data is missing
				orient = Ogre::Quaternion::IDENTITY;
				rot = pos = pin = Ogre::Vector3::ZERO;
				rad = height = mass = 1.0;
				damp = spring = length = 1.0;
				grip = 1.0;
				steer = drive = 0;

				// position, rotation, pin
				elem = tire_elem->FirstChildElement( "location" );
				if (!elem)
				{
#ifdef _SP_DEBUG_LOG
					Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find 'location' in <tire> in "+filename );
#endif	// _SP_DEBUG_LOG
				}
				else
				{
					rot = Ogre::StringConverter::parseVector3( (Ogre::String)elem->Attribute( "orient" ) );
					Ogre::Matrix3 mat;
					mat.FromEulerAnglesXYZ( Ogre::Degree(rot.x), Ogre::Degree(rot.y), Ogre::Degree(rot.z) );
					orient.FromRotationMatrix( mat );
					pos = Ogre::StringConverter::parseVector3( (Ogre::String)elem->Attribute( "pos" ) );
					pin = Ogre::StringConverter::parseVector3( (Ogre::String)elem->Attribute( "pin" ) );
				}

				// size
				elem = tire_elem->FirstChildElement( "size" );
				if (!elem)
				{
#ifdef _SP_DEBUG_LOG
					Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find `size` in <tire> in "+filename );
#endif	// _SP_DEBUG_LOG
				}
				else
				{
					rad = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "radius" ) );
					height = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "height" ) );
					mass = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "mass" ) );
				}

				// suspension
				elem = tire_elem->FirstChildElement( "suspension" );
				if (!elem)
				{
#ifdef _SP_DEBUG_LOG
					Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find `size` in <tire> in "+filename );
#endif	// _SP_DEBUG_LOG
				}
				else
				{
					damp = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "damp" ) );
					spring = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "spring" ) );
					length = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "length" ) );
				}

				// settings
				elem = tire_elem->FirstChildElement( "settings" );
				if (!elem)
				{
#ifdef _SP_DEBUG_LOG
					Application::getSingleton().mDebugLog->logMessage(" -V- ERROR!  cannot find `settings` in <tire> in "+filename );
#endif	// _SP_DEBUG_LOG
				}
				else
				{
					grip = Ogre::StringConverter::parseReal( (Ogre::String)elem->Attribute( "grip" ) );
					steer = Ogre::StringConverter::parseInt( (Ogre::String)elem->Attribute( "steer" ) );
					drive = Ogre::StringConverter::parseInt( (Ogre::String)elem->Attribute( "drive" ) );
				}

				// make the tire!
				tire = new Car::MyTire( mSceneMgr, node, this, orient, pos, pin, mass, height, rad, damp, spring, length, StuntPlayground::CT_TIRE );

				tire->setDrive( drive );
				tire->setSteer( steer );
				tire->setGrip( grip );

				tire->getOgreNode()->attachObject( ent );
				

			}

			// get the next tire.
			tire_elem = tire_elem->NextSiblingElement( "tire" );
		}


		mLastOmega = 0.0;
		mBrakesOn = false;
		mSteering = mThrottle = 0.0f;

		mCurrentGear = 1;

		mRecObject = NULL;

		// screech sound
		mScreechSnd = SoundManager::getSingleton().getSound("TireScreech");

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -V- Car Created: "+ filename );
#endif	// _SP_DEBUG_LOG
	}

	
	void Car::setup()
	{
		// setup the chassis a bit.
		m_chassis->setUserData( this );
		m_chassis->setType( StuntPlayground::BT_CAR );
		m_chassis->setStandardForceCallback();
		m_chassis->setCustomTransformCallback( Car::Transform );
		m_chassis->setAutoFreeze( 0 );
		m_chassis->setLinearDamping( 0.001f );
	}

	// user callback
	void Car::userCallback()
	{
		// singletons...
		Application& app = Application::getSingleton();
		SoundManager& sMgr = SoundManager::getSingleton();

		// increment the internal car timer
		Ogre::Real elap = m_chassis->getWorld()->getTimeStep();
		mTimer += elap;
		mChangeTimer += elap;
		
		// loop through wheels, and get the average wheel omega.
		int count = 0;
		Ogre::Real omega = 0.0;
		bool omegaIsPositive = true;

		int wheels_on_ground = 0;

#ifdef _SP_DEBUG_LOG
		app.mDebugLog->logMessage("---------");
		app.mDebugLog->logMessage("mThrottle:" + Ogre::StringConverter::toString( mThrottle ) + 
			" | mBrake:" + Ogre::StringConverter::toString( mBrakesOn ) );
#endif

		Ogre::Real originalThrottle = mThrottle;
		Ogre::Real originalSteering = mSteering;
		bool originalBrakes = mBrakesOn;

		for (Car::MyTire* tire = (Car::MyTire*)getFirstTire(); tire; tire = (Car::MyTire*)getNextTire( tire ) )
		{
			// get omega
			//if (tire->getDrive())
			//{ 
				//count this tire.
				omega += tire->getOmega();;
				count++;
			//}
			wheels_on_ground += (tire->isAirBorne()) ? 0 : 1;
		}

		if (count>0)
			omega /= (Ogre::Real)count;

		omegaIsPositive = (omega > 0.0f) ? true : false;
		omega = Math::Abs( omega );


		//////////////////////
		// VEHICLE STATE
		if (wheels_on_ground == 4)
		{
			mState = Car::VS_4ONFLOOR;
		}
		else
		{
			if (wheels_on_ground == 2)
			{
				if (mCollisions > 0)
				{
					mState = Car::VS_4ONFLOOR;
				}
				else
				{
					mState = Car::VS_2WHEELS;
				}
			}
			else
			{
				if (wheels_on_ground == 0)
				{
					if (mCollisions == 0)
					{
						mState = Car::VS_AIRBORNE;
					}
					else
					{
						mState = Car::VS_UPSIDEDOWN;
					}
				}
				else
				{
					// number of wheels on the ground is 1 or 3
					if (wheels_on_ground < 2)
						mState = Car::VS_AIRBORNE;
					else
						mState = Car::VS_4ONFLOOR;
				}
			}
		}

		if (mState != mOldState)
		{
			if (mCheckingChange)
			{
				if (mChangeTimer > 0.1f)
				{
					// allow the change.
					if (mState == Car::VS_AIRBORNE)
					{
						Ogre::Quaternion orient;
						m_chassis->getPositionOrientation(mTakeOffPos,orient);
						Ogre::Matrix3 rot;
						orient.ToRotationMatrix(rot);
						mLastY = rot.GetColumn(1);
					}
					if (mOldState == Car::VS_AIRBORNE)
					{
						// just landed!
						bool show = false;
						Ogre::String mess = "";

						Ogre::Real time = mTimer;

						if (time > 1.0)
							show = true;
						
						if (time > 4.0)
							mess = "BIG ";
						
						Ogre::Vector3 pos;
						Ogre::Quaternion orient;
						m_chassis->getPositionOrientation( pos, orient );

						Ogre::Real length = (pos - mTakeOffPos).length();

						if (length > 40.0)
							mess = mess + "LONG ";
						
						if (mNumFlips > 5)
							mess = mess + "CRAZY ";
						
						if ((show) && (Application::getSingleton().getProgState() == StuntPlayground::PS_PLAYING))
						{
							mess = mess + "JUMP!";
							Ogre::OverlayElement* message = Ogre::OverlayManager::getSingleton().getOverlayElement("MESSAGE2");
							message->setCaption( mess );

							mess = "JUMP " + Ogre::StringConverter::toString(length) +"m "+
								Ogre::StringConverter::toString(time)+"sec "+
								Ogre::StringConverter::toString(Ogre::Real(mNumFlips/2))+"flips";
							app.updateOldMessages( mess );

							// play sound
							sMgr.playSound( "Nice" );
						}

						app.possibleJumpRecord( length, time, Ogre::Real(mNumFlips/2) );
						
					}
					if (mOldState == Car::VS_2WHEELS)
					{
						//down from 2wheels!
						bool show = false;
						Ogre::String mess = "2 WHEELS!";
						Ogre::Real time = mTimer; //.getMilliseconds() / 1000.0;

						if (time > 1.5)
						{
							show = true;
							Ogre::OverlayElement* message = Ogre::OverlayManager::getSingleton().getOverlayElement("MESSAGE2");
							message->setCaption( mess );
						}

						app.possible2WheelRecord( time );

						if ((show) && (Application::getSingleton().getProgState() == StuntPlayground::PS_PLAYING))
						{
							mess = mess + Ogre::StringConverter::toString( time )+"sec";
							app.updateOldMessages( mess );

							// play sound
							sMgr.playSound( "Nice" );
						}
					}

					mOldState = mState;
					mTimer = 0.0; //.reset();
					mCheckingChange = false;
					mNumFlips = 0;
				}
				else
				{
					mCheckingChange = true;
				}
			}
			else
			{
				mCheckingChange = true;
				mChangeTimer = 0.0;
			}
		}
		else
		{
			mCheckingChange = false;
		}

		if (mState == Car::VS_AIRBORNE)
		{
			Ogre::Quaternion orient;
			Ogre::Vector3 pos;

			m_chassis->getPositionOrientation( pos, orient );

			Ogre::Matrix3 rot;
			orient.ToRotationMatrix(rot);
			Ogre::Vector3 Y = rot.GetColumn(1);

			if (((Y.y > 0.0) && (mLastY.y < 0.0)) || ((Y.y < 0.0) && (mLastY.y > 0.0)))
			{
				mNumFlips++;
				mLastY = Y;
			}
		}
		
		
		////////////////////////////////////////////////////////
		// continue with vehicle calculation...
		if (!mManualTransmission)
		{
			if ((mCurrentGear == 1) && (omega <= 1.0) && (omegaIsPositive) && (mBrakesOn))
				mCurrentGear = 0;

			if (mCurrentGear == 0)
			{
				if (mThrottle != 0.0f)
				{
					mCurrentGear = 1;
				}
				else if (mBrakesOn)
				{
					mThrottle = 1.0;
					mBrakesOn = false;
				}

			}
			else
			{
				if (mJustShifted)
				{
					mJustShiftedTimer += elap;

					if  (mJustShiftedTimer < mShiftTime)
					{
						mThrottle = 0.0;
					}
					else
					{
						 mJustShifted = false;
						 mJustShiftedTimer = 0.0;
					}					
				}
			}
				
		}

		//calculate the RPM from this omega.
		Ogre::Real rpm = omega * ((mGears[mCurrentGear]>0)?mGears[mCurrentGear]:-mGears[mCurrentGear]) * mDifferentialRatio * (60.0f/2.0f*3.14159f);
		if (rpm < mTorqueCurve[0].rpm) { rpm = mTorqueCurve[0].rpm; }
		if (rpm > mTorqueCurve[mTorqueCurve.size()-1].rpm) { rpm = mTorqueCurve[mTorqueCurve.size()-1].rpm; }

#ifdef _SP_DEBUG
		Ogre::OverlayElement* text = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug1");
		Ogre::String thetext( "omega: "+Ogre::StringConverter::toString(omega)+" | calculated rpm: "+Ogre::StringConverter::toString(rpm)+
			" | current gear: "+Ogre::StringConverter::toString( mCurrentGear )+" | omegaIsPositive: "+Ogre::StringConverter::toString(omegaIsPositive) );
		text->setCaption( thetext );
		app.mDebugLog->logMessage( thetext );
#endif	// _SP_DEBUG

		
		// find where we are in the torque curve.
		unsigned short high = 0;
		while ( rpm >= mTorqueCurve[high].rpm )
		{
			high++;
			if (high >= mTorqueCurve.size())
			{
				high = mTorqueCurve.size() - 1;
				break;
			}
		}

		// found low.
		unsigned int low = high - 1;
		if (high == 0) { low = 0; high = 1; }

#ifdef _SP_DEBUG
		text = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug2");
		thetext = Ogre::String( "clamped rpm: "+Ogre::StringConverter::toString(rpm)+" | low: "+Ogre::StringConverter::toString(low)+" | high: "+
			Ogre::StringConverter::toString( high )+" mShiftTimer: "+Ogre::StringConverter::toString( mShiftTimer ) );
		text->setCaption( thetext );
		app.mDebugLog->logMessage( thetext );
#endif	// _SP_DEBUG

		///////////////////////////////////////////////////
		// rpm is between [low,high] on the torque curve.
		Ogre::Real interp = ((rpm - mTorqueCurve[low].rpm) / (mTorqueCurve[high].rpm - mTorqueCurve[low].rpm));
		Ogre::Real engine_torque = mTorqueCurve[low].torque + ((mTorqueCurve[high].torque - mTorqueCurve[low].torque)*interp);
		Ogre::Real wheel_torque = 0.0;
		wheel_torque = mThrottle * engine_torque * mGears[mCurrentGear] * mDifferentialRatio * mTransmissionEfficiency;


		///////////////////////////////////////////////////
		int resetShiftTimer = 0;
		if (mCurrentGear > 1)
		{
			if ((rpm < mTorqueCurve[1].rpm * 0.5f) && (!mManualTransmission))
			{ 
				mShiftTimer += elap;
				if (mShiftTimer > mShiftDelay) 
				{ 
					shiftDown(); 
					mShiftTimer = 0.0; 
				}
			}
			else
			{ resetShiftTimer++; }
		}
		else
		{
			if (rpm < 2000) { rpm = 2000; }
			resetShiftTimer++;
		}

		if ((rpm > mTorqueCurve[ mTorqueCurve.size()-2 ].rpm) && (!mManualTransmission)) 
		{ 
			mShiftTimer += elap;
			if (mShiftTimer > mShiftDelay)
			{
				if (mCurrentGear > 0) 
				{ 
					shiftUp(); 
					mShiftTimer = 0.0f; 
				}
			}
		}
		else
		{ resetShiftTimer++; }

		if (resetShiftTimer == 2) { mShiftTimer = 0.0; }
		

#ifdef _SP_DEBUG
		text = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug3");
		thetext = Ogre::String( "interp: "+Ogre::StringConverter::toString( interp )+" | engine_torque: "+Ogre::StringConverter::toString( engine_torque )+
			" | wheel_torque: "+Ogre::StringConverter::toString( wheel_torque ) );
		text->setCaption( thetext );
		app.mDebugLog->logMessage( thetext );

		Ogre::Real carspeed = (m_chassis->getVelocity().length()*3.6);
		text = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug4");
		thetext = Ogre::String( "car speed: "+Ogre::StringConverter::toString(carspeed)+" km/h  || "+
			Ogre::StringConverter::toString(Ogre::Real(carspeed * 0.621371))+"mph");
		text->setCaption( thetext );

#endif	// _SP_DEBUG

		///////////////////////////////////////////////////
		// UPDATE THE ONSCREEN UI
		Ogre::OverlayElement* ui = Ogre::OverlayManager::getSingleton().getOverlayElement("GEAR");
		ui->setCaption( Ogre::StringConverter::toString( mCurrentGear ) );

		ui = Ogre::OverlayManager::getSingleton().getOverlayElement("SPEED");
		Ogre::Real kmh = (m_chassis->getVelocity().length()*3.6);
		ui->setCaption( Ogre::StringConverter::toString( (int)(kmh) ) );



		// steering!
		Ogre::Real deltat = m_chassis->getWorld()->getTimeStep();

		mSteerAngle += mSteerSpeed * mSteering * deltat;
		if (!mSteering)
		{
			if (mSteerAngle > Ogre::Radian(0.0))
			{
				mSteerAngle -= mSteerSpeed * deltat;
				if (mSteerAngle < Ogre::Radian(0.0)) { mSteerAngle = Ogre::Radian(0.0); }
			}
			else
			{
				mSteerAngle += mSteerSpeed * deltat;
				if (mSteerAngle > Ogre::Radian(0.0)) { mSteerAngle = Ogre::Radian(0.0); }
			}
		}

		// calculate the new max steering angle, based on velocity data.
		Ogre::Real maxsteer = mSteerMaxAngle.valueRadians();

		if (kmh < mSteerFromSpeed)
		{
			// going slower than the minimum speed, so no change.
		}
		else
		{
			if (kmh > mSteerToSpeed)
			{
				// going faster than highest speed, so remove completely!
				maxsteer -= (maxsteer* mSteerLossPercent);
			}
			else
			{
				// inside the range [mSteerFromSpeed, mSteerToSpeed].
				Ogre::Real delta = ((kmh - mSteerFromSpeed) / (mSteerSpeedRange));
				maxsteer -= (maxsteer*(mSteerLossPercent * delta));
			}
		}

		Ogre::Radian maxs( maxsteer );

		if (mSteerAngle > maxs) { mSteerAngle = maxs; }
		if (mSteerAngle < -maxs) { mSteerAngle = -maxs; }

		
		////////////////////////////////////////////////////
		// engine sound!
		if (mEngineSnd)
		{
			// set frequency based on RPM
			mEngineSnd->setFrequency( (int)( 44100 + ((mEngineBaseRPM*mEngineScale) - ((mMaxRPM - rpm)*mEngineScale))  ) );
		}

		bool playScreech = false;

		// re-loop through the tires, updating them!
		for (Car::MyTire* tire = (Car::MyTire*)getFirstTire(); tire; tire = (Car::MyTire*)getNextTire( tire ) )
		{
			// tire slip settings.
			Ogre::Real speed = tire->getOmega() * tire->getRadius();
			Ogre::Real load = tire->getNormalLoad();
			

			tire->setMaxSideSlipSpeed( speed * 0.1f );
			tire->setSideSlipCoefficient( speed * load * load * (tire->getGrip()) );

			speed = tire->getLongitudinalSpeed();

			tire->setMaxLongitudinalSlipSpeed( speed * tire->getGrip() );
			tire->setLongitudinalSlipCoefficient( speed * load * (tire->getGrip()) );

			// is this a driving tire?
			if (tire->getDrive())
			{
				tire->setTorque( wheel_torque * tire->getDrive() );
			}
				
			if (mBrakesOn)
			{
				Ogre::Real accel = tire->calculateMaxBrakeAcceleration();

				if (tire->getDrive())
				{
					tire->setBrakeAcceleration( accel, mBrakeStrength );
				}
				else
				{
					tire->setBrakeAcceleration( accel, mBrakeStrength * 0.5f );
				}
				
			}
		
			if (mThrottle == 0.0)
			{
				Ogre::Real accel = ((rpm - mMinRPM)/(mRPMRange)) * (mEngineMass * 2);
				tire->setTorque( (omegaIsPositive)? -accel : accel );
			}


			if (tire->getSteer())
				tire->setSteeringAngle( mSteerAngle * tire->getSteer() );

			// tire sound effects!
			if (!playScreech)
			{
				if ((tire->lostSideGrip() == 1) || (tire->lostTraction() == 1))
				{
					if (mTireState == Car::TS_ONROAD)
					{
						playScreech = true;
					}
				}
			}
		}

		// play sound effects.
		if (playScreech)
		{
			if (!mScreechSnd->isPlaying())
				mScreechSnd->play( (int)(m_chassis->getVelocity().length() * 2.0));
		}
		else
		{
			if (mScreechSnd->isPlaying())
				mScreechSnd->stop();
		}

		//reset collision counter.
		mCollisions = 0;

		// reset tire state.
		mTireState = TS_ONDIRT;


		mThrottle = originalThrottle;
		mSteering = originalSteering;
		mBrakesOn = originalBrakes;

	}


	void Car::setupRecording( Ogre::Real minwait )
	{
		if (!mRecObject)
		{
			mRecObject = WorldRecorder::getSingleton().addRecordableObject( m_chassis->getOgreNode(), minwait );
		}

		// loop through tires
		for (Car::MyTire* tire = (Car::MyTire*)getFirstTire(); tire; tire = (Car::MyTire*)getNextTire( tire ) )
		{
			if (!tire->mRecObj)
			{
				tire->mRecObj = WorldRecorder::getSingleton().addRecordableObject( tire->getOgreNode(), minwait );
			}
		}
	}


	void Car::Transform( OgreNewt::Body* me, const Ogre::Quaternion& orient, const Ogre::Vector3& pos )
	{
		Car* car = (Car*)me->getUserData();

		me->getOgreNode()->setOrientation( orient );
		me->getOgreNode()->setPosition( pos );

		// update the instant replay recording.
		if (car->mRecObject)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -V- transform() - adding car keyframe", Ogre::LML_TRIVIAL);
#endif	// _SP_DEBUG_LOG
			car->mRecObject->addKeyframe();
		}


		// loop through tires
		for (Car::MyTire* tire = (Car::MyTire*)car->getFirstTire(); tire; tire = (Car::MyTire*)car->getNextTire( tire ) )
		{
			// inside the tire loop.
			tire->updateNode();
			if (tire->mRecObj)
			{
#ifdef _SP_DEBUG_LOG
				Application::getSingleton().mDebugLog->logMessage(" -V- transform() - adding tire keyframe", Ogre::LML_TRIVIAL);
#endif	// _SP_DEBUG_LOG
				tire->mRecObj->addKeyframe();
			}

		}
	}

	void Car::incCollisions()
	{
		mCollisions++;
	}


	void Car::playEngine()
	{
		if (mEngineSnd)
		{
			mEngineSnd->setLoop( true );
			mEngineSnd->play( 128 );
		}
	}

	void Car::stopEngine()
	{
		if (mEngineSnd)
		{
			if (mEngineSnd->isPlaying()) { mEngineSnd->stop(); }
		}
	}

	



}	// end NAMESPACE StuntPlayground
