#pragma once
#include "stddef.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"d3dCompiler.lib")

class BasicObject
{
	struct Vertex
	{
		float position[3];
		float color[4];
		float normal[3];
	};

	struct ConstantBuffer {
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4   light;
		XMFLOAT4X4 invMatrix;
	};

	struct Position
	{
		float X;
		float Y;
		float Z;
	};

public:
	BasicObject();
	~BasicObject();
	void update();
	void draw();
	void SetPosition(float x, float y, float z) { mPosition.X = x; mPosition.Y = y; mPosition.Z = z; }
	void SetPositionX(float x) { mPosition.X = x; }
	void SetPositionY(float y) { mPosition.Y = y; }
	void SetPositionZ(float z) { mPosition.Z = z; }
	void SetCamera(XMMATRIX view_matrix) { mViewMatrix = view_matrix; }
	
protected:
	XMMATRIX mWorldMatrix;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjMatrix;
	XMMATRIX mInvMatrix;
	XMVECTOR mLight;

	Position mPosition;


	// シェーダー
	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;
	ID3D11GeometryShader* mGeometryShader;

	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	ID3D11Buffer* mConstBuffer;
	ID3D11InputLayout* VertexLayout;

	virtual HRESULT CreateVertexBuffer();
	virtual HRESULT CreateIndexBuffer();
	virtual HRESULT LoadVertexShader();
	virtual HRESULT CreateConstBuffer();
	virtual HRESULT LoadPixelShader();
	virtual HRESULT LoadGeometryShader();

private:
	void SetWorldMatrix();
	void SetViewMatrix();
	void SetProjMatrix();
	void SetLight();
	void SetInverseMatrix();
};

