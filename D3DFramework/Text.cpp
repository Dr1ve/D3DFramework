#include "stdafx.h"
#include "Text.h"
#include "macros.h"
#include "Buffer.h"

using namespace D3D11Framework;

Text::Text(Render *render, BitmapFont *font)
{
	m_render = render;
	m_font = font;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_numindex = 0;
	m_numdrawindex = 0;
	m_numvertex = 0;
	m_static = true;
	m_size = 0;
}

bool Text::Init(const std::wstring &text, bool statictext, int charsize)
{
	m_static = statictext;
	m_size = charsize;
	if (!m_InitBuffers(text))
		return false;

	return true;
}

bool Text::m_InitBuffers(const std::wstring &text)
{
	if (!m_size || (m_size<text.size()))
		m_size = text.size();

	m_numvertex = m_size * 4;
	m_numindex = m_size * 6;
	m_numdrawindex = text.size() * 6;
	if (m_numdrawindex > m_numindex)
		m_numdrawindex = m_numindex;

	VertexFont *vertex = new VertexFont[m_numvertex];
	if (!vertex)
		return false;

	unsigned long *indices = new unsigned long[m_numindex];
	if (!indices)
		return false;

	m_font->BuildVertexArray(vertex, m_numvertex, text.c_str());

	for (int i = 0; i<m_numindex / 6; i++)
	{
		indices[i * 6 + 0] = i * 4 + 0;
		indices[i * 6 + 1] = i * 4 + 1;
		indices[i * 6 + 2] = i * 4 + 2;
		indices[i * 6 + 3] = i * 4 + 0;
		indices[i * 6 + 4] = i * 4 + 3;
		indices[i * 6 + 5] = i * 4 + 1;
	}

	m_vertexBuffer = Buffer::CreateVertexBuffer(m_render->m_pd3dDevice, sizeof(VertexFont)*m_numvertex, !m_static, vertex);
	if (!m_vertexBuffer)
		return false;

	m_indexBuffer = Buffer::CreateIndexBuffer(m_render->m_pd3dDevice, sizeof(unsigned long)*m_numindex, false, indices);
	if (!m_indexBuffer)
		return false;

	_DELETE_ARRAY(vertex);
	_DELETE_ARRAY(indices);

	return true;
}

bool Text::m_updatebuffer(const std::wstring &text)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = m_render->m_pImmediateContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	VertexFont *verticesPtr = (VertexFont*)mappedResource.pData;

	m_font->BuildVertexArray(verticesPtr, m_numvertex, text.c_str());

	m_render->m_pImmediateContext->Unmap(m_vertexBuffer, 0);

	return true;
}

void Text::Draw(float r, float g, float b, float x, float y)
{
	m_RenderBuffers();
	m_font->Draw(m_numdrawindex, r, g, b, x, y);
}

void Text::m_RenderBuffers()
{
	unsigned int stride = sizeof(VertexFont);
	unsigned int offset = 0;
	m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Text::Close()
{
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_indexBuffer);
}

bool Text::SetText(const std::wstring &text)
{
	// статический текст нельзя изменить
	if (m_static)
		return false;
	// если текст меньше чем число индексов, мы не должны рисовать ненужные
	m_numdrawindex = text.size() * 6;
	return m_updatebuffer(text);
}