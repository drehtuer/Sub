set( ATB_SEARCH_DIR
	"extern/assimp" )
find_path( Assimp_INCLUDE_DIR
	aiAnim.h
	aiAssert.h
	aiCamera.h
	aiColor4D.h
	aiConfig.h
	aiDefines.h
	aiFileIO.h
	aiLight.h
	aiMaterial.h
	aiMatrix3x3.h
	aiMatrix4x4.h
	aiMesh.h
	aiPostProcess.h
	aiQuaternion.h
	aiScene.h
	aiTexture.h
	aiTypes.h
	aiVersion.h
	aiVector2D.h
	aiVector3D.h
	assimp.h
	assimp.hpp
	DefaultLogger.h
	export.h
	export.hpp
	IOStream.h
	IOSystem.h
	Logger.h
	LogStream.h
	NullLogger.h
	ProgressHandler.h
	PATH extern/assimp )
#message( STATUS "Assimp header: " ${Assimp_INCLUDE_DIR} )

set( AI_LIBS 
	assimp )
set( Assimp_LIBRARIES )
find_library( Assimp_LIBRARIES
	${AI_LIBS}
	PATH extern/assimp	)
#message( STATUS "Assimp libs: " ${Assimp_LIBRARIES} )

include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( Assimp
	DEFAULT_MSG
	Assimp_INCLUDE_DIR
	Assimp_LIBRARIES )
