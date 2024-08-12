//Texture2DArray<float4> Input : register(t0);
//RWTexture2DArray<float4> Output : register(u0);
//
//[numthreads(32, 32, 1)]
//void CS(uint3 id : SV_DispatchThreadID)
//{
//	float4 color = Input.Load(int4(id, 0));
//
//	Output[id] = float4(color.x, 1.0, color.z, 1.0);
//}



cbuffer cbSettings
{
	float gWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 5;
};

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256
#define CacheSize (N + 2*gBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if (groupThreadID.x < gBlurRadius) // ���� 
	{
		// Clamp out of bound samples that occur at image borders.( �ؽ�ó�� ���κ��� �Ѿ�� �ʰ� �ϵ��� �ϱ����� �ϴ°�)
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= N - gBlurRadius) // ����
	{
		// Clamp out of bound samples that occur at image borders.( �ؽ�ó�� ���κ��� �Ѿ�� �ʰ� �ϵ��� �ϱ����� �ϴ°�)

		/*  ���� 1: ��踦 ���� �ʴ� ���
			���� �ȼ� �ε��� : 750
			��� : dispatchThreadID.x + gBlurRadius = 750 + 5 = 755
			min �Լ� ��� : 755 (��踦 ���� �����Ƿ� �״�� ���)
			�� ���, �ؽ�ó�� 755��° �ȼ��� gCache�� �����մϴ�.

			���� 2 : ��踦 �Ѵ� ���
			���� �ȼ� �ε��� : 798
			��� : dispatchThreadID.x + gBlurRadius = 798 + 5 = 803
			min �Լ� ��� : min(803, 799) = 799 (�ؽ�ó�� ��踦 �����Ƿ�, 799�� Ŭ����)
			�� ���, �ؽ�ó�� ������ �ȼ�(799��°)�� gCache�� �����մϴ�.
			�߿��� ����, gCache�� gInput �ؽ�ó�� ��ü ���� �����ϴ� ���� �ƴ϶�, �� ������ �׷��� ó���� �ȼ��� �� �ֺ��� �Ϻ� �ȼ��鸸�� �����Ѵٴ� ���Դϴ�.
			 �� �� ������ �׷츶�� 266��(N +2*gBlurRadius)�� �ȼ� �����͸� �����Ѵٴ°���.
		*/
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
		gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.x + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
	/*	GroupMemoryBarrierWithGroupSync�� �� ������ �׷� �������� �����ϴ� ����ȭ ����Դϴ�.
		��, ��ü ������ �׷��� �۾��� ���� ������ ��ٸ��� ���� �ƴ϶�,
		�ϳ��� ������ �׷� ���� ��� �����尡 Ư�� �������� ����ȭ�� ������ ��ٸ��� �ϴ� ������ �մϴ�.
	*/
	GroupMemoryBarrierWithGroupSync();

	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);

	[unroll]
	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;

		blurColor += gWeights[i + gBlurRadius] * gCache[k];
	}

	gOutput[dispatchThreadID.xy] =  blurColor;//gInput[dispatchThreadID.xy];
}