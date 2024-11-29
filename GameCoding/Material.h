#pragma once
#include "ResourceBase.h"
#include "Shader.h"
#include "Texture.h"
#include "ConstantBufferType.h"

class GameObject;

class Material : public ResourceBase
{
	using Super = ResourceBase;

public:
	Material();
	virtual ~Material();
	void SetShader(shared_ptr<Shader> shader)
	{
		_shader = shader;
	}
	void SetTexture(shared_ptr<Texture> texture)
	{
		_texture = texture;
	}
	void SetNormalMap(shared_ptr<Texture> normal)
	{
		_normalMap = normal;
	}
	void SetDiffuseMap(shared_ptr<Texture> diffuse)
	{
		_diffuseMap = diffuse;
	}
	void SetSpecularMap(shared_ptr<Texture> specular)
	{
		_specularMap = specular;
	}

	ComPtr<ID3D11ShaderResourceView> AdjustTexture(shared_ptr<Shader> computeShader, shared_ptr<Texture> texture);
	ComPtr<ID3D11ShaderResourceView> GaussainBlur(shared_ptr<Shader> verticalBlurShader, shared_ptr<Shader> horizontalBlurShader, ComPtr<ID3D11ShaderResourceView> texture);

	void PushMaterialDesc();
	
	void SetMaterialDesc(MaterialDesc materialDesc);
	void CreateCubeMapTexture(shared_ptr<Texture> textureArray[6]);
	void CreateEnvironmentMapTexture(shared_ptr<GameObject> gameObject);
	void SetMaterialName(wstring name) { _materialName = name; }

	ComPtr<ID3D11ShaderResourceView> GetCubeMapSRV() { return _cubeMapSRV; }
	MaterialDesc GetMaterialDesc() { return _materialDesc; }
	shared_ptr<Shader> GetShader() { return _shader; }
	shared_ptr<Texture> GetTexture() { return _texture; }
	shared_ptr<Texture> GetNormalMap() { return _normalMap; }
	shared_ptr<Texture> GetSpecularMap() { return _specularMap; }
	shared_ptr<Texture> GetDiffuseMap() { return _diffuseMap; }
	shared_ptr<Buffer> GetMaterialBuffer() { return _materialBuffer; }
	wstring GetMaterialName() { return _materialName; }

private:
	shared_ptr<Shader> _shader;
	shared_ptr<Texture> _texture;
	shared_ptr<Texture> _normalMap;
	shared_ptr<Texture> _diffuseMap;
	shared_ptr<Texture> _specularMap;
	shared_ptr<Buffer> _materialBuffer;
	ComPtr<ID3D11ShaderResourceView> _cubeMapSRV;
	ComPtr<ID3D11ShaderResourceView> _environmentMapSRV;
	MaterialDesc _materialDesc;
	wstring _materialName = L"None";
};

