#include "DXLib/VirtualObject.h"
#include "DXLib/MeshImporter.h"

using namespace SubSurfaceScatter;

VirtualObject::VirtualObject()
	: MoveableObject(),
	  m_enabled(false),
	  m_ObjectName("VirtualObject")
{
	m_TextureManager = TextureManager::getInstance();
	if(!m_TextureManager->isInitialized())
		m_TextureManager->init(m_Device, m_Context);

	m_ShaderManager = ShaderManager::getInstance();
	if(!m_ShaderManager->isInitialized())
		m_ShaderManager->init(m_Device);
}

VirtualObject::~VirtualObject() {
	safe_delete_map(m_VertexBuffer);
	safe_delete_map(m_IndexBuffer);
}

HRESULT VirtualObject::init(ID3D11Device *Device, ID3D11DeviceContext *Context) {
	HRESULT hr = S_OK;

	m_Device = Device;
	m_Context = Context;

	return hr;
}

HRESULT VirtualObject::loadMesh(const std::string &Filename) {
	HRESULT hr = S_OK;

    // input description
	D3D11_INPUT_ELEMENT_DESC PosDesc    = {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0};
	m_InputDescs.push_back(PosDesc);
	D3D11_INPUT_ELEMENT_DESC NormalDesc = {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0};
	m_InputDescs.push_back(NormalDesc);
	D3D11_INPUT_ELEMENT_DESC TexDesc    = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0};
	m_InputDescs.push_back(TexDesc);


    // loading model
    MeshImporter MI;
    Model MeshModel;
	MI.read(Filename.c_str(), MeshModel);
    MI.printInfos(MeshModel);
    m_ObjectName = MeshModel.ModelName;


	m_vertexCount[m_ObjectName] = (UINT)MeshModel.Vertices.size();

    /*std::map<std::string, float> BB;
    MI.getBoundingBox(AdrianModel, BB);
    LOGG << "\t\tBounding box:";
    LOGG << "\t\t\tx: ["+num2str(BB["minX"])+", "+num2str(BB["maxX"])+"]";
    LOGG << "\t\t\ty: ["+num2str(BB["minY"])+", "+num2str(BB["maxY"])+"]";
    LOGG << "\t\t\tz: ["+num2str(BB["minZ"])+", "+num2str(BB["maxZ"])+"]";*/

    UINT texSize = (UINT)MeshModel.TexCoords.size();
    Point3dVNT *Points = new Point3dVNT[MeshModel.Vertices.size()];
    for(size_t m=0; m<MeshModel.Meshes.size(); m++) {
        for(size_t f=0; f<MeshModel.Meshes[m].Faces.size(); ++f) {
            Points[MeshModel.Meshes[m].Faces[f].f1].position = D3DXVECTOR3(MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f1].x, MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f1].y, MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f1].z);
            Points[MeshModel.Meshes[m].Faces[f].f1].normal = D3DXVECTOR3(MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n1].x, MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n1].y, MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n1].z);
            Points[MeshModel.Meshes[m].Faces[f].f1].texCoord = D3DXVECTOR2(MeshModel.TexCoords[MeshModel.Meshes[m].Faces[f].t1].u, MeshModel.TexCoords[MeshModel.Meshes[m].Faces[f].t1].v);

            Points[MeshModel.Meshes[m].Faces[f].f2].position = D3DXVECTOR3(MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f2].x, MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f2].y, MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f2].z);
            Points[MeshModel.Meshes[m].Faces[f].f2].normal = D3DXVECTOR3(MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n2].x, MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n2].y, MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n2].z);
            Points[MeshModel.Meshes[m].Faces[f].f2].texCoord = D3DXVECTOR2(MeshModel.TexCoords[MeshModel.Meshes[m].Faces[f].t2].u, MeshModel.TexCoords[MeshModel.Meshes[m].Faces[f].t2].v);

            Points[MeshModel.Meshes[m].Faces[f].f3].position = D3DXVECTOR3(MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f3].x, MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f3].y, MeshModel.Vertices[MeshModel.Meshes[m].Faces[f].f3].z);
            Points[MeshModel.Meshes[m].Faces[f].f3].normal = D3DXVECTOR3(MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n3].x, MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n3].y, MeshModel.Normals[MeshModel.Meshes[m].Faces[f].n3].z);
            Points[MeshModel.Meshes[m].Faces[f].f3].texCoord = D3DXVECTOR2(MeshModel.TexCoords[MeshModel.Meshes[m].Faces[f].t3].u, MeshModel.TexCoords[MeshModel.Meshes[m].Faces[f].t3].v);
        }
    }

    D3D11_BUFFER_DESC BufferDesc;
    ZEROMEM(BufferDesc);
    BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    BufferDesc.ByteWidth = m_vertexCount[MeshModel.ModelName] * sizeof(Point3dVNT);
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA SubresPointsData;
    ZEROMEM(SubresPointsData);
    SubresPointsData.pSysMem = Points;
    m_VertexBuffer[MeshModel.ModelName] = NULL;
    HR(chkHr(m_Device->CreateBuffer(&BufferDesc, &SubresPointsData, &m_VertexBuffer[MeshModel.ModelName])));
    d3dLiveName(m_VertexBuffer[MeshModel.ModelName], m_ObjectName+" Vertex Buffer");

    safe_delete_array(Points);
    
    for(UINT obj=0; obj<(UINT)MeshModel.Meshes.size(); ++obj) {
        Mesh *pMesh = &MeshModel.Meshes[obj];
        std::string &MeshName = MeshModel.Meshes[obj].MeshName;

        m_faceCount[MeshName] = (UINT)pMesh->Faces.size();
        DWORD *indices = new DWORD[pMesh->Faces.size() * 3];
        // only one mesh per group
        for(UINT face=0; face < pMesh->Faces.size(); ++face) {
            indices[3 * face    ] = pMesh->Faces[face].f1;
            indices[3 * face + 1] = pMesh->Faces[face].f2;
            indices[3 * face + 2] = pMesh->Faces[face].f3;
        }

        // buffer for indices
        BufferDesc.ByteWidth = m_faceCount[MeshName] * sizeof(DWORD) * 3;
        BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        
        SubresPointsData.pSysMem = indices;
        m_IndexBuffer[MeshName] = NULL;
        HR(chkHr(m_Device->CreateBuffer(&BufferDesc, &SubresPointsData, &m_IndexBuffer[MeshName])));
        d3dLiveName(m_IndexBuffer[MeshName], m_ObjectName+" Index Buffer");

        safe_delete_array(indices);
    }

	return hr;
}

