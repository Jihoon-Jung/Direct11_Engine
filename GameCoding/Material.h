#pragma once
#include "ResourceBase.h"
#include "Shader.h"
#include "Texture.h"
#include "ConstantBufferType.h"

class Material : public ResourceBase
{
	using Super = ResourceBase;

public:
	Material();
	virtual ~Material();

	void SetVertexShader(shared_ptr<Shader> vertexShader) {
		_vertexShader = vertexShader;
	}
	void SetPixelShader(shared_ptr<Shader> pixelShader) {
		_pixelShader = pixelShader;
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
	void PushMaterialDesc();
	MaterialDesc GetMaterialDesc() { return _materialDesc; }
	void SetMaterialDesc(MaterialDesc materialDesc);
	shared_ptr<Shader> GetVertexShader() { return _vertexShader; }
	shared_ptr<Shader> GetPixelShader() { return _pixelShader; }
	shared_ptr<Texture> GetTexture() { return _texture; }
	shared_ptr<Texture> GetNormalMap() { return _normalMap; }
	shared_ptr<Texture> GetSpecularMap() { return _specularMap; }
	shared_ptr<Texture> GetDiffuseMap() { return _diffuseMap; }
	shared_ptr<Buffer> GetMaterialBuffer() { return _materialBuffer; }
private:
	shared_ptr<Shader> _vertexShader;
	shared_ptr<Shader> _pixelShader;
	shared_ptr<Texture> _texture;
	shared_ptr<Texture> _normalMap;
	shared_ptr<Texture> _diffuseMap;
	shared_ptr<Texture> _specularMap;
	shared_ptr<Buffer> _materialBuffer;
	MaterialDesc _materialDesc;
};

