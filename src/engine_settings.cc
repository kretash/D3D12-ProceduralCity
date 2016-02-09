#include "core/engine_settings.hh"
#include <cassert>
#include <iostream>
#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

EngineSettings* EngineSettings::m_instance = nullptr;

EngineSettings::EngineSettings() :
  frame_counter( 0 ),
  m_delta_time( 0.0f ),
  m_time_elapsed( 0.0f ),
  m_fps_smooth( 0.0f ),
  m_update_smooth( 0.0f ),
  m_render_smooth( 0.0f ) {
  for( int32_t i = 0; i < MAX_TIMERS; ++i ) {
    ids_[i] = "NULL";
  }

  std::ifstream json;
  std::string json_line;
  std::string json_buffer;
  json.open( PATH"engine_settings.json", std::ios::out );

  if( json.is_open() ) {
    while( std::getline( json, json_line ) ) {
      json_buffer += json_line;
    }
    json.close();
  }

  rapidjson::Document doc;
  doc.Parse( json_buffer.c_str() );
  m_engine_settings.resolution_width = doc["resolution_width"].GetInt();
  m_engine_settings.resolution_height = doc["resolution_height"].GetInt();
  m_engine_settings.fullscreen = doc["fullscreen"].GetBool();
  m_engine_settings.grid = doc["grid"].GetInt();
  m_engine_settings.animated_camera = doc["animated_camera"].GetBool();
  m_engine_settings.play_sound = doc["play_sound"].GetBool();
  m_engine_settings.sound_file = doc["sound_file"].GetString();
  m_engine_settings.msaa_enabled = doc["MSAA_enabled"].GetBool();
  m_engine_settings.msaa_count = doc["MSAA_count"].GetInt();
  m_engine_settings.upscale_render = doc["upscale_render"].GetDouble();

}

void EngineSettings::start_timer( std::string timer_id ) {
  for( int32_t i = 0; i <= MAX_TIMERS; ++i ) {
    assert( i != MAX_TIMERS && "RAN OUT OF TIMERS " );

    if( ids_[i] == timer_id ) {
      QueryPerformanceFrequency( &frequency[i] );
      QueryPerformanceCounter( &starting_time[i] );
      break;
    }
    if( ids_[i] == "NULL" ) {
      ids_[i] = timer_id;
      QueryPerformanceFrequency( &frequency[i] );
      QueryPerformanceCounter( &starting_time[i] );
      break;
    }
  }
}

double EngineSettings::get_time( std::string timer_id ) {

  for( int32_t i = 0; i <= MAX_TIMERS; ++i ) {
    assert( i != MAX_TIMERS && "RAN OUT OF TIMERS " );

    if( ids_[i] == timer_id ) {
      QueryPerformanceCounter( &ending_time[i] );
      elapsed_microseconds[i].QuadPart = ending_time[i].QuadPart - starting_time[i].QuadPart;
      double interval = static_cast< double >( elapsed_microseconds[i].QuadPart ) / frequency[i].QuadPart;
      return interval;
    }
  }

  assert( false && "TIMER NOT FOUND" );
  return -1.0;
}

void EngineSettings::start_frame() {
  start_timer( "frame" );
}

void EngineSettings::start_render() {
  start_timer( "render" );
}

void EngineSettings::end_frame() {
  ++frame_counter;
  m_delta_time = ( float ) get_time( "frame" );
  float render_time = ( float ) get_time( "render" );
  float update_time = m_delta_time - render_time;

  m_time_elapsed += m_delta_time;
  m_fps_smooth += m_delta_time;
  m_update_smooth += update_time;
  m_render_smooth += render_time;

  if( frame_counter > FPS_SMOOTH ) {
    m_fps_smooth /= ( float ) frame_counter;
    m_update_smooth /= ( float ) frame_counter;
    m_render_smooth /= ( float ) frame_counter;

    std::string time = "Frame Time-> " + std::to_string( m_fps_smooth )
      + " FPS -> " + std::to_string( 1.0 / m_fps_smooth )
      + " Update Time-> " + std::to_string( m_update_smooth )
      + " Render Time-> " + std::to_string( m_render_smooth );

    log( time );

    m_fps_smooth = 0.0f;
    m_update_smooth = 0.0f;
    m_render_smooth = 0.0f;
    frame_counter = 0;
  }
}

engine_settings EngineSettings::get_settings() {
  return m_engine_settings;
}

void EngineSettings::log( std::string l ) {
  std::cout << l << std::endl;
}

EngineSettings::~EngineSettings() {

}