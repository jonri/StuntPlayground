#include "ReflectionRenderTarget.h"

namespace StuntPlayground
{


void ReflectionRenderTarget::addHideParticle( Ogre::ParticleSystem* particle )
{
	ParticleHide temp;

	temp.m_particle = particle;
	temp.wasVisible = true;

	mHideParticles.push_back( temp );
}


void ReflectionRenderTarget::preRenderTargetUpdate( const Ogre::RenderTargetEvent& evt )
{
	// no shadows in reflections!
	mOldShadowTech = mSceneMgr->getShadowTechnique();

	mSceneMgr->setShadowTechnique( Ogre::SHADOWTYPE_NONE );

	//also hide the overlay.
	if (mOverlay->isVisible())
	{
		mOverlay->hide();
		overlayWasShown = true;
	}
	else
	{
		overlayWasShown = false;
	}

	// hide the nodes we don't want to render
	std::vector<Ogre::SceneNode*>::iterator it;

	for (it=mHideNodes.begin(); it!=mHideNodes.end(); it++)
		(*it)->setVisible(false);

	// hide the particle systems we don't want to render
	std::vector<ParticleHide>::iterator itp;

	for (itp=mHideParticles.begin(); itp != mHideParticles.end(); itp++)
	{
		if (itp->m_particle->isVisible())
		{
			itp->m_particle->setVisible(false);
			itp->wasVisible = true;
		}
		else
		{
			itp->wasVisible = false;
		}
	}



#ifdef _SP_DEBUG
	Ogre::Overlay* debugOverlay = Ogre::OverlayManager::getSingleton().getByName("Walaber/StandardDebug");
	debugOverlay->hide();
#endif	// _SP_DEBUG
}

void ReflectionRenderTarget::postRenderTargetUpdate( const Ogre::RenderTargetEvent& evt )
{
	// restore shadows
	mSceneMgr->setShadowTechnique( mOldShadowTech );

	// show the overlay.
	if (overlayWasShown)
		mOverlay->show();

	// show the nodes we hid before
	std::vector<Ogre::SceneNode*>::iterator it;

	for (it=mHideNodes.begin(); it!=mHideNodes.end(); it++)
		(*it)->setVisible(true);

	// show the particles we hid before
	// hide the particle systems we don't want to render
	std::vector<ParticleHide>::iterator itp;

	for (itp=mHideParticles.begin(); itp != mHideParticles.end(); itp++)
	{
		if (itp->wasVisible)
			itp->m_particle->setVisible(true);
	}

#ifdef _SP_DEBUG
	Ogre::Overlay* debugOverlay = Ogre::OverlayManager::getSingleton().getByName("Walaber/StandardDebug");
	debugOverlay->show();
#endif	// _SP_DEBUG
}


}	// end NAMESPACE StuntPlayground