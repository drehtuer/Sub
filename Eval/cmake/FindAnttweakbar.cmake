set( ATB_SEARCH_DIR
	"extern/anttweakbar" )
find_path( AntTweakBar_INCLUDE_DIR
	AntTweakBar.h
	PATH extern/AntTweakBar )
#message( STATUS "AntTweakBar header: " ${AntTweakBar_INCLUDE_DIR} )

set( ATB_LIBS 
	AntTweakBar64 )
set( AntTweakBar_LIBRARIES )
find_library( AntTweakBar_LIBRARIES
	${ATB_LIBS}
	PATH extern/AntTweakBar	)
#message( STATUS "AntTweakBar libs: " ${AntTweakBar_LIBRARIES} )

include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( AntTweakBar
	DEFAULT_MSG
	AntTweakBar_INCLUDE_DIR
	AntTweakBar_LIBRARIES )
