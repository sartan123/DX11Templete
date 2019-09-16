#include "BasicObject.h"
#include "DX11Renderer.h"

BasicObject::BasicObject()
{
	HRESULT hr;
	hr = CreateVertexBuffer();
	hr = CreateIndexBuffer();
	hr = CreateConstBuffer();
	hr = LoadPixelShader();
	hr = LoadVertexShader();
	if (FAILED(hr))
	{
		MessageBox(0, "BasicObject作成失敗", NULL, MB_OK);
		return;
	}
}

BasicObject::~BasicObject()
{
}

void BasicObject::update()
{
	SetWorldMatrix();

	SetViewMatrix();

	SetProjMatrix();

	SetInverseMatrix();

	SetLight();

	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(mWorldMatrix));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(mViewMatrix));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(mProjMatrix));
	XMStoreFloat4(&cb.light, mLight);
	XMStoreFloat4x4(&cb.invMatrix, XMMatrixTranspose(mInvMatrix));
	DX11Renderer::GetDeviceContext()->UpdateSubresource(mConstBuffer, 0, NULL, &cb, 0, 0);
}

void BasicObject::draw()
{
	//使用するシェーダーの登録
	DX11Renderer::GetDeviceContext()->VSSetConstantBuffers(0, 1, &mConstBuffer);
	DX11Renderer::GetDeviceContext()->VSSetShader(mVertexShader, NULL, 0);
	DX11Renderer::GetDeviceContext()->PSSetShader(mPixelShader, NULL, 0);
	//プリミティブをレンダリング
	DX11Renderer::GetDeviceContext()->DrawIndexed(36, 0, 0);
}

void BasicObject::SetWorldMatrix()
{
	mWorldMatrix = XMMatrixTranslation(mPosition.X, mPosition.Y, mPosition.X);
}

void BasicObject::SetViewMatrix()
{
	XMVECTOR eye = XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);
	XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mViewMatrix = XMMatrixLookAtLH(eye, focus, up);
}

void BasicObject::SetProjMatrix()
{
	float    fov = XMConvertToRadians(45.0f);
	float    aspect = 640.0f / 480.0f;
	float    nearZ = 0.1f;
	float    farZ = 100.0f;
	mProjMatrix = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
}

void BasicObject::SetInverseMatrix()
{
	XMMATRIX tmpMatrix;
	tmpMatrix = XMMatrixIdentity();
	tmpMatrix = XMMatrixMultiply(tmpMatrix, mWorldMatrix);
	tmpMatrix = XMMatrixMultiply(tmpMatrix, mViewMatrix);
	tmpMatrix = XMMatrixMultiply(tmpMatrix, mProjMatrix);
	mInvMatrix = XMMatrixInverse(NULL, tmpMatrix);
}

void BasicObject::SetLight()
{
	mLight = XMVector3Normalize(XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));
}

HRESULT BasicObject::CreateVertexBuffer()
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
	hr = DX11Renderer::GetDevice()->CreateBuffer(&idxBufferDesc, &InitData, &mVertexBuffer);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DX11Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	DX11Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return hr;
}

HRESULT BasicObject::CreateIndexBuffer()
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

	hr = DX11Renderer::GetDevice()->CreateBuffer(&ibDesc, &irData, &mIndexBuffer);
	DX11Renderer::GetDeviceContext()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	return hr;
}

HRESULT BasicObject::CreateConstBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	hr = DX11Renderer::GetDevice()->CreateBuffer(&cbDesc, NULL, &mConstBuffer);
	
	return hr;
}

HRESULT BasicObject::LoadVertexShader()
{
	ID3DBlob* pCompiledShader = NULL;
	//ブロブから頂点シェーダー作成
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "頂点シェーダー読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(DX11Renderer::GetDevice()->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mVertexShader)))
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
	if (FAILED(DX11Renderer::GetDevice()->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &VertexLayout)))
	{
		SafeRelease(pCompiledShader);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);

	//頂点インプットレイアウトをセット
	DX11Renderer::GetDeviceContext()->IASetInputLayout(VertexLayout);
	return S_OK;
}

HRESULT BasicObject::LoadPixelShader()
{
	ID3DBlob* pCompiledShader = NULL;
	//ブロブからピクセルシェーダー作成
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "ピクセルシェーダー読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(DX11Renderer::GetDevice()->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mPixelShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);
	return S_OK;
}

HRESULT BasicObject::LoadGeometryShader()
{
	ID3DBlob* pCompiledShader = NULL;
	//ブロブからジオメトリシェーダー作成
	if (FAILED(D3DCompileFromFile(L"resource\\shader.hlsl", NULL, NULL, "GS", "gs_5_0", 0, 0, &pCompiledShader, NULL)))
	{
		MessageBox(0, "ジオメトリシェーダー読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(DX11Renderer::GetDevice()->CreateGeometryShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &mGeometryShader)))
	{
		SafeRelease(pCompiledShader);
		MessageBox(0, "ジオメトリシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SafeRelease(pCompiledShader);
	return S_OK;
}
