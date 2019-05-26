#pragma once
#include "stddef.h"
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"d3dCompiler.lib")

struct Vertex
{
	float position[3];
	float color[4];
};

class DX11Renderer
{
private:
	int		mWidth;
	int		mHeight;
	HWND*    mHwnd;

	// インターフェース
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;
	IDXGISwapChain* mSwapChain;
	ID3D11RenderTargetView* mRenderTargetView;
	D3D_FEATURE_LEVEL mFeatureSupportLevel;
	HRESULT	CreateSwapChain();

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

	// 頂点バッファ
	ID3D11Buffer* mVertexBuffer;
	HRESULT CreateVertexBuffer();

	// 頂点シェーダー
	ID3D11InputLayout* VertexLayout;
	ID3D11VertexShader* mVertexShader;
	HRESULT LoadVertexShader();

	// ピクセルシェーダー
	ID3D11PixelShader* mPixelShader;
	HRESULT LoadPixelShader();

public:
	DX11Renderer(HWND *hwnd, int Width, int Height);
	~DX11Renderer();
	HRESULT Iinitialize();

	void render();
};
