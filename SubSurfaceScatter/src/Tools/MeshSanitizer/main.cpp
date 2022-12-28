#include "DXLib/Utilities.h"
#include "DXLib/MeshImporter.h"

using namespace SubSurfaceScatter;

size_t checkFace(Model &M, size_t m, size_t f, size_t mm, size_t ff) {
	size_t counter = 0;
	// f1 -> f1
	// same vertex index?
	if(M.Meshes[m].Faces[f].f1 == M.Meshes[mm].Faces[ff].f1) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t1 != M.Meshes[mm].Faces[ff].t1) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f1]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n1]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f1 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	// f1 -> f2
	// same vertex index?
	if(M.Meshes[m].Faces[f].f1 == M.Meshes[mm].Faces[ff].f2) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t1 != M.Meshes[mm].Faces[ff].t2) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f1]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n1]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f2 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	// f1 -> f3
	// same vertex index?
	if(M.Meshes[m].Faces[f].f1 == M.Meshes[mm].Faces[ff].f3) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t1 != M.Meshes[mm].Faces[ff].t3) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f1]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n1]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f3 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}


	// f2 -> f1
	// same vertex index?
	if(M.Meshes[m].Faces[f].f2 == M.Meshes[mm].Faces[ff].f1) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t2 != M.Meshes[mm].Faces[ff].t1) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f2]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n2]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f1 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	// f2 -> f2
	// same vertex index?
	if(M.Meshes[m].Faces[f].f2 == M.Meshes[mm].Faces[ff].f2) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t2 != M.Meshes[mm].Faces[ff].t2) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f2]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n2]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f2 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	// f2 -> f3
	// same vertex index?
	if(M.Meshes[m].Faces[f].f2 == M.Meshes[mm].Faces[ff].f3) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t2 != M.Meshes[mm].Faces[ff].t3) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f2]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n2]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f3 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}


	// f3 -> f1
	// same vertex index?
	if(M.Meshes[m].Faces[f].f3 == M.Meshes[mm].Faces[ff].f1) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t3 != M.Meshes[mm].Faces[ff].t1) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f3]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n3]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f1 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	// f3 -> f2
	// same vertex index?
	if(M.Meshes[m].Faces[f].f3 == M.Meshes[mm].Faces[ff].f2) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t3 != M.Meshes[mm].Faces[ff].t2) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f3]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n3]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f2 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	// f3 -> f3
	// same vertex index?
	if(M.Meshes[m].Faces[f].f3 == M.Meshes[mm].Faces[ff].f3) {
		// texcoords differ?
		if(M.Meshes[m].Faces[f].t3 != M.Meshes[mm].Faces[ff].t3) {
			// duplicate vertex
			M.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f3]);
			M.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n3]);
			// change vertex in ff face
			M.Meshes[mm].Faces[ff].f3 = (DWORD)M.Vertices.size()-1;
			++counter;
		}
	}

	return counter;
}

int main(int argc, char *argv[]) {
	std::cout << "Sanitizing model '" << argv[1] << "'"<<std::endl<<"\t(so it will be easily compatible with DirectX)."<<std::endl;
    std::cout << "Sanitized model will be written to: "<<argv[2]<<std::endl;

	LOGG << "Loading model...";
	Model M, MSanitized;
	MeshImporter MI;
	MI.read(argv[1], M);
	LOGG << "Model loaded.";

	LOGG << "Checks if vertices need to be duplicated to avoid ugly texture seams.";
	LOGG << "If two vertices have different texcoords or normals, they need to be";
	LOGG << "duplicated (DirectX doesn't support texcoord or normal indices).\n";
	LOGG << "\tModel: "+M.ModelName;

    MSanitized.ModelName = M.ModelName;
    
    // for all meshes
    for(size_t m=0; m<M.Meshes.size(); ++m) {
        LOGG << "\t\tMesh: "+M.Meshes[m].MeshName+" ("+num2str(M.Meshes[m].Faces.size())+" faces)";
        Mesh Msh;
        Msh.MeshName = M.Meshes[m].MeshName;
        MSanitized.Meshes.push_back(Msh);
        // for all faces
        for(size_t f=0; f<M.Meshes[m].Faces.size(); ++f) {
            MeshFace MshFc;
            // copy vertices & vertex indices
            MSanitized.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f1]);
            MshFc.f1 = (DWORD)MSanitized.Vertices.size()-1;
            MSanitized.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f2]);
            MshFc.f2 = (DWORD)MSanitized.Vertices.size()-1;
            MSanitized.Vertices.push_back(M.Vertices[M.Meshes[m].Faces[f].f3]);
            MshFc.f3 = (DWORD)MSanitized.Vertices.size()-1;

            // copy normals & normal indices
            MSanitized.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n1]);
            MshFc.n1 = (DWORD)MSanitized.Normals.size()-1;
            MSanitized.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n2]);
            MshFc.n2 = (DWORD)MSanitized.Normals.size()-1;
            MSanitized.Normals.push_back(M.Normals[M.Meshes[m].Faces[f].n3]);
            MshFc.n3 = (DWORD)MSanitized.Normals.size()-1;


            // copy texcoords & texcoord indices
            MSanitized.TexCoords.push_back(M.TexCoords[M.Meshes[m].Faces[f].t1]);
            MshFc.t1 = (DWORD)MSanitized.TexCoords.size()-1;
            MSanitized.TexCoords.push_back(M.TexCoords[M.Meshes[m].Faces[f].t2]);
            MshFc.t2 = (DWORD)MSanitized.TexCoords.size()-1;
            MSanitized.TexCoords.push_back(M.TexCoords[M.Meshes[m].Faces[f].t3]);
            MshFc.t3 = (DWORD)MSanitized.TexCoords.size()-1;
            
            MSanitized.Meshes[m].Faces.push_back(MshFc);
        }
    }

	//size_t counter = 0;
	//for(size_t m=0; m<M.Meshes.size(); ++m) {
	//	LOGG << "\t\tMesh: "+M.Meshes[m].MeshName+" ("+num2str(M.Meshes[m].Faces.size())+" faces)";

	//	// compare with these faces
	//	for(size_t mm=m; mm<M.Meshes.size(); ++mm) {
	//		LOGG << "\t\t\tChecking in Mesh: "+M.Meshes[mm].MeshName+" ("+num2str(M.Meshes[mm].Faces.size())+" faces)";

	//		for(size_t f=0; f<M.Meshes[m].Faces.size(); ++f) {

	//			for(size_t ff=f+1; ff<M.Meshes[mm].Faces.size(); ++ff) {
 //                   counter += checkFace(M, m, f, mm, ff);
	//			}
	//		}
	//		LOGG << "\t\t\t\tduplicated vertex count: "+num2str(counter);
	//	}
	//}
	//LOGG << "\tDuplicated " + num2str(counter) + " vertices.";
	std::cout << "Writing sanitized mesh to "<<argv[1]<<std::endl;
    MI.exportTo(argv[2], MSanitized);
	LOGG << "All done.";

	return 0;
}