# Try to find headers of modPET
#
# Defines: MODPET_FOUND, modPET_INCLUDE_DIR

set( modPET_INCLUDE_DIR )
set( _MODPET_POSSIBLE_ROOT_DIRS
	../../../prototypes/modPET
	../../prototypes/modPET )

set( _MODPET_INCLUDE_DIRS
	include )

# try to find roi.h, DefaultAccessor.h, Reconstruction.h
find_path( modPET_INCLUDE_DIR
	NAMES
		PET/mod/Helpers/roi.h
		PET/mod/Helpers/roi/DefaultAccessor.h
		PET/mod/Reconstruction/Reconstruction.h
	PATHS
		${_MODPET_POSSIBLE_ROOT_DIRS}
	PATH_SUFFIXES
		${_MODPET_INCLUDE_DIRS} )

# no libraries

if( modPET_INCLUDE_DIR )
	set( MODPET_FOUND
		TRUE )
else()
	set( MODPET_FOUND
		FALSE )
endif()
