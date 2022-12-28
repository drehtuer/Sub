#include "DXLib/Sphere.h"
#include <cmath>
#include "DXLib/ShaderManager.h"

using namespace SubSurfaceScatter;

Sphere::Sphere()
	: m_InputBuffer(NULL),
	  m_IndexBuffer(NULL),
	  m_Device(NULL),
	  m_Context(NULL),
	  m_radius(0.0f),
	  m_slices(10),
	  m_stacks(10)
{
}

Sphere::~Sphere() {
	safe_delete(m_InputBuffer);
	safe_delete(m_IndexBuffer);
}

void Sphere::init(ID3D11Device *Device, ID3D11DeviceContext *Context, const float radius, const UINT slices, const UINT stacks) {
	m_Device = Device;
	m_Context = Context;

	m_slices = slices;
	m_stacks = stacks;

	setRadius(radius);
}

void Sphere::setShaders(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
	m_VSName = VSName;
	m_GSName = GSName;
	m_PSName = PSName;
	m_CSName = CSName;
}

void Sphere::draw() {
	UINT stride = sizeof(Point3dVNT);
	UINT offset = 0;

	ShaderManager *Shaders = ShaderManager::getInstance();
	m_Context->VSSetShader(Shaders->getVS(m_VSName), NULL, 0);
	m_Context->GSSetShader(Shaders->getGS(m_GSName), NULL, 0);
	m_Context->PSSetShader(Shaders->getPS(m_PSName), NULL, 0);
	m_Context->CSSetShader(Shaders->getCS(m_CSName), NULL, 0);

	m_Context->IASetVertexBuffers(0, 1, &m_InputBuffer, &stride, &offset);
	m_Context->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Context->DrawIndexed(m_faceCount * 3, 0, 0);
}

bool Sphere::setRadius(const float radius) {
	HRESULT hr = S_OK;

	if(radius == m_radius)
		return false;

	safe_delete(m_InputBuffer);
	safe_delete(m_IndexBuffer);

	m_radius = radius;
	std::vector<Point3dVNT> Vertices;
	std::vector<DWORD> Indices;

	buildStacks(m_radius, Vertices, Indices);

	m_vertexCount = (UINT)Vertices.size();
	m_faceCount = (UINT)Indices.size()/3;


	D3D11_BUFFER_DESC BufferDesc;
	ZEROMEM(BufferDesc);
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(Point3dVNT) * m_vertexCount;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA Subres;
	ZEROMEM(Subres);
	Subres.pSysMem = &Vertices[0];
	chkHr(m_Device->CreateBuffer(&BufferDesc, &Subres, &m_InputBuffer));


	ZEROMEM(BufferDesc);
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(DWORD) * m_faceCount * 3;
	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;

	ZEROMEM(Subres);
	Subres.pSysMem = &Indices[0];
	chkHr(m_Device->CreateBuffer(&BufferDesc, &Subres, &m_IndexBuffer));

	return true;
}

// this function is based on Frank D. Luna's book  'Introduction To 3D Game Programming With Direct3D 10'
void Sphere::buildStacks(const float radius, std::vector<Point3dVNT> &Vertices, std::vector<DWORD> &Indices) {
	float phiStep = bmc::pi<float>()/m_stacks;
	UINT rings = m_stacks-1;

	// vertices for each stack ring
	float phi, theta, thetaStep;
	for(UINT i=1; i<=rings; ++i) {
		phi = i * phiStep;
		thetaStep = 2.0f * bmc::pi<float>() / m_slices;
		for(UINT j=0; j<=m_slices; ++j) {
			theta = j * thetaStep;
			Point3dVNT P;
			P.position = D3DXVECTOR3(radius * sinf(phi) * cosf(theta), radius * cosf(phi), radius * sinf(phi) * sinf(theta));
			D3DXVec3Normalize(&P.normal, &P.position);
			P.texCoord = D3DXVECTOR2(theta / (2.0f * bmc::pi<float>()), phi / bmc::pi<float>());

			Vertices.push_back(P);
		}
	}
	Point3dVNT P;
	// south pole
	P.position = D3DXVECTOR3(0.0f, -radius, 0.0f);
	P.normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	P.texCoord = D3DXVECTOR2(0.0f, 1.0f);
	Vertices.push_back(P);
	// north pole
	P.position = D3DXVECTOR3(0.0f, radius, 0.0f);
	P.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	P.texCoord = D3DXVECTOR2(0.0f, 0.0f);
	Vertices.push_back(P);

	UINT northPoleIndex = (UINT)Vertices.size() - 1;
	UINT southPoleIndex = (UINT)Vertices.size() - 2;

	UINT ringVertices = m_slices + 1;

	// indices for innter stack
	for(UINT i=0; i<m_stacks-2; ++i) {
		for(UINT j=0; j<m_stacks; ++j) {
			Indices.push_back(i * ringVertices + j);
			Indices.push_back(i * ringVertices + j + 1);
			Indices.push_back((i+1) * ringVertices + j);

			Indices.push_back((i+1) * ringVertices + j);
			Indices.push_back(i * ringVertices + j + 1);
			Indices.push_back((i+1) * ringVertices + j + 1);
		}
	}

	// indices for top stack
	for(UINT i=0; i<m_slices; ++i) {
		Indices.push_back(northPoleIndex);
		Indices.push_back(i+1);
		Indices.push_back(i);
	}

	// indices for bottom stack
	UINT baseIndex = (rings-1) * ringVertices;
	for(UINT i=0; i<m_slices; ++i) {
		Indices.push_back(southPoleIndex);
		Indices.push_back(baseIndex+i);
		Indices.push_back(baseIndex+i+1);
	}
}
