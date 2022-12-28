cmake_minimum_required( VERSION 2.8 )
set( CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include( PET_cmake_tools )
include( PET_cmake_use_pkg )

set_project_start( "SubSurfaceScatter" DEBUG SHARED )

add_definitions( /D _CONSOLE )
add_definitions( /wd4251 )

#optimizeFlags( "sse" "sse2" )

message( STATUS
	"Looking for libraries:" )
use_pkg_boost( REQUIRED VERSION 1.40 )
use_pkg_directx( REQUIRED )
use_pkg_anttweakbar( REQUIRED )
use_pkg_opencv( OPTIONAL VERSION 2.1 )

include_directories( include )

message(STATUS
	"Actual targets:" )
add_all_subdirectories()
