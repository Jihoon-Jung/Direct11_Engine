// Input structure for vertex shader
struct VS_INPUT {
    float4 position : POSITION;        // Vertex position
    float2 uv : TEXCOORD;              // Texture coordinates
    float3 normal : NORMAL;            // Normal vector
    float3 tangent : TANGENT;          // Tangent vector for normal mapping
    float4 blendIndices : BLENDINDICES; // Bone indices for skinning
    float4 blendWeights : BLENDWEIGHTS; // Bone weights for skinning
};

// Output structure for vertex shader and input for hull shader
struct HS_CONTROL_POINT_OUTPUT {
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHTS;
};

// Output structure for hull shader and input for domain shader
struct HS_CONSTANT_OUTPUT {
    float TessFactor[3] : SV_TessFactor; // Tessellation factors for the three edges of the triangle
    float InsideTessFactor : SV_InsideTessFactor; // Inside tessellation factor for the triangle
};

// Output structure for domain shader and input for pixel shader
struct DS_OUTPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

// Vertex Shader (VS)
HS_CONTROL_POINT_OUTPUT VS(VS_INPUT input) {
    HS_CONTROL_POINT_OUTPUT output;

    // Pass all data directly to Hull Shader
    output.position = input.position;
    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.blendIndices = input.blendIndices;
    output.blendWeights = input.blendWeights;

    return output;
}

// Hull Shader (HS) - Control point phase
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HS_CalculateTessellationFactors")]
HS_CONTROL_POINT_OUTPUT HS(InputPatch<HS_CONTROL_POINT_OUTPUT, 3> patch, uint id : SV_OutputControlPointID) {
    HS_CONTROL_POINT_OUTPUT output;

    // Pass control points to Domain Shader
    output.position = patch[id].position;
    output.uv = patch[id].uv;
    output.normal = patch[id].normal;
    output.tangent = patch[id].tangent;
    output.blendIndices = patch[id].blendIndices;
    output.blendWeights = patch[id].blendWeights;

    return output;
}

// Hull Shader (HS) - Patch constant phase
HS_CONSTANT_OUTPUT HS_CalculateTessellationFactors(InputPatch<HS_CONTROL_POINT_OUTPUT, 3> patch) {
    HS_CONSTANT_OUTPUT output;

    // Set Tessellation factors for each edge of the triangle
    output.TessFactor[0] = 4.0f; // Edge 1
    output.TessFactor[1] = 4.0f; // Edge 2
    output.TessFactor[2] = 4.0f; // Edge 3

    // Set Inside tessellation factor for the triangle
    output.InsideTessFactor = 4.0f;

    return output;
}

// Domain Shader (DS)
[domain("tri")]
DS_OUTPUT DS(HS_CONSTANT_OUTPUT patchConstants, const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> patch, float3 barycentricCoords : SV_DomainLocation) {
    DS_OUTPUT output;

    // Interpolate positions, normals, tangents, and UVs using barycentric coordinates
    output.position = barycentricCoords.x * patch[0].position +
        barycentricCoords.y * patch[1].position +
        barycentricCoords.z * patch[2].position;

    output.uv = barycentricCoords.x * patch[0].uv +
        barycentricCoords.y * patch[1].uv +
        barycentricCoords.z * patch[2].uv;

    output.normal = normalize(barycentricCoords.x * patch[0].normal +
        barycentricCoords.y * patch[1].normal +
        barycentricCoords.z * patch[2].normal);

    output.tangent = normalize(barycentricCoords.x * patch[0].tangent +
        barycentricCoords.y * patch[1].tangent +
        barycentricCoords.z * patch[2].tangent);

    return output;
}

// Pixel Shader (PS)
float4 PS(DS_OUTPUT input) : SV_Target{
    // Simple color based on UV coordinates and normal for shading
    float3 lighting = abs(input.normal); // Simple lighting effect based on normal
    return float4(input.uv, 0.0f, 1.0f) * float4(lighting, 1.0f);
}
