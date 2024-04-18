#if 0
// 𝐍𝐎𝐓𝐄: You will not be able to compile this as it relies on a simple 3d rendering engine which is
// not included.  It is provided as an example of something interesting - programming
// interactively in a notebook interface while allowing full real-time virtual reality rendering.
//
// The way it works is it exports a few simple functions that call into a 3d rendering library for
// interactive visualization that uses vulkan or directx12 to give you a real-time 3d window with
// WASD controls and vr support.
//
// The interface provided is cheesy - all you can do is add spheres to the scene.  From Julia you
// initialize it by calling StartRendering() which will spawn a thread to run the 3d ui.  This
// means that rendering and camera movement will not be blocked by computation or compilation on
// the main thread.


#include <codebase/all.h>
#include <exposer/exposer.h>
#include "codebase/filesystem.h"
#include "codebase/application.h"

#include <codebase/fixedvector.h>
#include <Eigen/Core>
#include <random>
#include <math.h>
#include <codebase/stringtokens.h>
#include "assimp_helpers/assimp_helpers.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include "tbb/tbb.h"
#include "tbb/flow_graph.h"
#include "numeric/numeric_optimization.h"
#include "3dclient/3dclient.h"


struct Sphere_t
{
	v3f m_vPos;
	v3f m_vColor;
	float m_flRadius;
};
	
class CTestApp : public I3DBaseClient
{
	typedef I3DBaseClient BaseClass;
public:
	using I3DBaseClient::I3DBaseClient;

	CLockableVector<Sphere_t> m_vSpheres;
	BlockingConcurrentQueue<int> m_msgQueue;
	bool m_bStarted = false;
	
	void RunMainLoop() override;
	
	void SetupFrame( CRenderFrame *pFrame ) override;

	void SetupUI()
	{
		BaseClass::SetupUI();
	}

	void AddSphere( float flX, float flY, float flZ, float flRad,  float flRed, float flGreen, float flBlue )
	{
		with_lock( m_vSpheres )
		{
			Sphere_t newOne = { v3f( flX, flY, flZ ), v3f( flRed, flGreen, flBlue ), flRad };
			m_vSpheres.push_back( newOne );
		}
	}
	void MoveSphere( int nSphereIndex, float flX, float flY, float flZ )
	{
		with_lock( m_vSpheres )
		{
			if ( nSphereIndex < m_vSpheres.Count() )
			{
				m_vSpheres[nSphereIndex].m_vPos.x = flX;
				m_vSpheres[nSphereIndex].m_vPos.y = flY;
				m_vSpheres[nSphereIndex].m_vPos.z = flZ;
			}
		}
	}
};

void CTestApp::SetupFrame( CRenderFrame *pFrame )
{
	BaseClass::SetupFrame( pFrame );
	with_lock( m_vSpheres )
	{
		for( Sphere_t const &sph : m_vSpheres )
		{
			pFrame->Draw3DMesh( &Renderer()->m_sphereMesh, sph.m_vPos, { 0, 0, sph.m_flRadius },  { 0, sph.m_flRadius, 0 }, sph.m_vColor );
		}
	}
}

void CTestApp::RunMainLoop()
{
	BaseClass::RunMainLoop();
}

CTestApp *g_pApp;

void ClearSpheres()
{
	with_lock( g_pApp->m_vSpheres )
	{
		g_pApp->m_vSpheres.clear();
	}
}

void MoveSphere( int nSphereIndex, float flX, float flY, float flZ )
{
	g_pApp->MoveSphere( nSphereIndex, flX, flY, flZ );
}

void AddSphere( float flX, float flY, float flZ, float flRad , float flRed, float flGreen, float flBlue )
{
	g_pApp->AddSphere( flX, flY, flZ, flRad, flRed, flGreen, flBlue );
}

void StopRendering()
{
	g_pApp->RequestQuit();
}
void StartRendering()
{
	if ( ! g_pApp->m_bStarted )
	{
		g_pApp->m_msgQueue.enqueue( 1 );
	}
}

void StartDLL( int argc, char **argv )
{
	// Call this before setting the application object so that we can look at the command line args
	ApplicationInit( argc, argv );
	ExposeGeo();
	EXPOSEFN( StopRendering );
	EXPOSEFN( AddSphere );
	EXPOSEFN( MoveSphere );
	EXPOSEFN( ClearSpheres );
	EXPOSEFN( StartRendering );
	CTestApp *pApp = new CTestApp( 0, nullptr );
	g_pApp = pApp;
	new std::thread( []{
		// Wait for start message. We need to create the window on this thread so that windows
		// messages will get here
		int nMessage;
		g_pApp->m_msgQueue.wait_dequeue( nMessage );
		g_pApp->Initialize();
		g_pApp->RunMainLoop();
	} );
}

#endif
