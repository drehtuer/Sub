#ifndef SUBSURFACESCATTER_READER_H
#define SUBSURFACESCATTER_READER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "DXLib/Utilities.h"

typedef unsigned char uchar;
typedef unsigned int uint;

// 4*3
#define LASHESSHAPE_OBJCOORD_VERTICES_START 88
#define LASHESSHAPE_OBJCOORD_VERTICES_END 663640

// 4*3
#define LASHESSHAPE_OBJCOORD_NORMALS_START 663685
#define LASHESSHAPE_OBJCOORD_NORMALS_END 1327237

#define LASHES_TRIANGLE_COUNT 28


// 4*3
#define HEADSHAPE_OBJCOORD_VERTICES_START 1493893
#define HEADSHAPE_OBJCOORD_VERTICES_END 1868233

// 4*3
#define HEADSHAPE_OBJCOORD_NORMALS_START 1868278
#define HEADSHAPE_OBJCOORD_NORMALS_END 2242618

// 4*3
#define HEADSHAPE_TEXCOORD_START 2242664
#define HEADSHAPE_TEXCOORD_END 2492224

#define HEAD_TRIANGLE_COUNT 4
#define HEAD_TRIANGLE_STRIPS_COUNT 61

namespace SubSurfaceScatter {

	struct vertex {
		float x;
		float y;
		float z;
	};

	struct texCoord {
		float u;
		float v;
	};

	struct normal {
		float x;
		float y;
		float z;
	};

	struct triangle {
		int f1;
		int f2;
		int f3;
	};

	struct triangleStrip {
		int f1;
		int f2;
		int f3;
	};


	class DLLE Reader {
	public:
		Reader();

		void open(std::string Filename);
		void close();
	
	
		std::vector<vertex> getHeadVertices();
		std::vector<normal> getHeadNormals();
		std::vector<triangle> getHeadTriangles();
		std::vector<std::vector<int> > getHeadTriangleStrips();
		std::vector<vertex> getLashesVertices();
		std::vector<normal> getLashesNormals();
		std::vector<triangle> getLashesTriangles();
		std::vector<texCoord> getHeadTexCoords();
	
	private:
		void reverse(char *original, char *reversed, int buffersize) const;
		void reverse(char *original, int buffersize) const;
		void readBuffer(char *buffer, int buffersize);
		void readRBuffer(char *buffer, int buffersize);
		std::string printBuffer(char *buffer, int buffersize) const;
		std::string printBuffer(uchar *buffer, int buffersize) const;
		bool compareBuffers(char *buffer1, char *buffer2, int buffersize) const;
		bool compareBuffers(uchar *buffer1, char *buffer2, int buffersize) const;
		bool compareBuffers(char *buffer1, uchar *buffer2, int buffersize) const;
		int bufferValue(char *buffer, int buffersize) const;
		float bufferValuef(char *buffer) const;
		void switchBytes(char* originalBuffer, char *switchedBuffer, int buffersize) const;

		long m_head_trinagles_start[HEAD_TRIANGLE_COUNT];
		long m_head_trinagles_end[HEAD_TRIANGLE_COUNT];
		long m_head_triangleStrips_start[HEAD_TRIANGLE_STRIPS_COUNT];
		long m_head_triangleStrips_end[HEAD_TRIANGLE_STRIPS_COUNT];
		long m_lashes_triangle_start[LASHES_TRIANGLE_COUNT];
		long m_lashes_triangle_end[LASHES_TRIANGLE_COUNT];

		std::string m_Filename;
		std::ifstream m_File;
	};
};

#endif