#pragma once
#include "stddef.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"d3dCompiler.lib")

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

class DX11Renderer
{
private:
	int		mWidth;
	int		mHeight;
	HWND*    mHwnd;

	// 回転角
	float mRadian;

	XMMATRIX mWorldMatrix;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjMatrix;
	XMVECTOR mLight;

	// インターフェース
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;
	IDXGISwapChain* mSwapChain;
	ID3D11RenderTargetView* mRenderTargetView;
	D3D_FEATURE_LEVEL mFeatureSupportLevel;
	HRESULT	CreateSwapChain();

	// ビューポート
	D3D11_VIEWPORT mViewPort[1];

	// バックバッファ
	ID3D11Texture2D* mBackBuffer;
	HRESULT CreateBackBuffer();

	// 深度, ステンシル・バッファ
	ID3D11Texture2D* mDepthStencil;
	ID3D11DepthStencilView* mDepthStencilView;
	HRESULT CreateDepthStencilBuffer();

	//レンダー
	void clear();
	void draw();
	void update();

	// 頂点バッファ・インデックスバッファ
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	HRESULT CreateVertexBuffer();
	HRESULT CreateIndexBuffer();

	// 定数バッファ
	ID3D11Buffer* mConstBuffer;
	HRESULT CreateConstBuffer();

	// 頂点シェーダー
	ID3D11InputLayout* VertexLayout;
	ID3D11VertexShader* mVertexShader;
	HRESULT LoadVertexShader();

	// ピクセルシェーダー
	ID3D11PixelShader* mPixelShader;
	HRESULT LoadPixelShader();

	ID3D11GeometryShader* mGeometryShader;
	HRESULT LoadGeometryShader();

public:
	DX11Renderer(HWND *hwnd, int Width, int Height);
	~DX11Renderer();
	HRESULT Iinitialize();

	void render();
};
