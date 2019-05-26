#include "DX11Renderer.h"

DX11Renderer::DX11Renderer(HWND *hwnd, int width, int height)
: mHwnd(hwnd)
, mWidth(width)
, mHeight(height)
{
	
}

DX11Renderer::~DX11Renderer()
{
	if (mDeviceContext)
	{
		mDeviceContext->ClearState();
	}
	SafeRelease(mRenderTargetView);
	SafeRelease(mSwapChain);
	SafeRelease(mDeviceContext);
	SafeRelease(mDevice);
}

HRESULT DX11Renderer::Iinitialize()
{
	HRESULT hr;
	hr = CreateSwapChain();
	hr = CreateBackBuffer();
	hr = CreateDepthStencilBuffer();

	hr = CreateVertexBuffer();
	hr = LoadPixelShader();
	hr = LoadVertexShader();
	return hr;
}

// デバイスとスワップチェインの設定
HRESULT DX11Renderer::CreateSwapChain()
{
	HRESULT hr;
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = mWidth;
	sd.BufferDesc.Height = mHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = *mHwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#if defined(DEBUG) || defined(_DEBUG)
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT createDeviceFlags = 0;
#endif

	UINT feature_level_max = 3;
	D3D_FEATURE_LEVEL feature_level[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	// ハードウェアデバイスを作成
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		feature_level,
		feature_level_max,
		D3D11_SDK_VERSION,
		&sd,
		&mSwapChain,
		&mDevice,
		&mFeatureSupportLevel,
		&mDeviceContext
	);
	if (FAILED(hr)) // 失敗したらWARPドライバを作成
	{
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_WARP,
			NULL,
			createDeviceFlags,
			feature_level,
			feature_level_max,
			D3D11_SDK_VERSION,
			&sd,
			&mSwapChain,
			&mDevice,
			&mFeatureSupportLevel,
			&mDeviceContext
		);
		if (FAILED(hr)) // 失敗したらリファレンスドライバを作成
		{
			hr = D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_REFERENCE,
				NULL,
				createDeviceFlags,
				feature_level,
				feature_level_max,
				D3D11_SDK_VERSION,
				&sd,
				&mSwapChain,
				&mDevice,
				&mFeatureSupportLevel,
				&mDeviceContext
			);
			if (FAILED(hr))
			{
				MessageBox(*mHwnd, "デバイスとスワップチェインを作成に失敗しました", 0, MB_OK);
			}
		}
	}
	return hr;
}

// バックバッファの設定
HRESULT DX11Renderer::CreateBackBuffer()
{
	HRESULT hr;
	// スワップチェインから最初のバックバッファを取得
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBuffer);
	if (SUCCEEDED(hr))
	{
		// バックバッファの描画ターゲットビューを作成
		hr = mDevice->CreateRenderTargetView(mBackBuffer, NULL, &mRenderTargetView);
		SafeRelease(mBackBuffer);

		// 描画ターゲットビューを出力マージャーの描画ターゲットとして設定
		mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, NULL);

		// ラスタライザにビューポートを設定
		D3D11_VIEWPORT view_port[1];
		view_port[0].TopLeftX = 0.0f;
		view_port[0].TopLeftY = 0.0f;
		view_port[0].Width = 640.0f;
		view_port[0].Height = 480.0f;
		view_port[0].MinDepth = 0.0f;
		view_port[0].MaxDepth = 1.0f;
		mDeviceContext->RSSetViewports(1, view_port);
	}

	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "バックバッファを作成に失敗しました", 0, MB_OK);
	}
	return hr;
}

// 深度, ステンシル・バッファの設定
HRESULT DX11Renderer::CreateDepthStencilBuffer()
{
	HRESULT hr;

	// 深度, ステンシル・テクスチャの作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = mWidth;
	descDepth.Height = mHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = mDevice->CreateTexture2D(&descDepth, NULL, &mDepthStencil);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "深度, ステンシル・テクスチャの作成に失敗しました", 0, MB_OK);
	}

	// 深度, ステンシル・ビューの作成
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mDepthStencilView);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "深度, ステンシル・ビューの作成に失敗しました", 0, MB_OK);
	}

	// 深度, ステンシル・ビューの使用
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	return hr;
}

void DX11Renderer::render()
{
	clear();
	draw();
	update();
}

// クリア処理
void DX11Renderer::clear()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	// クリアする描画ターゲット
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	// 深度、ステンシル値のクリア
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

// 描画処理
void DX11Renderer::draw()
{
	//使用するシェーダーの登録
	mDeviceContext->VSSetShader(mVertexShader, NULL, 0);
	mDeviceContext->PSSetShader(mPixelShader, NULL, 0);
	//プリミティブをレンダリング
	mDeviceContext->Draw(3, 0);
}

// 画面更新
void DX11Renderer::update()
{
	HRESULT hr;
	hr = mSwapChain->Present(0, 0);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to Screen Update");
	}
}

// インデックスバッファの作成
HRESULT DX11Renderer::CreateVertexBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC idxBufferDesc;
	idxBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	idxBufferDesc.ByteWidth = sizeof(UINT) * 36;
	idxBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxBufferDesc.CPUAccessFlags = 0;
	idxBufferDesc.MiscFlags = 0;
	idxBufferDesc.StructureByteStride = 0;
	Vertex vertices[] =
	{
		{ { -0.5f,  0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	hr = mDevice->CreateBuffer(&idxBufferDesc, &InitData, &mVertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "インデックスバッファの作成に失敗しました", 0, MB_OK);
	}
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return hr;
}

HRESULT DX11Renderer::LoadVertexShader()
{
	ID3DBlob *pCompiledShader = NULL;
	//ブロブから頂点シェーダー作成
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "頂点シェーダー読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(mDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mVertexShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "頂点シェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	//頂点インプットレイアウトを定義 
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
	 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	 { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//頂点インプットレイアウトを作成
	if (FAILED(mDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &VertexLayout)))
	{
		SafeRelease(pCompiledShader);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);

	//頂点インプットレイアウトをセット
	mDeviceContext->IASetInputLayout(VertexLayout);
	return S_OK;
}

HRESULT DX11Renderer::LoadPixelShader()
{
	ID3DBlob *pCompiledShader = NULL;
	//ブロブからピクセルシェーダー作成
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "ピクセルシェーダー読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(mDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mPixelShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);
	return S_OK;
}