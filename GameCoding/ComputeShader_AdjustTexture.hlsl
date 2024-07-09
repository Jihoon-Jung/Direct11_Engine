Texture2DArray<float4> Input : register(t0);
RWTexture2DArray<float4> Output : register(u0);

[numthreads(32, 32, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
	float4 color = Input.Load(int4(id, 0));

	Output[id] = float4(1.0f - color.xyz, 1.0);
}