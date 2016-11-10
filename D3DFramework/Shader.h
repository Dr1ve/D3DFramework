#pragma once

#include "Render.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	class Shader
	{
	public:
		Shader(Render *render);
		// данную функцию мы должны вызывать до вызова CreateShader
		void AddInputElementDesc(const char *SemanticName, DXGI_FORMAT format);

		bool CreateShader(wchar_t *namevs, wchar_t *nameps);
		bool LoadTexture(const wchar_t *name);

		void Draw();
		void Close();

	private:
		HRESULT m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

		Render *m_render;

		ID3D11VertexShader *m_vertexShader;
		ID3D11PixelShader *m_pixelShader;
		ID3D11InputLayout *m_layout;
		ID3D11ShaderResourceView *m_texture;
		ID3D11SamplerState *m_sampleState;

		D3D11_INPUT_ELEMENT_DESC *m_layoutformat;
		unsigned int m_numlayout;
	};

	//------------------------------------------------------------------
}