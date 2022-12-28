# Try to find source and lib directories of libPET
#
# This module defines
#   PET_FOUND
#   PTC_FOUND
#   LIBPET_INC_DIR
#   LIBPTC_INC_DIR
#   LIBPET_LIBRARY
#   LIBPETGUI_LIBRARY
#   LIBPTC_LIBRARY
#
# 2010/06 Johannes Meng
# 2010/08 Karsten Brand

# typical root dirs of installations, exactly one of them is used
set(LIBPET_POSSIBLE_ROOT_DIRS
	../../../../PET
	../../../PET )

# typical include dirs above the root dirs
set( LIBPET_INCDIR_SUFFIXES
    include )

set( LIBPTC_INCDIR_SUFFIXES
    libPtc/include )

# typical lib dirs above the root dirs
set( LIBPET_LIBDIR_SUFFIXES
    lib
	lib/Linux
	lib/Windows/Release
	lib/Windows/Debug )

# try to find at least Euler.h, Face.h and Quaternion.h
find_path( LIBPET_INC_DIR
	NAMES PET/Helpers/GLCalibration.h PET/Reconstruction/CameraConfig.h PET/Reconstruction/Camera.h PET/Reconstruction/CameraManager.h PETGui/HumanConfig.h
	PATHS ${LIBPET_POSSIBLE_ROOT_DIRS}
	PATH_SUFFIXES ${LIBPET_INCDIR_SUFFIXES} )

find_path( LIBPTC_INC_DIR
	NAMES Ptc/common.h Ptc/Reader.h
	PATHS ${LIBPET_POSSIBLE_ROOT_DIRS}
	PATH_SUFFIXES ${LIBPTC_INCDIR_SUFFIXES} )

find_library( LIBPET_LIBRARY
	PET
	PATHS ${LIBPET_POSSIBLE_ROOT_DIRS}
	PATH_SUFFIXES ${LIBPET_LIBDIR_SUFFIXES} )

find_library(LIBPETGUI_LIBRARY
	PETGui
	PATHS ${LIBPET_POSSIBLE_ROOT_DIRS}
	PATH_SUFFIXES ${LIBPET_LIBDIR_SUFFIXES} )

find_library( LIBPTC_LIBRARY
	Ptc
	PATHS ${LIBPET_POSSIBLE_ROOT_DIRS}
	PATH_SUFFIXES ${LIBPET_LIBDIR_SUFFIXES} )

# handle the QUIETLY and REQUIRED arguments and set Gen3D_FOUND to TRUE if
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( PET
	DEFAULT_MSG
	LIBPET_INC_DIR
	LIBPET_LIBRARY LIBPETGUI_LIBRARY )

find_package_handle_standard_args( PTC
	Default_MSG
	LIBPTC_INC_DIR
	LIBPTC_LIBRARY )
