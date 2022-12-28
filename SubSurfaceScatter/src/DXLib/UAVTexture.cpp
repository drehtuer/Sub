#include "DXLib/UAVTexture.h"
#include <D3Dcompiler.h>

using namespace SubSurfaceScatter;

UAVTexture::UAVTexture()
	: m_Buffer(NULL),
	  m_SRV(NULL),
	  m_UAV(NULL),
	  m_width(0),
	  m_height(0),
	  m_bufferStructSize(0)
{

}

UAVTexture::~UAVTexture() {
	safe_delete(m_SRV);
	safe_delete(m_UAV);
	safe_delete(m_Buffer);
}

UINT UAVTexture::getWidth() const {
	return m_width;
}

UINT UAVTexture::getHeight() const {
	return m_height;
}

HRESULT UAVTexture::create(ID3D11Device *Device, const UINT width, const UINT height, const UINT bufferStructSize) {
	HRESULT hr = S_OK;

	m_bufferStructSize = bufferStructSize;

	resize(Device, width, height);
	

	return hr;
}

ID3D11ShaderResourceView *UAVTexture::getSRV() const {
	return m_SRV;
}

ID3D11UnorderedAccessView *UAVTexture::getUAV() const {
	return m_UAV;
}

ID3D11Buffer *UAVTexture::getBuffer() const {
	return m_Buffer;
}

HRESULT UAVTexture::resize(ID3D11Device *Device, const UINT width, const UINT height) {
	HRESULT hr = S_OK;

	if(m_width == width && m_height == height) {
		return hr; // all is well
	}
	// so we need to change something?
	m_width = width;
	m_height = height;

	D3D11_BUFFER_DESC BufferDesc;
	ZEROMEM(BufferDesc);
	BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = m_bufferStructSize;
	BufferDesc.ByteWidth = BufferDesc.StructureByteStride * m_width * m_height;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	
	chkHr(Device->CreateBuffer(&BufferDesc, NULL, &m_Buffer));

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZEROMEM(UAVDesc);
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Buffer.Flags = 0;
	UAVDesc.Buffer.NumElements = m_width * m_height;
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	
	chkHr(Device->CreateUnorderedAccessView(m_Buffer, &UAVDesc, &m_UAV));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZEROMEM(SRVDesc);
	SRVDesc.Buffer.ElementOffset = 0;
	SRVDesc.Buffer.ElementWidth = BufferDesc.StructureByteStride;
	SRVDesc.Buffer.FirstElement = 0;
	SRVDesc.Buffer.NumElements = m_width * m_height;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

	chkHr(Device->CreateShaderResourceView(m_Buffer, &SRVDesc, &m_SRV));

	return hr;
}
