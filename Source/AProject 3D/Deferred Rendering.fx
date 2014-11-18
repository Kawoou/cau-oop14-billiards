//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
struct Material
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float4 EmissiveColor;
	float4 SpecularColor;
	float Shininess;
};

struct Light
{
	int LightType;

	float3 LightDirection;	// world space direction
	float3 LightPosition;	// world space position

	float4 DiffuseColor;
	float4 SpecularColor;

	float SpotInnerCone;	// spot light inner cone (theta) angle
	float SpotOuterCone;	// spot light outer cone (phi) angle
	float Radius;           // applies to point and spot lights only

	float4x4 ViewProjMatrix;

	Material material;
};

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
float4x4 World;
float4x4 View;
float4x4 Projection;
float4x4 ITViewProjection;

float3 cameraPosition;

float2 halfPixel;

float lightIntensity = 1.0f;
float specularIntensity = 0.8f;
float specularPower = 0.5f;

Light g_lLight;
Material g_mMaterial;

//-----------------------------------------------------------------------------
// Textures
//-----------------------------------------------------------------------------
texture Texture1;		// Default: Diffuse
texture Texture2;		// Default: Normal, Alpha
texture Texture3;		// Default: Depth, Height
texture Texture4;		// Default: XYMap, Glow
texture Texture5;		// Default: Reflection
texture Texture6;		// Default: Specular
texture Texture7;		// Default: Light
texture Texture8;		// Default: Normal
texture Texture9;		// Default: SSAO

