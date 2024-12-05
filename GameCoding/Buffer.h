#pragma once
enum class BufferType
{
	VERTEX_BUFFER,
	INDEX_BUFFER,
	CONSTANT_BUFFER
};

class Buffer
{
public:
	Buffer();
	~Buffer();

	template <typename T>
	void CreateBuffer(BufferType type, vector<T> source, bool cpuWrite = false, bool gpuWrite = false)
	{
		if (type == BufferType::VERTEX_BUFFER)
		{
			_stride = sizeof(T);
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			//desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.ByteWidth = (uint32)(sizeof(T) * static_cast<uint32>(source.size()));
			//desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

			if (cpuWrite == false && gpuWrite == false)
			{
				desc.Usage = D3D11_USAGE_IMMUTABLE; // CPU Read, GPU Read
			}
			else if (cpuWrite == true && gpuWrite == false)
			{
				desc.Usage = D3D11_USAGE_DYNAMIC; // CPU Write, GPU Read
				desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			}
			else if (cpuWrite == false && gpuWrite == true) // CPU Read, GPU Write
			{
				desc.Usage = D3D11_USAGE_DEFAULT;
			}
			else
			{
				desc.Usage = D3D11_USAGE_STAGING;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			}
			D3D11_SUBRESOURCE_DATA data;
			ZeroMemory(&data, sizeof(data));
			data.pSysMem = source.data();

			HRESULT hr = Graphics::GetInstance().GetDevice()->CreateBuffer(&desc, &data, _vertexBuffer.GetAddressOf());
			CHECK(hr);
		}
		else if (type == BufferType::INDEX_BUFFER)
		{
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.ByteWidth = (uint32)(sizeof(uint32) * source.size());

			D3D11_SUBRESOURCE_DATA data;
			ZeroMemory(&data, sizeof(data));
			data.pSysMem = source.data();

			HRESULT hr = Graphics::GetInstance().GetDevice()->CreateBuffer(&desc, &data, _indexBuffer.GetAddressOf());
			CHECK(hr);
		}
		//else if (type == BufferType::CONSTANT_BUFFER)
		//{
		//	D3D11_BUFFER_DESC desc;
		//	ZeroMemory(&desc, sizeof(desc));
		//	desc.Usage = D3D11_USAGE_DYNAMIC;
		//	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		//	desc.ByteWidth = sizeof(T);
		//	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//	HRESULT hr = Graphics::GetInstance().GetDevice()->CreateBuffer(&desc, nullptr, _constantBuffer.GetAddressOf());
		//}
	}
	template <typename T>
	void CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(T);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = Graphics::GetInstance().GetDevice()->CreateBuffer(&desc, nullptr, _constantBuffer.GetAddressOf());
		CHECK(hr);
	}
	template <typename T>
	void CopyData(const T& data)
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		ZeroMemory(&subResource, sizeof(subResource));

		Graphics::GetInstance().GetDeviceContext()->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		::memcpy(subResource.pData, &data, sizeof(data));
		Graphics::GetInstance().GetDeviceContext()->Unmap(_constantBuffer.Get(), 0);
	}

	uint32 GetStride() { return _stride; }
	ComPtr<ID3D11Buffer> GetVertexBuffer() { return _vertexBuffer; }
	ComPtr<ID3D11Buffer> GetIndexBuffer() { return _indexBuffer; }
	ComPtr<ID3D11Buffer> GetConstantBuffer() { return _constantBuffer; }
private:
	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;
	ComPtr<ID3D11Buffer> _constantBuffer = nullptr;
	uint32 _stride = 0;
	uint32 _offset = 0;
};

