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
	if (groupThreadID.x < gBlurRadius) // 좌측 
	{
		// Clamp out of bound samples that occur at image borders.( 텍스처의 경계부분은 넘어가지 않게 하도록 하기위해 하는것)
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= N - gBlurRadius) // 우측
	{
		// Clamp out of bound samples that occur at image borders.( 텍스처의 경계부분은 넘어가지 않게 하도록 하기위해 하는것)

		/*  예제 1: 경계를 넘지 않는 경우
			현재 픽셀 인덱스 : 750
			계산 : dispatchThreadID.x + gBlurRadius = 750 + 5 = 755
			min 함수 결과 : 755 (경계를 넘지 않으므로 그대로 사용)
			이 경우, 텍스처의 755번째 픽셀을 gCache에 저장합니다.

			예제 2 : 경계를 넘는 경우
			현재 픽셀 인덱스 : 798
			계산 : dispatchThreadID.x + gBlurRadius = 798 + 5 = 803
			min 함수 결과 : min(803, 799) = 799 (텍스처의 경계를 넘으므로, 799로 클램핑)
			이 경우, 텍스처의 마지막 픽셀(799번째)을 gCache에 저장합니다.
			중요한 점은, gCache는 gInput 텍스처의 전체 행을 저장하는 것이 아니라, 각 스레드 그룹이 처리할 픽셀과 그 주변의 일부 픽셀들만을 저장한다는 것입니다.
			 즉 각 스레드 그룹마다 266개(N +2*gBlurRadius)의 픽셀 데이터를 저장한다는거임.
		*/
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
		gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.x + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
	/*	GroupMemoryBarrierWithGroupSync는 각 스레드 그룹 내에서만 동작하는 동기화 명령입니다.
		즉, 전체 스레드 그룹이 작업을 끝낼 때까지 기다리는 것이 아니라,
		하나의 스레드 그룹 내의 모든 스레드가 특정 지점에서 동기화될 때까지 기다리게 하는 역할을 합니다.
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