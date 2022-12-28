#ifndef SUBSURFACESCATTER_COMPUTESHADER_H
#define SUBSURFACESCATTER_COMPUTESHADER_H

#include "DXLib/Utilities.h"
#include "DXLib/Structs.h"
#include <D3D11.h>

namespace SubSurfaceScatter {

	class DLLE UAVTexture {
	public:
		UAVTexture();
		~UAVTexture();
		
		HRESULT create(ID3D11Device *Device, const UINT width, const UINT height, const UINT bufferStructSize);
		ID3D11ShaderResourceView *getSRV() const;
		ID3D11UnorderedAccessView *getUAV() const;
		ID3D11Buffer *getBuffer() const;
		HRESULT resize(ID3D11Device *Device, const UINT width, const UINT height);
		UINT getWidth() const;
		UINT getHeight() const;

	private:
		ID3D11Buffer *m_Buffer;
		ID3D11ShaderResourceView *m_SRV;
		ID3D11UnorderedAccessView *m_UAV;
		UINT m_width, m_height, m_bufferStructSize;
	};
}

#endif
