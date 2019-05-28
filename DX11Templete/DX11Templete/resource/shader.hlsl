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

cbuffer ConstantBuffer
{
	float4x4 World;         //���[���h�ϊ��s��
	float4x4 View;          //�r���[�ϊ��s��
	float4x4 Projection;    //�����ˉe�ϊ��s��
}

//���_�V�F�[�_�[
VS_OUT VS( VS_IN input )
{
	VS_OUT output;

	output.pos = mul(input.pos, World);
	output.pos = mul(output.pos, View);
	output.pos = mul(output.pos, Projection);
	output.col = input.col;
	return output;
}

//�s�N�Z���V�F�[�_�[
float4 PS( PS_IN input ) : SV_Target
{
	return input.col;
}
