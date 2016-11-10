#pragma once

#include "Render.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	class Image
	{
	public:
		Image(Render *render);

		bool Init(const wchar_t *name, float bitmapWidth, float bitmapHeight);
		void Draw(float positionX, float positionY);
		void Close();

	private:
		bool m_InitBuffers();
		void m_RenderBuffers();
		void m_SetShaderParameters(float x, float y);
		void m_RenderShader();

		Render *m_render;

		ID3D11Buffer *m_vertexBuffer;
		ID3D11Buffer *m_indexBuffer;
		ID3D11Buffer *m_constantBuffer;
		Shader *m_shader;

		float m_bitmapWidth, m_bitmapHeight;
		float m_previousPosX, m_previousPosY;
	};

	//------------------------------------------------------------------
}