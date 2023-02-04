#pragma once

#include <d3d11.h>

namespace framework
{
	template <typename BufferType>
	class D3DConstantBuffer
	{
	public:
		D3DConstantBuffer() : bufferD3D11_( nullptr ) {}
		~D3DConstantBuffer() { Release(); }

		void Create( ID3D11Device* pDevice )
		{
			D3D11_BUFFER_DESC BufferDesc;

			BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			BufferDesc.ByteWidth = sizeof( BufferType );
			BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			BufferDesc.MiscFlags = 0;
			BufferDesc.StructureByteStride = 0;
			BufferDesc.Usage = D3D11_USAGE_DYNAMIC;

			HRESULT Result = pDevice->CreateBuffer( &BufferDesc, nullptr, &bufferD3D11_ );
			_ASSERT( SUCCEEDED( Result ) );
		}

		void Release()
		{
			SAFE_RELEASE( bufferD3D11_ );
		}

		void UpdateBuffer( ID3D11DeviceContext* pDevContext )
		{
			void* pData = Lock( pDevContext );
			memcpy( pData, &bufferData_, sizeof( bufferData_ ) );
			Unlock( pDevContext );
		}

		// Setters for buffers
		void SetVS( ID3D11DeviceContext* pDevContext, UINT slot )
		{
			ID3D11Buffer* pBuffers[1];
			pBuffers[0] = bufferD3D11_;

			pDevContext->VSSetConstantBuffers( slot, 1, pBuffers );
		}

		void SetPS( ID3D11DeviceContext* pDevContext, UINT slot )
		{
			ID3D11Buffer* pBuffers[1];
			pBuffers[0] = bufferD3D11_;

			pDevContext->PSSetConstantBuffers( slot, 1, pBuffers );
		}

		void SetGS( ID3D11DeviceContext* pDevContext, UINT slot )
		{
			ID3D11Buffer* pBuffers[1];
			pBuffers[0] = bufferD3D11_;

			pDevContext->GSSetConstantBuffers( slot, 1, pBuffers );
		}

		void SetDS( ID3D11DeviceContext* pDevContext, UINT slot )
		{
			ID3D11Buffer* pBuffers[1];
			pBuffers[0] = bufferD3D11_;

			pDevContext->DSSetConstantBuffers( slot, 1, pBuffers );
		}

		void SetHS( ID3D11DeviceContext* pDevContext, UINT slot )
		{
			ID3D11Buffer* pBuffers[1];
			pBuffers[0] = bufferD3D11_;

			pDevContext->HSSetConstantBuffers( slot, 1, pBuffers );
		}

		void SetCS( ID3D11DeviceContext* pDevContext, UINT slot )
		{
			ID3D11Buffer* pBuffers[1];
			pBuffers[0] = bufferD3D11_;

			pDevContext->CSSetConstantBuffers( slot, 1, pBuffers );
		}


		BufferType& GetBufferData() { return bufferData_; }
		ID3D11Buffer* GetBuffer() { return bufferD3D11_; }

	private:
		void* Lock( ID3D11DeviceContext* pDevContext )
		{
			D3D11_MAPPED_SUBRESOURCE Resource;
			HRESULT hr = pDevContext->Map( bufferD3D11_, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource );
			_ASSERT( SUCCEEDED( hr ) );

			return Resource.pData;
		}

		void Unlock( ID3D11DeviceContext* pDevContext )
		{
			pDevContext->Unmap( bufferD3D11_, 0 );
		}

	private:
		BufferType		bufferData_;
		ID3D11Buffer*	bufferD3D11_;
	};

}	// namespace framework