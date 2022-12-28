create_library( DXLib
	ADD_PATHS ../../shaders
	LINK	${DirectX_LIBRARIES}
			${AntTweakBar_LIBRARIES}
            Pfm )
