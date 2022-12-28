#include "DXLib/MeshImporter.h"
#include <cmath>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a < b ? b : a)

using namespace SubSurfaceScatter;

bool MeshImporter::read(const char *Filename, Model &LoadModel) const {
    // get filesize
    boost::filesystem::path FilePath(Filename);
    boost::uintmax_t fileSize;
    try {
    boost::filesystem::path FilePath(Filename);
    fileSize = boost::filesystem::file_size(FilePath);
    } catch(...) {
        LOGG << "Could not open '"+std::string(Filename);
        return false;
    }

    // prepare memory buffer
    std::vector<char> MemoryBuffer;
    MemoryBuffer.reserve(fileSize+1);
    MemoryBuffer.resize(fileSize+1);

    // read file
    FILE *f = fopen(Filename, "rb");
    if(!f) {
        LOGG<< "Could not open file: '" + std::string(Filename) + "'";
        return false;
    }
    fread(&MemoryBuffer[0], sizeof(char), fileSize, f);
    fclose(f);

    // add a terminator at the end, just to be sure
    MemoryBuffer[fileSize] = ' ';

    // ModelName = Filename
    LoadModel.ModelName = Filename;
    std::string::size_type pos = LoadModel.ModelName.find_last_of("\\/");
    if(pos != std::string::npos)
        LoadModel.ModelName = LoadModel.ModelName.substr(pos+1, LoadModel.ModelName.size() - pos - 5);

    // preparse step: set all vectors to correct size
    if(!preparse(MemoryBuffer, LoadModel)) {
        LOGG << "Parsing error in file '" + std::string(Filename) + "'";
        return false;
    }

    // actual parse step: fill vectors
    if(!parse(MemoryBuffer, LoadModel)) {
        LOGG << "Parsing error in file '" + std::string(Filename) + "'";
        return false;
    }

    return true;
}

void MeshImporter::printInfos(const Model &LoadModel) const {
    LOGG << "\t\tName: " + LoadModel.ModelName;
    LOGG << "\t\tNumber of vertices: " + num2str(LoadModel.Vertices.size());
    LOGG << "\t\tNumber of normals: " + num2str(LoadModel.Normals.size());
    LOGG << "\t\tNumber of tex coords: " + num2str(LoadModel.TexCoords.size());
    LOGG << "\t\tNumber of meshes: " + num2str(LoadModel.Meshes.size());
    size_t sumFaces = 0;
    for(size_t i=0; i<LoadModel.Meshes.size(); ++i) {
        LOGG << std::string("\t\t\tMesh: ") + LoadModel.Meshes[i].MeshName + std::string(" with ") + num2str(LoadModel.Meshes[i].Faces.size()) + " faces";
        sumFaces += LoadModel.Meshes[i].Faces.size();
    }
    LOGG << "\t\tTotal of " + num2str(sumFaces) + " faces";
}

// skips all entries except for v, vn, vt, f, g, o
bool MeshImporter::preparse(const std::vector<char> &Buffer, Model &LoadModel) const {
    unsigned long numVertices = 0, numNormals = 0, numTexCoords = 0;
    unsigned long numFaces = 0;
    size_t lines = 0;
    std::string MeshName = "Mesh";
    for(boost::uintmax_t i=0; i<(boost::uintmax_t)Buffer.size(); ++i) {
        if(Buffer[i] == '\n')
            lines++;
        switch(Buffer[i]) {
        case 'v':
            if(Buffer[i-1] == '\n') {
                switch(Buffer[i+1]) {
                case ' ':
                    ++numVertices;
                    break;

                case 'n':
                    if(Buffer[i+2] == ' ')
                        ++numNormals;
                    break;

                case 't':
                    if(Buffer[i+2] == ' ')
                        ++numTexCoords;
                    break;

                default:
                    break;
                }
            }
            break;

        case 'f':
            if(Buffer[i-1] == '\n' && Buffer[i+1] == ' ')
                ++numFaces;
            break;

        /*case 'u':
            if(Buffer[i-1] == '\n' && Buffer[i+1] == 's' && Buffer[i+2] == 'e' && Buffer[i+3] == 'm' && Buffer[i+4] == 't' && Buffer[i+5] == 'l' && Buffer[i+6] == ' ') {
            
            }
            break;*/

        case 'g':
        case 'o':
            if(Buffer[i-1] == '\n' && Buffer[i+1] == ' ') {
                addMesh(MeshName, numFaces, LoadModel);
                getString(Buffer, i+1, MeshName);
                numFaces = 0;
            }
            break;

        default:
            break;
        }
    }

    // set vertex, normal & texcoord size
    LoadModel.Vertices.reserve(numVertices);
    LoadModel.Vertices.resize(numVertices);
    
    LoadModel.Normals.reserve(numNormals);
    LoadModel.Normals.resize(numNormals);

    LoadModel.TexCoords.reserve(numTexCoords);
    LoadModel.TexCoords.resize(numTexCoords);

    // remaining mesh size
    addMesh(MeshName, numFaces, LoadModel);

    return true;
}

