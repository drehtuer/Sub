#ifndef SUBSURFACESCATTER_MESHIMPORTER_H
#define SUBSURFACESCATTER_MESHIMPORTER_H

#include "DXLib/Utilities.h"
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <map>


namespace SubSurfaceScatter {

    struct MeshVertex {
        float x, y, z;
    };

    struct MeshNormal {
        float x, y, z;
    };

    struct MeshTexCoord {
        float u, v;
	};

    struct MeshFace {
        DWORD f1, f2, f3;
        DWORD n1, n2, n3;
        DWORD t1, t2, t3;
    };

    struct Mesh {
        std::string MeshName;
        std::vector<MeshFace> Faces;
    };

    struct Model {
        std::string ModelName;
        std::vector<MeshVertex> Vertices;
        std::vector<MeshNormal> Normals;
        std::vector<MeshTexCoord> TexCoords;
        std::vector<Mesh> Meshes;
    };

    static const float g_digitFactor[] = {
        0.1f,               // 1
        0.01f,              // 2
        0.001f,             // 3
        0.0001f,            // 4
        0.00001f,           // 5
        0.000001f,          // 6
        0.0000001f,         // 7
        0.00000001f,        // 8
        0.000000001f,       // 9
        0.0000000001f,      // 10
        0.00000000001f,     // 11
        0.000000000001f,    // 12
        0.0000000000001f,   // 13
        0.00000000000001f,  // 14
        0.000000000000001f, // 15
        0.0000000000000001f // 16
    };

    class DLLE MeshImporter {
    public:
        bool read(const char *Filename, Model &LoadModel) const;
        void printInfos(const Model &LoadMesh) const;
        void getBoundingBox(const Model &LoadModel, std::map<std::string, float> &BoundingBox) const;
		void exportTo(const std::string &Filename, const Model &LoadMesh) const;

    private:
        bool preparse(const std::vector<char> &Buffer, Model &LoadModel) const;
        bool parse(const std::vector<char> &Buffer, Model &LoadModel) const;

        inline void getString(const std::vector<char> &Buffer, boost::uintmax_t i, std::string &Str) const {
            Str.clear();
            char c = Buffer[++i];
            while(c != '\n') {
                Str.push_back(c);
                c = Buffer[++i];
            }
        };

        inline void addMesh(const std::string MeshName, const unsigned long numFaces, Model &LoadModel) const {
            if(numFaces > 0) {
                for(size_t i=0; i<LoadModel.Meshes.size(); ++i) {
                    if(LoadModel.Meshes[i].MeshName == MeshName) {
                        size_t oldNumFaces = LoadModel.Meshes[i].Faces.size();
                        LoadModel.Meshes[i].Faces.reserve(numFaces + oldNumFaces);
                        LoadModel.Meshes[i].Faces.resize(numFaces + oldNumFaces);
                        return;
                    }
                }

                Mesh M;
                M.MeshName = MeshName;
                LoadModel.Meshes.push_back(M);
                // no need to copy the resized mesh struct
                LoadModel.Meshes.back().Faces.reserve(numFaces);
                LoadModel.Meshes.back().Faces.resize(numFaces);
            }
        };

        inline float getFloat(const std::vector<char> &Buffer, boost::uintmax_t &i) const {
            /*char c = Buffer[++i];
            char tmp[20];
            int pos = 0;
            while(c != ' ' && c != '\n') {
                tmp[pos++] = c;
                c = Buffer[++i];
            }
            tmp[pos] = ' ';
            return atof(tmp);*/
            
            char c = Buffer[++i];
            float f = 0.0f, f_tmp;
            unsigned int digitPos = 0;
            bool isNegative = false, isAfterPoint = false;
            while(c != ' ' && c != '\n') {
                switch(c) {
                case '-':
                    isNegative = true;
                    break;

                case '.':
                    isAfterPoint = true;
                    break;

                default:
                    if(isAfterPoint) {
                        f_tmp = (float)(c - 48);
                        f_tmp *= g_digitFactor[digitPos++];
                    } else {
                        f_tmp = (float)(c - 48);
                        f *= 10;
                    }
                    f += f_tmp;
                    break;
                }
                c = Buffer[++i];
            }

            if(isNegative)
                f *= -1;

            return f;
        };

        inline void getDWORD(const std::vector<char> &Buffer, boost::uintmax_t &i, DWORD &f, DWORD &t, DWORD &n) const {
            f = 0;
            t = 0;
            n = 0;

            char c = Buffer[++i];

            // vertex index
            while(c != ' ' && c != '/' && c != '\n') {
                f = f * 10 + c - 48;
                c = Buffer[++i];
            }
            // tex coords index
            if(Buffer[i] == '/') {
                c = Buffer[++i];
                while(c != ' ' && c != '/' && c != '\n') {
                    t = t * 10 + c - 48;
                    c = Buffer[++i];
                }

                // normal index
                if(Buffer[i] == '/') {
                    c = Buffer[++i];
                    while(c != ' ' && c != '/' && c != '\n') {
                        n = n * 10 + c - 48;
                        c = Buffer[++i];
                    }
                }
            }
        };

        inline void getVector(const std::vector<char> &Buffer, boost::uintmax_t i, MeshVertex &Vertex) const {
            Vertex.x = getFloat(Buffer, i);
            Vertex.y = getFloat(Buffer, i);
            Vertex.z = getFloat(Buffer, i);
        };

        inline void getNormal(const std::vector<char> &Buffer, boost::uintmax_t i, MeshNormal &Normal) const {
            Normal.x = getFloat(Buffer, i);
            Normal.y = getFloat(Buffer, i);
            Normal.z = getFloat(Buffer, i);
        };

        inline void getTexCoord(const std::vector<char> &Buffer, boost::uintmax_t i, MeshTexCoord &TexCoord) const {
            TexCoord.u = getFloat(Buffer, i);
            TexCoord.v = getFloat(Buffer, i);
            // w not used
        };

        inline void getFace(const std::vector<char> &Buffer, boost::uintmax_t i, MeshFace &MF) const {
            getDWORD(Buffer, i, MF.f1, MF.t1, MF.n1);
            MF.f1--; // obj index starts at 1;
            MF.t1--;
            MF.n1--;
            getDWORD(Buffer, i, MF.f2, MF.t2, MF.n2);
            MF.f2--;
            MF.t2--;
            MF.n2--;
            getDWORD(Buffer, i, MF.f3, MF.t3, MF.n3);
            MF.f3--;
            MF.t3--;
            MF.n3--;
        };
    };
}

#endif
