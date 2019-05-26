struct VS_IN
{
	float4 pos : POSITION0;
	float4 color : COLOR;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};


VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.pos = input.pos;
	output.color = input.color;
	return output;
}
