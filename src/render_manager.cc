#include "core/render_manager.hh"
#include "core/engine_settings.hh"
#include "core/engine.hh"
#include "core/camera.hh"
#include "core/input.hh"
#include <algorithm>

RenderManager::RenderManager(){}

void RenderManager::add_child( Drawable* d ) {
  m_render_bin.push_back( d );
}

void RenderManager::update( float df ) {
  
  Camera* c = k_engine->get_camera();
  float3 c_pos = c->get_position();
  _generate_frustum_planes();

  float3 camera = k_engine->get_camera()->get_position();
  m_active_render_bin.clear();

  for( int32_t i = 0; i < m_render_bin.size(); ++i ) {

    float3 pos = m_render_bin[i]->get_position();

    if( 0.0f < m_render_bin[i]->get_radius() ) {

      float maxh = m_render_bin[i]->get_max_height();

      float3 v_lenght = c_pos - pos;
      float length = float3::lenght( v_lenght );
      m_render_bin[i]->set_distance( length );

      if( _inside_frustum( pos, m_render_bin[i]->get_radius(), maxh ) ) {

        m_active_render_bin.push_back( m_render_bin[i] );

        if( length < 500 )
          m_render_bin[i]->set_lod( 0 );
        else if( length < 1000 )
          m_render_bin[i]->set_lod( 1 );
        else
          m_render_bin[i]->set_lod( 2 );

        m_render_bin[i]->set_active( true );

      } else {
        //Outside frustum
        m_render_bin[i]->set_active( false );
      }
    } else {

      //Ignore frustum is enabled
      m_active_render_bin.push_back( m_render_bin[i] );
      float3 v_lenght = c_pos - pos;
      float length = float3::lenght( v_lenght );

      if( length < 500 )
        m_render_bin[i]->set_lod( 0 );
      else if( length < 1000 )
        m_render_bin[i]->set_lod( 1 );
      else
        m_render_bin[i]->set_lod( 2 );

      m_render_bin[i]->set_distance( length );
      m_render_bin[i]->set_active( true );
    }
  }

}

bool RenderManager::_inside_frustum( float3 point, float r, float maxh ) {

  // 0 - Left clipping plane
  // 1 - Right clipping plane
  // 2 - Top clipping plane
  // 3 - Bottom clipping plane
  // 4 - Near clipping plane
  // 5 - Far clipping plane

  float d = 0.0f;

  d = float3::dot( point, m_frustum_planes[0].xyz() );
  d += r;
  d += m_frustum_planes[0].d;
  if( d <= 0 ) return false;

  d = float3::dot( point, m_frustum_planes[1].xyz() );
  d += r;
  d += m_frustum_planes[1].d;
  if( d <= 0 ) return false;

  d = float3::dot( point, m_frustum_planes[2].xyz() );
  d += r;
  d += m_frustum_planes[2].d;
  if( d <= 0 ) return false;

  d = float3::dot( point, m_frustum_planes[3].xyz() );
  d += maxh;
  d += m_frustum_planes[3].d;
  if( d <= 0 ) return false;

  d = float3::dot( point, m_frustum_planes[4].xyz() );
  d += r;
  d += m_frustum_planes[4].d;
  if( d <= 0 ) return false;

  d = float3::dot( point, m_frustum_planes[5].xyz() );
  d += r;
  d += m_frustum_planes[5].d;
  if( d <= 0 ) return false;

  return true;
}

//Fast Extraction of Viewing Frustum Planes from the World-View - Projection Matrix
void RenderManager::_generate_frustum_planes() {
  Camera* cc = k_engine->get_camera();
  float4x4 c = cc->get_view() * cc->get_projection();
  //c.transpose()

#ifdef __DIRECTX12__

  m_frustum_planes[0] = plane( c.m[0][3] + c.m[0][0],
    c.m[1][3] + c.m[1][0],
    c.m[2][3] + c.m[2][0],
    c.m[3][3] + c.m[3][0] );

  m_frustum_planes[1] = plane( c.m[0][3] - c.m[0][0],
    c.m[1][3] - c.m[1][0],
    c.m[2][3] - c.m[2][0],
    c.m[3][3] - c.m[3][0] );

  m_frustum_planes[2] = plane( c.m[0][3] - c.m[0][1],
    c.m[1][3] - c.m[1][1],
    c.m[2][3] - c.m[2][1],
    c.m[3][3] - c.m[3][1] );

  m_frustum_planes[3] = plane( c.m[0][3] + c.m[0][1],
    c.m[1][3] + c.m[1][1],
    c.m[2][3] + c.m[2][1],
    c.m[3][3] + c.m[3][1] );

  m_frustum_planes[4] = plane( c.m[0][2], c.m[1][2], c.m[2][2], c.m[3][2] );

  m_frustum_planes[5] = plane( c.m[0][3] - c.m[0][2],
    c.m[1][3] - c.m[1][2],
    c.m[2][3] - c.m[2][2],
    c.m[3][3] - c.m[3][2] );

#elif __VULKAN__

  m_frustum_planes[0] = plane( c.m[3][0] + c.m[0][0], c.m[3][1] + c.m[0][1],
    c.m[3][2] + c.m[0][2], c.m[3][3] + c.m[0][3] );

  m_frustum_planes[1] = plane( c.m[3][0] - c.m[0][0], c.m[3][1] - c.m[0][1],
    c.m[3][2] - c.m[0][2], c.m[3][3] - c.m[0][3] );

  m_frustum_planes[2] = plane( c.m[3][0] - c.m[1][0], c.m[3][1] - c.m[1][1],
    c.m[3][2] - c.m[1][2], c.m[3][3] - c.m[1][3] );

  m_frustum_planes[3] = plane( c.m[3][0] + c.m[1][0], c.m[3][1] + c.m[1][1],
    c.m[3][2] + c.m[1][2], c.m[3][3] + c.m[1][3] );

  m_frustum_planes[4] = plane( c.m[3][0] + c.m[2][0], c.m[3][1] + c.m[2][1],
    c.m[3][2] + c.m[2][2], c.m[3][3] + c.m[2][3] );

  m_frustum_planes[5] = plane( c.m[3][0] - c.m[2][0], c.m[3][1] - c.m[2][1],
    c.m[3][2] - c.m[2][2], c.m[3][3] - c.m[2][3] );
#else
#error no API defined at frustum planes computation
#endif

  m_frustum_planes[0].normalize();
  m_frustum_planes[1].normalize();
  m_frustum_planes[2].normalize();
  m_frustum_planes[3].normalize();
  m_frustum_planes[4].normalize();
  m_frustum_planes[5].normalize();

}

RenderManager::~RenderManager() {

}