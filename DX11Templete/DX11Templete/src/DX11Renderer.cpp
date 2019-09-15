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

// �f�o�C�X�ƃX���b�v�`�F�C���̐ݒ�
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

	// �n�[�h�E�F�A�f�o�C�X���쐬
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
	if (FAILED(hr)) // ���s������WARP�h���C�o���쐬
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
		if (FAILED(hr)) // ���s�����烊�t�@�����X�h���C�o���쐬
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
				MessageBox(*mHwnd, "�f�o�C�X�ƃX���b�v�`�F�C�����쐬�Ɏ��s���܂���", 0, MB_OK);
			}
		}
	}
	return hr;
}

// �o�b�N�o�b�t�@�̐ݒ�
HRESULT DX11Renderer::CreateBackBuffer()
{
	HRESULT hr;
	// �X���b�v�`�F�C������ŏ��̃o�b�N�o�b�t�@���擾
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBuffer);
	if (SUCCEEDED(hr))
	{
		// �o�b�N�o�b�t�@�̕`��^�[�Q�b�g�r���[���쐬
		hr = mDevice->CreateRenderTargetView(mBackBuffer, NULL, &mRenderTargetView);
		SafeRelease(mBackBuffer);

		// �`��^�[�Q�b�g�r���[���o�̓}�[�W���[�̕`��^�[�Q�b�g�Ƃ��Đݒ�
		mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, NULL);

		// ���X�^���C�U�Ƀr���[�|�[�g��ݒ�
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
		MessageBox(*mHwnd, "�o�b�N�o�b�t�@���쐬�Ɏ��s���܂���", 0, MB_OK);
	}
	return hr;
}

// �[�x, �X�e���V���E�o�b�t�@�̐ݒ�
HRESULT DX11Renderer::CreateDepthStencilBuffer()
{
	HRESULT hr;

	// �[�x, �X�e���V���E�e�N�X�`���̍쐬
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
		MessageBox(*mHwnd, "�[�x, �X�e���V���E�e�N�X�`���̍쐬�Ɏ��s���܂���", 0, MB_OK);
	}

	// �[�x, �X�e���V���E�r���[�̍쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mDepthStencilView);
	if (FAILED(hr))
	{
		MessageBox(*mHwnd, "�[�x, �X�e���V���E�r���[�̍쐬�Ɏ��s���܂���", 0, MB_OK);
	}

	// �[�x, �X�e���V���E�r���[�̎g�p
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	return hr;
}

void DX11Renderer::render()
{
	clear();
	draw();
	update();
}

// �N���A����
void DX11Renderer::clear()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	// �N���A����`��^�[�Q�b�g
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	// �[�x�A�X�e���V���l�̃N���A
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

// �`�揈��
void DX11Renderer::draw()
{
	//�g�p����V�F�[�_�[�̓o�^
	mDeviceContext->VSSetConstantBuffers(0, 1, &mConstBuffer);
	mDeviceContext->VSSetShader(mVertexShader, NULL, 0);
	mDeviceContext->PSSetShader(mPixelShader, NULL, 0);
	//�v���~�e�B�u�������_�����O
	mDeviceContext->DrawIndexed(36, 0, 0);
}

// ��ʍX�V
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

// ���_�o�b�t�@�̍쐬
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
		MessageBox(*mHwnd, "���_�o�b�t�@�̍쐬�Ɏ��s���܂���", 0, MB_OK);
	}
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return hr;
}

// �C���f�b�N�X�o�b�t�@�̍쐬
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
		MessageBox(*mHwnd, "�C���f�b�N�X�o�b�t�@�̍쐬�Ɏ��s���܂���", 0, MB_OK);
	}
	mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	return hr;
}

// �萔�o�b�t�@�̍쐬
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
		MessageBox(*mHwnd, "�萔�o�b�t�@�̍쐬�Ɏ��s���܂���", 0, MB_OK);
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
	//�u���u���璸�_�V�F�[�_�[�쐬
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "���_�V�F�[�_�[�ǂݍ��ݎ��s", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(mDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mVertexShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "���_�V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	//���_�C���v�b�g���C�A�E�g���` 
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
	 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	 { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	 { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//���_�C���v�b�g���C�A�E�g���쐬
	if (FAILED(mDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &VertexLayout)))
	{
		SafeRelease(pCompiledShader);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);

	//���_�C���v�b�g���C�A�E�g���Z�b�g
	mDeviceContext->IASetInputLayout(VertexLayout);
	return S_OK;
}

HRESULT DX11Renderer::LoadPixelShader()
{
	ID3DBlob *pCompiledShader = NULL;
	//�u���u����s�N�Z���V�F�[�_�[�쐬
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "�s�N�Z���V�F�[�_�[�ǂݍ��ݎ��s", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(mDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mPixelShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);
	return S_OK;
}

HRESULT DX11Renderer::LoadGeometryShader()
{
	ID3DBlob *pCompiledShader = NULL;
	//�u���u����W�I���g���V�F�[�_�[�쐬
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "GS", "gs_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "�W�I���g���V�F�[�_�[�ǂݍ��ݎ��s", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(mDevice->CreateGeometryShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mGeometryShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "�W�I���g���V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);
	return S_OK;
}