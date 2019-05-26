struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

//���_�V�F�[�_�[
VS_OUT VS( VS_IN input )
{
	VS_OUT output;
	output.pos = input.pos;
	output.col = input.col;
    return output;
}

//�s�N�Z���V�F�[�_�[
float4 PS( PS_IN input ) : SV_Target
{
	return input.col;
}
