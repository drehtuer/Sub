#include "DXLib/Utilities.h"
#include "DXLib/MeshImporter.h"
#include <iostream>

using namespace SubSurfaceScatter;



int main(int argc, char *argv[]) {
	std::cout<<"Switching face orientation of model '"<<argv[1]<<"'"<<std::endl;
	std::cout<<"New mesh will be saved as '"<<argv[2]<<"'"<<std::endl;

	std::cout<<"Loading model..."<<std::endl;
	Model M;
	MeshImporter MI;
	MI.read(argv[1], M);
	std::cout << "Model loaded"<<std::endl;

	// switch f2 and f3 -> changed orientation of mesh
	DWORD fx, nx, tx;
	for(size_t m=0; m<2/*M.Meshes.size()*/; ++m) {
		std::cout<<"\tMesh: "<<M.Meshes[m].MeshName<<" ("<<num2str(M.Meshes[m].Faces.size())<<" faces)"<<std::endl;
		for(size_t f=0; f<M.Meshes[m].Faces.size(); ++f) {
			/*fx = M.Meshes[m].Faces[f].f2;
			M.Meshes[m].Faces[f].f2 = M.Meshes[m].Faces[f].f3;
			M.Meshes[m].Faces[f].f3 = fx;*/

			//nx = M.Meshes[m].Faces[f].n2;
            M.Normals[M.Meshes[m].Faces[f].n1].x = -M.Normals[M.Meshes[m].Faces[f].n1].x;
            M.Normals[M.Meshes[m].Faces[f].n1].y = -M.Normals[M.Meshes[m].Faces[f].n1].y;
            M.Normals[M.Meshes[m].Faces[f].n1].z = -M.Normals[M.Meshes[m].Faces[f].n1].z;
			M.Normals[M.Meshes[m].Faces[f].n2].x = -M.Normals[M.Meshes[m].Faces[f].n2].x;
            M.Normals[M.Meshes[m].Faces[f].n2].y = -M.Normals[M.Meshes[m].Faces[f].n2].y;
            M.Normals[M.Meshes[m].Faces[f].n2].z = -M.Normals[M.Meshes[m].Faces[f].n2].z;
            M.Normals[M.Meshes[m].Faces[f].n3].x = -M.Normals[M.Meshes[m].Faces[f].n3].x;
            M.Normals[M.Meshes[m].Faces[f].n3].y = -M.Normals[M.Meshes[m].Faces[f].n3].y;
            M.Normals[M.Meshes[m].Faces[f].n3].z = -M.Normals[M.Meshes[m].Faces[f].n3].z;

			/*tx = M.Meshes[m].Faces[f].t2;
			M.Meshes[m].Faces[f].t2 = M.Meshes[m].Faces[f].t3;
			M.Meshes[m].Faces[f].t3 = M.Meshes[m].Faces[f].t2;*/
		}
	}
	std::cout<<"Switched everything, now saving ..."<<std::endl;
	MI.exportTo(argv[2], M);
	std::cout<<"All done."<<std::endl;
}