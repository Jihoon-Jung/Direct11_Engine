#pragma once
class InputLayout
{
public:
	InputLayout();
	~InputLayout();
	//void SetVertexBlob(ComPtr<ID3DBlob> blob) { _blob = blob; }
	ComPtr<ID3D11InputLayout> GetInputLayout() { return _inputLayout; }
	template <typename T>
	void CreateInputLayout(const vector<T>& desc, ComPtr<ID3DBlob> blob)
	{
		const int32 count = static_cast<int32>(desc.size());

		Graphics::GetInstance().GetDevice()->CreateInputLayout(desc.data(), count, blob->GetBufferPointer(), blob->GetBufferSize(), _inputLayout.GetAddressOf());

	}
private:
	//ComPtr<ID3DBlob> _blob;
	ComPtr<ID3D11InputLayout> _inputLayout;
};

