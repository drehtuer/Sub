#include "DXLib/Quad.h"
#include <vector>
#include "DXLib/ShaderManager.h"
#include <D3DX10math.h>

using namespace SubSurfaceScatter;

Quad::Quad()
	: m_InputBuffer(NULL),
	  m_IndexBuffer(NULL)
{
}

Quad::~Quad() {
	safe_delete(m_InputBuffer);
	safe_delete(m_IndexBuffer);
}

void Quad::setShaders(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
	m_VSName = VSName;
	m_GSName = GSName;
	m_PSName = PSName;
	m_CSName = CSName;
}

// this function is based on Frank D. Luna's book  'Introduction To 3D Game Programming With Direct3D 10'
void Quad::init(ID3D11Device *Device, ID3D11DeviceContext *Context, const DWORD pointsX, const DWORD pointsY, const float cellSpaceing) {
	HRESULT hr = S_OK;

	m_Context = Context;

	m_vertexCount = pointsX * pointsY;
	m_faceCount = (pointsX - 1) * (pointsY - 1) * 2;

	std::vector<Point3dVNT> VNT(m_vertexCount);
	float halfWidth = (pointsY - 1) * cellSpaceing * 0.5f;
	float halfDepth = (pointsX - 1) * cellSpaceing * 0.5f;

	float du = 1.0f / (pointsY - 1);
	float dv = 1.0f / (pointsX - 1);

	// input buffer
	float z = 0.0f;
	for(DWORD i=0; i<pointsX; ++i) {
		float y = halfDepth - i * cellSpaceing;
		for(DWORD j=0; j<pointsY; ++j) {
			float x = -halfWidth + j * cellSpaceing;
			VNT[i * pointsY + j].position = D3DXVECTOR3(x, y, z);
			VNT[i * pointsY + j].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
			VNT[i * pointsY + j].texCoord = D3DXVECTOR2(j * du, i * dv);
		}
	}
	
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(Point3dVNT) * m_vertexCount;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA Subres;
	ZeroMemory(&Subres, sizeof(Subres));
	Subres.pSysMem = &VNT[0];
	chkHr(Device->CreateBuffer(&BufferDesc, &Subres, &m_InputBuffer));

	// index buffer
	std::vector<DWORD> Indices(m_faceCount * 3);

	int k=0;
	for(DWORD i=0; i<pointsX-1; ++i) {
		for(DWORD j=0; j<pointsY-1; ++j) {
			Indices[k++] = i * pointsY + j;
			Indices[k++] = i * pointsY + j +1;
			Indices[k++] = (i + 1) * pointsY + j;
			Indices[k++] = (i + 1) * pointsY + j;
			Indices[k++] = i * pointsY + j + 1;
			Indices[k++] = (i + 1) * pointsY + j + 1;
		}
	}

	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(DWORD) * m_faceCount * 3;
	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;

	ZeroMemory(&Subres, sizeof(Subres));
	Subres.pSysMem = &Indices[0];
	chkHr(Device->CreateBuffer(&BufferDesc, &Subres, &m_IndexBuffer));
}

void Quad::draw() {
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