ID3D11InputLayout *VirtualObject::getInputLayout() const {
	return m_ShaderManager->getIL("IL_"+m_ObjectName);
}

UINT VirtualObject::getVertexCount(const std::string &Name) {
	return m_vertexCount[Name];
}

UINT VirtualObject::getFaceCount(const std::string &Name) {
	return m_faceCount[Name];
}

bool VirtualObject::isEnabled() const {
	return m_enabled;
}

void VirtualObject::setEnabled(const bool enabled) {
	m_enabled = enabled;
}

std::string VirtualObject::getName() const {
	return m_ObjectName;
}

void VirtualObject::draw(const std::string &MeshName, const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
	m_Context->IASetInputLayout(getInputLayout());
    m_Context->VSSetShader(m_ShaderManager->getVS(VSName), NULL, 0);
    m_Context->GSSetShader(m_ShaderManager->getGS(GSName), NULL, 0);
    m_Context->PSSetShader(m_ShaderManager->getPS(PSName), NULL, 0);
    m_Context->CSSetShader(m_ShaderManager->getCS(CSName), NULL, 0);

	UINT stride = sizeof(Point3dVNT), offset = 0;
	m_Context->IASetVertexBuffers(0, 1, &m_VertexBuffer[m_ObjectName], &stride, &offset);
    m_Context->IASetIndexBuffer(m_IndexBuffer[MeshName], DXGI_FORMAT_R32_UINT, 0);
    m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Context->DrawIndexed(m_faceCount[MeshName] * 3, 0, 0);
}
