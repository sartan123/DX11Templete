#pragma once
#include "stddef.h"
#include <d3d11.h>
#include "BasicObject.h"

#pragma comment(lib, "d3d11.lib")

class DX11Renderer
{
private:
	int		mWidth;
	int		mHeight;
	HWND*    mHwnd;

	// インターフェース
	static ID3D11Device* mDevice;
	static ID3D11DeviceContext* mDeviceContext;
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

	std::vector<BasicObject*> objects;

	//レンダー
	void clear();
	void draw();
	void update();


public:
	DX11Renderer();
	~DX11Renderer();
	HRESULT Iinitialize(HWND *hwnd, int Width, int Height);

	static ID3D11Device* GetDevice() { return mDevice; }
	static ID3D11DeviceContext* GetDeviceContext() { return mDeviceContext; }

	void render();
};
