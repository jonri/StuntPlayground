/*
	Stunt Playground
	
	ReflectionRenderTarget
*/

#ifndef _STUNTPLAYGROUND_REFLECTION_RENDER_TARGET_
#define _STUNTPLAYGROUND_REFLECTION_RENDER_TARGET_

#include <Ogre.h>

namespace StuntPlayground
{

	class ReflectionRenderTarget : public Ogre::RenderTargetListener
	{
	public:
		ReflectionRenderTarget( Ogre::SceneManager* mgr, Ogre::Overlay* ov ) : Ogre::RenderTargetListener()
		{
			mSceneMgr = mgr;
			mOverlay = ov;
		}

		void preRenderTargetUpdate( const Ogre::RenderTargetEvent &evt);
		void postRenderTargetUpdate( const Ogre::RenderTargetEvent &evt );

		void addHideNode( Ogre::SceneNode* node ) { mHideNodes.push_back(node); }

		void addHideParticle( Ogre::ParticleSystem* particle );

		void clearHideNodes()
		{
			mHideNodes.clear();
			mHideParticles.clear();
		}

	private:
		struct ParticleHide
		{
			Ogre::ParticleSystem* m_particle;
			bool wasVisible;
		};

		Ogre::SceneManager* mSceneMgr;
		Ogre::Overlay* mOverlay;
		bool overlayWasShown;

		Ogre::ShadowTechnique mOldShadowTech;

		std::vector<Ogre::SceneNode*> mHideNodes;
		std::vector<ParticleHide> mHideParticles;

	};


}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_REFLECTION_RENDER_TARGET_