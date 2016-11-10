#include "stdafx.h"
#include "Image.h"
#include "Shader.h"
#include "macros.h"
#include "Log.h"
#include "Buffer.h"

using namespace D3D11Framework;

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};

struct ConstantBuffer
{
	XMMATRIX Ortho;
};

Image::Image(Render *render)
{
	m_render = render;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_constantBuffer = nullptr;
	m_shader = nullptr;
}

bool Image::Init(const wchar_t *textureFilename, float bitmapWidth, float bitmapHeight)
{
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;
	m_previousPosX = -1;
	m_previousPosY = -1;

	if (!m_InitBuffers())
		return false;

	m_shader = new Shader(m_render);
	if (!m_shader)
		return false;
	if (!m_shader->LoadTexture(textureFilename))
		return false;
	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!m_shader->CreateShader(L"image.vs", L"image.ps"))
		return false;

	return true;
}

bool Image::m_InitBuffers()
{
	Vertex vertices[4];

	signed int centreW = m_render->m_width / 2 * -1;
	signed int centreH = m_render->m_height / 2;
	float left = (float)centreW;
	float right = left + m_bitmapWidth;
	float top = (float)centreH;
	float bottom = top - m_bitmapHeight;

	vertices[0].pos = XMFLOAT3(left, top, 0.0f);
	vertices[0].tex = XMFLOAT2(0.0f, 0.0f);

	vertices[1].pos = XMFLOAT3(right, bottom, 0.0f);
	vertices[1].tex = XMFLOAT2(1.0f, 1.0f);

	vertices[2].pos = XMFLOAT3(left, bottom, 0.0f);
	vertices[2].tex = XMFLOAT2(0.0f, 1.0f);

	vertices[3].pos = XMFLOAT3(right, top, 0.0f);
	vertices[3].tex = XMFLOAT2(1.0f, 0.0f);

	unsigned long indices[6] =
	{
		0,1,2,
		0,3,1
	};

	m_vertexBuffer = Buffer::CreateVertexBuffer(m_render->m_pd3dDevice, sizeof(Vertex) * 4, false, &vertices);
	if (!m_vertexBuffer)
		return false;

	m_indexBuffer = Buffer::CreateIndexBuffer(m_render->m_pd3dDevice, sizeof(unsigned long) * 6, false, &indices);
	if (!m_indexBuffer)
		return false;

	m_constantBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice, sizeof(ConstantBuffer), false);
	if (!m_constantBuffer)
		return false;

	return true;
}

void Image::Draw(float positionX, float positionY)
{
	m_RenderBuffers();
	m_SetShaderParameters(positionX, positionY);
	m_RenderShader();
}

void Image::m_RenderBuffers()
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Image::m_SetShaderParameters(float x, float y)
{
	XMMATRIX objmatrix = XMMatrixTranslation(x, -y, 0.0f);
	XMMATRIX wvp = objmatrix*m_render->m_Ortho;
	ConstantBuffer cb;
	cb.Ortho = XMMatrixTranspose(wvp);
	m_render->m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);

	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
}

void Image::m_RenderShader()
{
	m_shader->Draw();
	m_render->m_pImmediateContext->DrawIndexed(6, 0, 0);
}

void Image::Close()
{
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_indexBuffer);
	_RELEASE(m_constantBuffer);
	_CLOSE(m_shader);
}