bool MeshImporter::parse(const std::vector<char> &Buffer, Model &LoadModel) const {
    size_t vertexIndex = 0, normalIndex = 0, texCoordIndex = 0, meshIndex = 0, faceIndex = 0;
    std::string MeshName = "Mesh";
    std::vector<size_t> lastFaceItem(LoadModel.Meshes.size());
    for(boost::uintmax_t i=0; i<(boost::uintmax_t)Buffer.size(); ++i) {
        switch(Buffer[i]) {
        case 'v':
            if(Buffer[i-1] == '\n') {
                switch(Buffer[i+1]) {
                case ' ':
                    getVector(Buffer, i+1, LoadModel.Vertices[vertexIndex++]);
                    break;

                case 'n':
                    if(Buffer[i+2] == ' ')
                        getNormal(Buffer, i+2, LoadModel.Normals[normalIndex++]);
                    break;

                case 't':
                    if(Buffer[i+2] == ' ')
                        getTexCoord(Buffer, i+2, LoadModel.TexCoords[texCoordIndex++]);
                    break;

                default:
                    break;
                }
            }
            break;

        case 'g':
        case 'o':
            if(Buffer[i-1] == '\n' && Buffer[i+1] == ' ') {
                lastFaceItem[meshIndex] = faceIndex;
                getString(Buffer, i+1, MeshName);
                for(meshIndex = 0; meshIndex < LoadModel.Meshes.size(); ++meshIndex) {
                    if(LoadModel.Meshes[meshIndex].MeshName == MeshName) {
                        faceIndex = lastFaceItem[meshIndex];
                        break;
                    }
                }
            }
            break;

        case 'f':
            if(Buffer[i-1] == '\n' && Buffer[i+1] == ' ') {
                getFace(Buffer, i+1, LoadModel.Meshes[meshIndex].Faces[faceIndex++]);
            }
            break;

        default:
            break;
        }
    }
    return true;
}