sampler samTexture1 = sampler_state
{
    Texture = (Texture1);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture2 = sampler_state
{
    Texture = (Texture2);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture3 = sampler_state
{
    Texture = (Texture3);
    MAGFILTER = POINT;
    MINFILTER = POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture4 = sampler_state
{
    Texture = (Texture4);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture5 = sampler_state
{
    Texture = (Texture5);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture6 = sampler_state
{
    Texture = (Texture6);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture7 = sampler_state
{
    Texture = (Texture7);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture8 = sampler_state
{
    Texture = (Texture8);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler samTexture9 = sampler_state
{
    Texture = (Texture9);
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

//-----------------------------------------------------------------------------
// Lighting
//-----------------------------------------------------------------------------
float4 DirectionLight (float3 position, float3 normal, float sIntensity, float sPower)
{
	//surface-to-light vector
	float3 lightVector = -normalize (g_lLight.LightDirection);

	//compute diffuse light
	float NdL = max (0, dot (normal, lightVector) );
	float3 diffuseLight = NdL * g_lLight.DiffuseColor.rgb * g_lLight.material.DiffuseColor.rgb;

	//reflexion vector
	float3 reflectionVector = normalize (reflect (-lightVector, normal) );

	//camera-to-surface vector
	float3 directionToCamera = normalize (cameraPosition - position);

	//compute specular light
	float specularLight = sIntensity * pow (saturate (dot (reflectionVector, directionToCamera) ), sPower);

	return lightIntensity * float4 (diffuseLight.rgb, specularLight);
}

float4 PointLight (float3 position, float3 normal, float sIntensity, float sPower)
{
	//surface-to-light vector
    float3 lightVector = g_lLight.LightPosition - position;

    //compute attenuation based on distance - linear attenuation
    float attenuation = saturate (1.0f - length (lightVector) / g_lLight.Radius); 

    //normalize light vector
    lightVector = normalize (lightVector); 

    //compute diffuse light
    float NdL = max (0, dot (normal, lightVector));
    float3 diffuseLight = NdL * g_lLight.DiffuseColor.rgb * g_lLight.material.DiffuseColor.rgb;

    //reflection vector
    float3 reflectionVector = normalize (reflect (-lightVector, normal) );
    //camera-to-surface vector
    float3 directionToCamera = normalize (cameraPosition - position);
    //compute specular light
    float specularLight = sIntensity * pow (saturate (dot (reflectionVector, directionToCamera) ), sPower);

    //take into account attenuation and lightIntensity.
    return attenuation * lightIntensity * float4 (diffuseLight.rgb, specularLight);
	//return float4 (lightVector, 0.0f);
}

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------
struct VertexShaderInput1
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderInput2
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
	float2 TexCoord1 : TEXCOORD0;
	float2 TexCoord2 : TEXCOORD1;
};

struct VertexShaderOutput1
{
	float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 WorldPos : TEXCOORD2;
};

struct VertexShaderOutput2
{
	float4 Position : POSITION0;
	float2 TexCoord1 : TEXCOORD0;
	float2 TexCoord2 : TEXCOORD1;
	float3 Normal : TEXCOORD2;
	float3 WorldPos : TEXCOORD3;
};

float gFarClip;

VertexShaderOutput1 Vertex_O2DDif(VertexShaderInput1 input)
{
	VertexShaderOutput1 output;

	float4 worldPosition = mul(float4 (input.Position, 1.0f), World);

	output.Position = float4(worldPosition.xy, 0, 1);
	output.TexCoord = input.TexCoord;                            //pass the texture coordinates further
	output.Normal = mul(input.Normal, World);                   //get normal into world space
	output.WorldPos = output.Position;

	return output;
}

VertexShaderOutput1 Vertex_Dif (VertexShaderInput1 input)
{
	VertexShaderOutput1 output;

	float4 worldPosition = mul(float4 (input.Position, 1.0f), World);
	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.TexCoord = input.TexCoord;                            //pass the texture coordinates further
	output.Normal = mul (input.Normal, World);                   //get normal into world space
	output.WorldPos = viewPosition.xyz;
	
	return output;
}

VertexShaderOutput2 Vertex_DifAlp (VertexShaderInput2 input)
{
	VertexShaderOutput2 output;

	float4 worldPosition = mul(float4 (input.Position, 1.0f), World);
	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.TexCoord1 = input.TexCoord1;                          //pass the texture coordinates further
	output.TexCoord2 = input.TexCoord2;
	output.Normal = mul (input.Normal, World);                   //get normal into world spacer
	output.WorldPos = viewPosition.xyz;
	
	return output;
}

/*
struct VertexShaderLightInput_Dir
{
	float3 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderLightOutput_Dir
{
	float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderLightInput_Point
{
	float3 Position : POSITION0;
};

struct VertexShaderLightOutput_Point
{
	float4 Position : POSITION0;
	float4 ScreenPosition : TEXCOORD0;
};

VertexShaderLightOutput_Dir Vertex_LightScene_Dir (VertexShaderLightInput_Dir input)
{
	VertexShaderLightOutput_Dir output;

	output.Position = float4 (input.Position, 1);
	//align texture coordinates
	output.TexCoord = input.TexCoord - halfPixel;

	return output;
}

VertexShaderLightOutput_Point Vertex_LightScene_Point (VertexShaderLightInput_Point input)
{
	VertexShaderLightOutput_Point output;

    //processing geometry coordinates
    float4 worldPosition = mul (float4 (input.Position, 1), World);
    float4 viewPosition = mul (worldPosition, View);
    output.Position = mul (viewPosition, Projection);
    output.ScreenPosition = output.Position;

    return output;
}
*/

struct VertexShaderLastInput
{
	float3 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderLastOutput
{
	float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

VertexShaderLastOutput Vertex_LastScene (VertexShaderLastInput input)
{
	VertexShaderLastOutput output;

	output.Position = float4 (input.Position, 1.0f);
	output.TexCoord = input.TexCoord - halfPixel;

	return output;
}

//-----------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------
struct PixelShaderOriOutput
{
	float4 Color : COLOR0;
	float4 Normal : COLOR1;
	float4 Depth : COLOR2;
	float4 XY: COLOR3;
};
struct PixelShaderOutput
{
	float4 Color : COLOR0;
	float4 Normal : COLOR1;
	float4 Depth : COLOR2;
	float4 XY: COLOR3;
};
struct PixelShaderO2DOutput
{
	float4 Color : COLOR0;
};

PixelShaderOutput Pixel_None (VertexShaderOutput1 input)
{
	PixelShaderOutput output;

	output.Color = g_mMaterial.DiffuseColor;									//output Color
	//output.Color.a = 1.0f;

	output.Normal.rgb = 0.5f * (normalize(input.Normal) + 1.0f);				//transform normal domain
	output.Normal.a = 1.0f;

	output.XY = float4(input.WorldPos.xy, g_mMaterial.Shininess, 1);					//output XY, SpecularPower
	output.Depth = float4(input.WorldPos.z, 0, 0, 1);							//output Depth

	return output;
}

PixelShaderO2DOutput Pixel_O2DNone(VertexShaderOutput1 input)
{
	PixelShaderO2DOutput output;

	output.Color = g_mMaterial.DiffuseColor;										//output Color

	return output;
}

PixelShaderO2DOutput Pixel_O2DDif(VertexShaderOutput1 input)
{
	PixelShaderO2DOutput output;

	output.Color = tex2D(samTexture1, input.TexCoord) * g_mMaterial.DiffuseColor;	//output Color

	return output;
}

PixelShaderOutput Pixel_Dif (VertexShaderOutput1 input)
{
	PixelShaderOutput output;

	output.Color = tex2D (samTexture1, input.TexCoord);							//output Color
	//output.Color.a = 1.0f;
	
	output.Normal.rgb = 0.5f * (normalize (input.Normal) + 1.0f);				//transform normal domain
	output.Normal.a = 1.0f;

	output.XY = float4(input.WorldPos.xy, specularPower, 1);					//output XY, SpecularPower
	output.Depth = float4(input.WorldPos.z, 0, 0, 1);							//output Depth
	
	return output;
}

PixelShaderOutput Pixel_DifAlp (VertexShaderOutput2 input)
{
	PixelShaderOutput output;

	output.Color = tex2D (samTexture1, input.TexCoord2);						//output Color
	output.Color.a = tex2D (samTexture2, input.TexCoord1);
	
	output.Normal.rgb = 0.5f * (normalize (input.Normal) + 1.0f);				//transform normal domain
	output.Normal.a = 1.0f;
	
	output.XY = float4(input.WorldPos.xy, 0, 1);			//output XY, SpecularPower
	output.Depth = float4(input.WorldPos.z, 0, 0, 1);							//output Depth
	
	return output;
}

struct PixelShaderLightInput_Dir
{
	float2 TexCoord : TEXCOORD0;
};
struct PixelShaderLightOutput_Dir
{
	float4 light : COLOR0;
	float4 specular : COLOR1;
};

PixelShaderLightOutput_Dir Pixel_LightScene_Dir(PixelShaderLightInput_Dir input)
{
	PixelShaderLightOutput_Dir output;

	float3 xySpacular = tex2D(samTexture4, input.TexCoord).xyz;
	float3 normal = (tex2D(samTexture2, input.TexCoord) - 0.5f) * 2.0f;
	float3 position;
	position.xy = xySpacular.xy;
	position.z = tex2D(samTexture3, input.TexCoord).x;
	//position = mul(position, ITViewProjection);

	if (position.z != 0.0f)
	{
		float3 lightDir = -normalize(g_lLight.LightDirection);
		float3 eyeVec = normalize(cameraPosition - position);
		float3 diffuseLight = max(0.1f, dot(lightDir, normal)) * g_lLight.DiffuseColor.rgb * g_lLight.material.DiffuseColor.rgb;
		float3 specularLight = specularIntensity * pow(max(0, dot(eyeVec, reflect(-lightDir, normal))), xySpacular.z) * g_lLight.SpecularColor.rgb * g_lLight.material.SpecularColor.rgb;

		output.light = float4(diffuseLight * lightIntensity, 1.0f);
		output.specular = float4(specularLight * lightIntensity, 1.0f);
	}
	else
	{
		output.light = float4(1.0f, 1.0f, 1.0f, 1.0f);
		output.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	return output;
}

PixelShaderLightOutput_Dir Pixel_LightScene_Point(PixelShaderLightInput_Dir input)
{
	
	PixelShaderLightOutput_Dir output;
	/*
	//obtain screen position
    input.ScreenPosition.xy /= input.ScreenPosition.w;

    //obtain textureCoordinates corresponding to the current pixel
    //the screen coordinates are in [-1,1]*[1,-1]
    //the texture coordinates need to be in [0,1]*[0,1]
    float2 texCoord = 0.5f * (float2 (input.ScreenPosition.x, -input.ScreenPosition.y) + 1.0f);
    //allign texels to pixels
    texCoord -= halfPixel;

    //get normal data from the normalMap
    float4 normalData = tex2D (samTexture2, texCoord);
    //tranform normal back into [-1,1] range
    float3 normal = 2.0f * normalData.xyz - 1.0f;

    //read depth
	float depthVal = tex2D (samTexture1, texCoord).r;

    //compute screen-space position
	float3 position;
	position.xy = xySpacular.xy;
	position.z = tex2D(samTexture3, input.TexCoord).x;
	position = mul(position, ITViewProjection);

	/// Light ///
	// Point Light
	//surface-to-light vector
	float3 lightVector = g_lLight.LightPosition - position;

		//compute attenuation based on distance - linear attenuation
		float attenuation = saturate(1.0f - length(lightVector) / g_lLight.Radius);

	//normalize light vector
	lightVector = normalize(lightVector);

	//compute diffuse light
	float NdL = max(0, dot(normal, lightVector));
	float3 diffuseLight = NdL * g_lLight.DiffuseColor.rgb * g_lLight.material.DiffuseColor.rgb;

		//reflection vector
		float3 reflectionVector = normalize(reflect(-lightVector, normal));
		//camera-to-surface vector
		float3 directionToCamera = normalize(cameraPosition - position);
		//compute specular light
		float specularLight = sIntensity * pow(saturate(dot(reflectionVector, directionToCamera)), sPower);

	//take into account attenuation and lightIntensity.
	return attenuation * lightIntensity * float4 (diffuseLight.rgb, specularLight);

    //output the two lights
	*/
    return output;
}

float4 Pixel_LastScene (VertexShaderLastOutput input) : COLOR0
{
	float3 diffuseColor;
	float4 light = tex2D (samTexture2, input.TexCoord);
	float3 specular = tex2D (samTexture3, input.TexCoord).rgb;

	diffuseColor = tex2D(samTexture1, input.TexCoord).rgb;

	float3 diffuseLight = light.rgb;
	float specularLight = light.a;

	return float4 ( (diffuseColor * diffuseLight + specular * specularLight), 1.0f);
}

//-----------------------------------------------------------------------------
// Techniques
// 备己规过 : "Obj_" + "Dif | Alp | Nor | Hei | Glo | Ref | Spe | Lig | Opa | SSA"
//-----------------------------------------------------------------------------
technique O2D_None
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;
		ZEnable = FALSE;
		StencilEnable = FALSE;
		VertexShader = compile vs_2_0 Vertex_O2DDif();
		PixelShader = compile ps_2_0 Pixel_O2DNone();
	}
}
technique O2D_Dif
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;
		ZEnable = FALSE;
		StencilEnable = FALSE;
		VertexShader = compile vs_2_0 Vertex_O2DDif();
		PixelShader = compile ps_2_0 Pixel_O2DDif();
	}
}

technique Obj_OriNone
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;
		VertexShader = compile vs_2_0 Vertex_Dif();
		PixelShader = compile ps_2_0 Pixel_None();
	}
}
technique Obj_None
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = ZERO;
		SrcBlend = ONE;
		VertexShader = compile vs_2_0 Vertex_Dif ();
		PixelShader = compile ps_2_0 Pixel_None ();
	}
}
technique Obj_OriDif
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;
		VertexShader = compile vs_2_0 Vertex_Dif ();
		PixelShader = compile ps_2_0 Pixel_Dif ();
	}
}

technique Obj_Dif
{
    pass Pass0
    {
		AlphaBlendEnable = TRUE;
		DestBlend = ZERO;
        SrcBlend = ONE;
        VertexShader = compile vs_2_0 Vertex_Dif ();
        PixelShader = compile ps_2_0 Pixel_Dif ();
    }
}

technique Obj_DifAlp
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
        SrcBlend = SRCALPHA;
		VertexShader = compile vs_2_0 Vertex_DifAlp ();
		PixelShader = compile ps_2_0 Pixel_DifAlp ();
	}
}

technique LightScene_Dir
{
	pass Pass0
	{
		CullMode = NONE;
		FillMode = SOLID;
		ZEnable = FALSE;
		StencilEnable = FALSE;
		AlphaBlendEnable = FALSE;
		AlphaTestEnable = FALSE;
		BlendOP = ADD;
		DestBlend = ONE;
        SrcBlend = ONE;
		ColorWriteEnable = RED | GREEN | BLUE;

		VertexShader = NULL;
		PixelShader = compile ps_3_0 Pixel_LightScene_Dir ();
	}
}
/*
technique LightScene_Point
{
	pass Pass0
	{
		AlphaBlendEnable = TRUE;
		BlendOP = ADD;
		DestBlend = ONE;
        SrcBlend = ONE;
		VertexShader = NULL;
		PixelShader = compile ps_3_0 Pixel_LightScene_Point ();
	}
}
*/

technique LastScene
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 Vertex_LastScene ();
		PixelShader = compile ps_3_0 Pixel_LastScene ();
	}
}