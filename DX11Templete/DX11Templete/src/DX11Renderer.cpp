#include "DX11Renderer.h"

DX11Renderer::DX11Renderer(HWND *hwnd, int width, int height)
: mWidth(width)
, mHeight(height)
, mRadian(0.0f)
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
	hr = CreateIndexBuffer();
	hr = CreateConstBuffer();
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
		mViewPort[0].TopLeftX = 0.0f;
		mViewPort[0].TopLeftY = 0.0f;
		mViewPort[0].Width = 640.0f;
		mViewPort[0].Height = 480.0f;
		mViewPort[0].MinDepth = 0.0f;
		mViewPort[0].MaxDepth = 1.0f;
		mDeviceContext->RSSetViewports(1, mViewPort);
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
	mDeviceContext->VSSetConstantBuffers(0, 1, &mConstBuffer);
	mDeviceContext->VSSetShader(mVertexShader, NULL, 0);
	mDeviceContext->PSSetShader(mPixelShader, NULL, 0);
	//プリミティブをレンダリング
	mDeviceContext->DrawIndexed(36, 0, 0);
}

// 画面更新
void DX11Renderer::update()
{
	HRESULT hr;
	mRadian += 0.0001f;
	XMMATRIX hRotate;
	hRotate = XMMatrixRotationY(mRadian);
	mWorldMatrix = XMMatrixIdentity();
	mWorldMatrix = XMMatrixMultiply(mWorldMatrix, hRotate);

	XMMATRIX mvpMatrix;
	XMMATRIX invMatrix;
	mvpMatrix = XMMatrixIdentity();
	mvpMatrix = XMMatrixMultiply(mvpMatrix, mWorldMatrix);
	mvpMatrix = XMMatrixMultiply(mvpMatrix, mViewMatrix);
	mvpMatrix = XMMatrixMultiply(mvpMatrix, mProjMatrix);
	invMatrix = XMMatrixInverse(NULL, mvpMatrix);
	
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(mWorldMatrix));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(mViewMatrix));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(mProjMatrix));
	XMStoreFloat4(&cb.light, mLight);
	XMStoreFloat4x4(&cb.invMatrix, XMMatrixTranspose(invMatrix));
	mDeviceContext->UpdateSubresource(mConstBuffer, 0, NULL, &cb, 0, 0);

	hr = mSwapChain->Present(0, 0);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to Screen Update");
	}
}

// 頂点バッファの作成
HRESULT DX11Renderer::CreateVertexBuffer()
{
	HRESULT hr;
	Vertex vertices[] =
	{
	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }  },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }  },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }  },
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }  },

	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f } },

	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }  },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }  },
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }  },
	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }  },

	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }  },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }  },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }  },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }  },

	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  },
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  },
	{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  },
	{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  },

	{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }  },
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }  },
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }  },
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }  },
	};

	D3D11_BUFFER_DESC idxBufferDesc;
	idxBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	idxBufferDesc.ByteWidth = sizeof(Vertex) * 24;
	idxBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	idxBufferDesc.CPUAccessFlags = 0;
	idxBufferDesc.MiscFlags = 0;
	idxBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	hr = mDevice->CreateBuffer(&idxBufferDesc, &InitData, &mVertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "頂点バッファの作成に失敗しました", 0, MB_OK);
	}
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return hr;
}

// インデックスバッファの作成
HRESULT DX11Renderer::CreateIndexBuffer()
{
	HRESULT hr;
	WORD g_IndexList[]{
	 0,  1,  2,     3,  2,  1,
	 4,  5,  6,     7,  6,  5,
	 8,  9, 10,    11, 10,  9,
	12, 13, 14,    15, 14, 13,
	16, 17, 18,    19, 18, 17,
	20, 21, 22,    23, 22, 21,
	};
	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = sizeof(WORD) * 36;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA irData;
	irData.pSysMem = g_IndexList;
	irData.SysMemPitch = 0;
	irData.SysMemSlicePitch = 0;

	hr = mDevice->CreateBuffer(&ibDesc, &irData, &mIndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "インデックスバッファの作成に失敗しました", 0, MB_OK);
	}
	mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	return hr;
}

// 定数バッファの作成
HRESULT DX11Renderer::CreateConstBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	hr = mDevice->CreateBuffer(&cbDesc, NULL, &mConstBuffer);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "定数バッファの作成に失敗しました", 0, MB_OK);
	}
	mWorldMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	XMVECTOR eye = XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);
	XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mViewMatrix = XMMatrixLookAtLH(eye, focus, up);

	float    fov = XMConvertToRadians(45.0f);
	float    aspect = mViewPort[0].Width / mViewPort[0].Height;
	float    nearZ = 0.1f;
	float    farZ = 100.0f;
	mProjMatrix = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

	XMMATRIX tmpMatrix;
	tmpMatrix = XMMatrixIdentity();
	tmpMatrix = XMMatrixMultiply(tmpMatrix, mWorldMatrix);
	tmpMatrix = XMMatrixMultiply(tmpMatrix, mViewMatrix);
	tmpMatrix = XMMatrixMultiply(tmpMatrix, mProjMatrix);
	XMMatrixInverse(NULL, tmpMatrix);

	mLight = XMVector3Normalize(XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));

	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(mWorldMatrix));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(mViewMatrix));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(mProjMatrix));
	XMStoreFloat4(&cb.light, mLight);
	XMStoreFloat4x4(&cb.invMatrix, XMMatrixTranspose(tmpMatrix));
	mDeviceContext->UpdateSubresource(mConstBuffer, 0, NULL, &cb, 0, 0);
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
	 { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

HRESULT DX11Renderer::LoadGeometryShader()
{
	ID3DBlob *pCompiledShader = NULL;
	//ブロブからジオメトリシェーダー作成
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "GS", "gs_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "ジオメトリシェーダー読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(mDevice->CreateGeometryShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mGeometryShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "ジオメトリシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);
	return S_OK;
}