void MeshImporter::exportTo(const std::string &Filename, const Model &LoadMesh) const {
	LOGG << "Exporting mesh '"+LoadMesh.ModelName+"' to file '"+Filename+"'";
	
	size_t maxsize = 0;
	maxsize += 2 * 50;
	maxsize += 3 * 20 * LoadMesh.Vertices.size();
	maxsize += 3 * 20 * LoadMesh.Normals.size();
	maxsize += 2 * 20 * LoadMesh.TexCoords.size();
	for(size_t m=0; m<LoadMesh.Meshes.size(); ++m)
		maxsize += 2*50 + 3 * 30 * LoadMesh.Meshes[m].Faces.size();
	char *buffer = new char[maxsize];

	char *c = &buffer[0];

	// comments
	c += sprintf(c, "# vertices: %d, normals: %d, tex coords: %d\n", LoadMesh.Vertices.size(), LoadMesh.Normals.size(), LoadMesh.TexCoords.size());
	size_t numFaces = 0;
	for(size_t i=0; i<LoadMesh.Meshes.size(); ++i)
		numFaces += LoadMesh.Meshes[i].Faces.size();
	c += sprintf(c, "# meshes: %d, faces(total): %d\n", LoadMesh.Meshes.size(), numFaces);

	// vertices
	for(size_t v=0; v<LoadMesh.Vertices.size(); ++v) {
		c += sprintf(c, "v %f %f %f\n", LoadMesh.Vertices[v].x, LoadMesh.Vertices[v].y, LoadMesh.Vertices[v].z);
	}

	// normals
	for(size_t n=0; n<LoadMesh.Normals.size(); ++n) {
		c += sprintf(c, "vn %f %f %f\n", LoadMesh.Normals[n].x, LoadMesh.Normals[n].y, LoadMesh.Normals[n].z);
	}

	// texcoords
	for(size_t t=0; t<LoadMesh.TexCoords.size(); ++t) {
		c += sprintf(c, "vt %f %f\n", LoadMesh.TexCoords[t].u, LoadMesh.TexCoords[t].v);
	}

	// for all meshes
	for(size_t m=0; m<LoadMesh.Meshes.size(); ++m) {
		// for all faces
		c += sprintf(c, "# Mesh %d, faces: %d\n",m, LoadMesh.Meshes[m].Faces.size());
		c += sprintf(c, "o %s\n", LoadMesh.Meshes[m].MeshName.c_str());
		DWORD t1, t2, t3, n1, n2, n3;
		for(size_t f=0; f<LoadMesh.Meshes[m].Faces.size(); ++f) {
			if(m == 1) {
				t1 = 0;
				t2 = 0;
				t3 = 0;
			} else {
			if(LoadMesh.Meshes[m].Faces[f].t1 == -1)
				t1 = LoadMesh.Meshes[m].Faces[f].f1;
			else
				t1 = LoadMesh.Meshes[m].Faces[f].t1;
			if(LoadMesh.Meshes[m].Faces[f].t2 == -1)
				t2 = LoadMesh.Meshes[m].Faces[f].f2;
			else
				t2 = LoadMesh.Meshes[m].Faces[f].t2;
			if(LoadMesh.Meshes[m].Faces[f].t3 == -1)
				t3 = LoadMesh.Meshes[m].Faces[f].f3;
			else
				t3 = LoadMesh.Meshes[m].Faces[f].t3;
			}
			if(LoadMesh.Meshes[m].Faces[f].n1 == -1)
				n1 = LoadMesh.Meshes[m].Faces[f].f1;
			else
				n1 = LoadMesh.Meshes[m].Faces[f].n1;
			if(LoadMesh.Meshes[m].Faces[f].n2 == -1)
				n2 = LoadMesh.Meshes[m].Faces[f].f2;
			else
				n2 = LoadMesh.Meshes[m].Faces[f].n2;
			if(LoadMesh.Meshes[m].Faces[f].n3 == -1)
				n3 = LoadMesh.Meshes[m].Faces[f].f3;
			else
				n3 = LoadMesh.Meshes[m].Faces[f].n3;
			// don't forget obj indices start at 1
			c += sprintf(c, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", LoadMesh.Meshes[m].Faces[f].f1+1, t1+1, n1+1,  LoadMesh.Meshes[m].Faces[f].f2+1, t2+1, n2+1,  LoadMesh.Meshes[m].Faces[f].f3+1, t3+1, n3+1);
		}
	}

	FILE *f = fopen(Filename.c_str(), "wb");
	fwrite(&buffer[0], sizeof(char), (c - buffer)/sizeof(char), f);
	// append an empty line
	fprintf(f, "\n");
	fclose(f);
	delete [] buffer;
}

void MeshImporter::getBoundingBox(const Model &LoadModel, std::map<std::string, float> &BoundingBox) const {
    BoundingBox.clear();
    BoundingBox["maxX"] = 0.0f;
    BoundingBox["minX"] = 0.0f;
    BoundingBox["maxY"] = 0.0f;
    BoundingBox["minY"] = 0.0f;
    BoundingBox["maxZ"] = 0.0f;
    BoundingBox["minZ"] = 0.0f;

    for(size_t i=0; i<LoadModel.Vertices.size(); ++i) {
        BoundingBox["maxX"] = MAX(BoundingBox["maxX"], LoadModel.Vertices[i].x);
        BoundingBox["minX"] = MIN(BoundingBox["minX"], LoadModel.Vertices[i].x);
        BoundingBox["maxY"] = MAX(BoundingBox["maxY"], LoadModel.Vertices[i].y);
        BoundingBox["minY"] = MIN(BoundingBox["minY"], LoadModel.Vertices[i].y);
        BoundingBox["maxZ"] = MAX(BoundingBox["maxZ"], LoadModel.Vertices[i].z);
        BoundingBox["minZ"] = MIN(BoundingBox["minZ"], LoadModel.Vertices[i].z);
    }
}
