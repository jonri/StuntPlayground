#include "RigidBodyLoader.h"
#include <tinyxml.h>

namespace StuntPlayground
{


OgreNewt::Body* loadRigidBody( Ogre::String& filename, Ogre::String& entityname, OgreNewt::World* world, Ogre::SceneNode* parentnode,
							  Ogre::SceneManager* mgr, int colID )
{
	OgreNewt::Body* rigidbody = NULL;

	Ogre::Real mass = 0.0f;
	Ogre::Vector3 inertia = Ogre::Vector3::ZERO;
	Ogre::SceneNode* scenenode;

	// first, parse in the XML file.
	TiXmlDocument doc;

	doc.LoadFile( (std::string)filename );

	// get the rigid_body element.
	TiXmlNode* node = doc.FirstChild( "rigid_body" );
	TiXmlElement* body = node->ToElement();

	if (!body)
	{
		return NULL;
	}

	//get the name.
	Ogre::String rigidname = (Ogre::String)body->Attribute( "name" );
	Ogre::LogManager::getSingleton().logMessage("    rigid body name: "+rigidname );

	// get the visual mesh.
	node = body->FirstChild( "mesh" );
	if (!node)
	{
		Ogre::LogManager::getSingleton().logMessage("   cannot find mesh element...");
	}
	else
	{
		TiXmlElement* mesh = node->ToElement();
		Ogre::String meshfile = (Ogre::String)mesh->Attribute( "filename" );

		// LOAD MESH HERE!
		scenenode = parentnode->createChildSceneNode();
		Ogre::Entity* ent = mgr->createEntity( entityname, meshfile );
		scenenode->attachObject( ent );
	}

	// now get the properties...
	node = body->FirstChild( "properties" );

	if (!node)
	{
		Ogre::LogManager::getSingleton().logMessage("   cannot find rigid body properties..");
	}
	else
	{
		TiXmlElement* prop = node->ToElement();

		mass = Ogre::StringConverter::parseReal( (Ogre::String)prop->Attribute("mass") );
		inertia = Ogre::StringConverter::parseVector3( (Ogre::String)prop->Attribute("inertia") );
	}

	// get the collision pieces.
	OgreNewt::Collision* tempcol;
	std::vector<OgreNewt::Collision*> collision_vector;

	// now loop through the primitives!
	//get the first primitive;
	node = body->FirstChild( "primitive" );
	if (!node)
	{
		Ogre::LogManager::getSingleton().logMessage("    cannot find any primitives in the body...");
		return NULL;
	}
	TiXmlElement* prim = node->ToElement();
	bool done = false;

	while (!done)
	{
		//okay, get details from the current primitive.
		//name
		Ogre::String name = (Ogre::String)prim->Attribute( "name" );
			
		//shape
		StuntPlayground::PrimitiveType shape;
		Ogre::String shapetxt = (Ogre::String)prim->Attribute( "shape" );

		//default is box.
		shape = StuntPlayground::BOX;
		if (shapetxt == "ellipsoid") { shape = StuntPlayground::ELLIPSOID; }
		if (shapetxt == "cylinder") { shape = StuntPlayground::CYLINDER; }
		if (shapetxt == "capsule") { shape = StuntPlayground::CAPSULE; }
		if (shapetxt == "cone") { shape = StuntPlayground::CONE; }
		if (shapetxt == "chamfer_cylinder") { shape = StuntPlayground::CHAMFER_CYLINDER; }
		if (shapetxt == "convex_hull") { shape = StuntPlayground::CONVEX_HULL; }
		if (shapetxt == "tree") { shape = StuntPlayground::TREE; }
			
		// default values.
		Ogre::Vector3 size(1,1,1);
		Ogre::Quaternion orient = Ogre::Quaternion::IDENTITY;
		Ogre::Vector3 pos(0,0,0);

		// get properties
		TiXmlElement* prop = prim->FirstChildElement( "properties" );
		if (!prop)
		{
			Ogre::LogManager::getSingleton().logMessage("      cannot find properties for this primitive!");
		}
		else
		{
			//SIZE
			if ((shape==StuntPlayground::BOX) || (shape==StuntPlayground::ELLIPSOID))
			{
				//should have property "size"
				size = Ogre::StringConverter::parseVector3( (Ogre::String)prop->Attribute("size") );
			}
			if ((shape==StuntPlayground::CYLINDER) || (shape==StuntPlayground::CAPSULE) || (shape==StuntPlayground::CONE) || (shape==StuntPlayground::CHAMFER_CYLINDER))
			{
				// has 2 properties, "height" and "radius"
				Ogre::Real height, radius;
				height = Ogre::StringConverter::parseReal( (Ogre::String)prop->Attribute("height") );
				radius = Ogre::StringConverter::parseReal( (Ogre::String)prop->Attribute("radius") );

				size = Ogre::Vector3( height, radius, 0);
			}

			//ORIENTATION
			Ogre::Vector3 euler;
			Ogre::Degree x,y,z;
			Ogre::Matrix3 mat;

			euler = Ogre::StringConverter::parseVector3( (Ogre::String)prop->Attribute("rotation") );
			x = euler.x; y = euler.y; z = euler.z;
			mat.FromEulerAnglesXYZ( x, y, z );
			orient.FromRotationMatrix( mat );


			//POSITION
			pos = Ogre::StringConverter::parseVector3( (Ogre::String)prop->Attribute("position") );

		}

		// put in special case for convex hulls here!
		if (shape==StuntPlayground::CONVEX_HULL)
		{
			// a vector of vertices.
			std::vector<Ogre::Vector3> vert_vector;

			// special case for convex hulls.
			TiXmlElement* vertice = prop->FirstChildElement("vertex");
			if (!vertice)
			{
				Ogre::LogManager::getSingleton().logMessage("        cannot find vertices for convex hull!");
			}
			else
			{
				while (vertice)
				{
					//get first vertex.
					Ogre::Vector3 v = Ogre::StringConverter::parseVector3( (Ogre::String)vertice->Attribute("position") );
					vert_vector.push_back( v );

					//get next vertex.
					vertice = vertice->NextSiblingElement( "vertex" );
				}

				//done parsing the verts.  make the convex hull!
				int count = vert_vector.size();
				Ogre::Vector3* verts = new Ogre::Vector3[count];

				//fill the array.
				for (int i=0;i<count;i++)
				{
					verts[i] = vert_vector[i];
				}

				tempcol = new OgreNewt::CollisionPrimitives::ConvexHull( world, verts, count );
				collision_vector.push_back(tempcol);

				delete []verts;
			}
				
		}
		else
		{
			// create the collision item.
			switch( shape )
			{
			case StuntPlayground::BOX:
				tempcol = new OgreNewt::CollisionPrimitives::Box( world, size, orient, pos );
				break;

			case StuntPlayground::ELLIPSOID:
				tempcol = new OgreNewt::CollisionPrimitives::Ellipsoid( world, size, orient, pos );
				break;

			case StuntPlayground::CYLINDER:
				tempcol = new OgreNewt::CollisionPrimitives::Cylinder( world, size.y, size.x, orient, pos );
				break;

			case StuntPlayground::CAPSULE:
				tempcol = new OgreNewt::CollisionPrimitives::Capsule( world, size.y, size.x, orient, pos );
				break;

			case StuntPlayground::CONE:
				tempcol = new OgreNewt::CollisionPrimitives::Cone( world, size.y, size.x, orient, pos );
				break;

			case StuntPlayground::CHAMFER_CYLINDER:
				tempcol = new OgreNewt::CollisionPrimitives::ChamferCylinder( world, size.y, size.x, orient, pos );
				break;

			case StuntPlayground::TREE:
				tempcol = new OgreNewt::CollisionPrimitives::TreeCollision( world, scenenode, false );
				break;

			default:
				tempcol = new OgreNewt::CollisionPrimitives::Box( world, size, orient, pos );
				break;
			}

			tempcol->setUserID( colID );

			collision_vector.push_back(tempcol);

		}

		Ogre::LogManager::getSingleton().logMessage("       primitive! name:"+name+"  shape:"+shapetxt );

		/////////////////////////////////////////////////////////////
		//okay, onto next primitive.
		node = prim->NextSibling( "primitive" );
		if (!node)
		{
			//no more primitives, we're done here.
			done = true;
		}
		else
		{
			//prepare for the next one!
			prim = node->ToElement();
		}

	}

	// all collision elements have been found and created, let's create the body!!
	if (collision_vector.size() > 1)
		tempcol = new OgreNewt::CollisionPrimitives::CompoundCollision( world, collision_vector );
	
	rigidbody = new OgreNewt::Body( world, tempcol );

	rigidbody->attachToNode( scenenode );
	rigidbody->setMassMatrix(mass, inertia);

	return rigidbody;

}



}	// end NAMESPACE StuntPlayground

	