#include "stdafx.h"
#include "StaticMesh.h"
#include <fstream>
#include "macros.h"
#include "Shader.h"
#include "Buffer.h"

using namespace D3D11Framework;
using namespace std;

//������ ��������� vertex
struct VERTEX
{
	XMFLOAT3 Pos;
	XMFLOAT3 Norm;
	XMFLOAT2 Tex;
};

struct ConstantBuffer
{
	XMMATRIX WVP;
};

//��������� ����� ��� �������� ������ � ��������
struct
{
	VERTEX vertices[buffermax];
	DWORD indices[buffermax];
	int verticesI;
	int indicesI;
} buffer;

StaticMesh::StaticMesh(Render *render)
{
	m_render = render;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_constantBuffer = nullptr;
	m_shader = nullptr;
}

bool StaticMesh::Init(wchar_t *fnameobj, wchar_t *fnametex)
{
	Identity();

	m_shader = new Shader(m_render);
	if (!m_shader)
		return false;

	m_shader->AddInputElementDesc("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	m_shader->AddInputElementDesc("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!m_shader->CreateShader(L"mesh.vs", L"mesh.ps"))
		return false;

	LoadMeshFromObj(fnameobj);
	if (buffer.verticesI == 0) return false;
	
	
	if (!m_shader->LoadTexture(fnametex)) return false;

	m_vertexBuffer = Buffer::CreateVertexBuffer(m_render->m_pd3dDevice, sizeof(VERTEX)*(buffer.verticesI + 1), false, buffer.vertices);
	if (!m_vertexBuffer) return false;

	m_indexCount = buffer.indicesI;
	m_indexBuffer = Buffer::CreateIndexBuffer(m_render->m_pd3dDevice, sizeof(DWORD) * m_indexCount, false, buffer.indices);
	if (!m_indexBuffer) return false;

	m_constantBuffer = Buffer::CreateConstantBuffer(m_render->m_pd3dDevice, sizeof(ConstantBuffer), false);
	if (!m_constantBuffer) return false;

	return true;
}

//������� ��� ���������� ������ � ��������
int AddVertex(int vertexI, VERTEX vertex)
{
	int res = -1;
	//����� ������������ �������
	for (int i = 0; i < buffer.verticesI; i++)
		if (memcmp(&buffer.vertices[i], &vertex, sizeof(VERTEX)) == 0) res = i;
	//����������
	if (res < 0)
	{
		buffer.vertices[buffer.verticesI++] = vertex;
		res = buffer.verticesI - 1;
	}
	return res;
}
void AddIndex(int index)
{
	buffer.indices[buffer.indicesI++] = index;
}

//������� ��������� ����� � ��������� ���������� � ���������� ������
void StaticMesh::LoadMeshFromObj(wchar_t *fname)
{
	//������� ������
	buffer.verticesI = 0;
	buffer.indicesI = 0;

	//�������� ���������� ���������
	XMFLOAT3 *Positions = (XMFLOAT3*)malloc(buffermax * sizeof(XMFLOAT3));
	XMFLOAT2 *TexCoords = (XMFLOAT2*)malloc(buffermax * sizeof(XMFLOAT2));
	XMFLOAT3 *Normals = (XMFLOAT3*)malloc(buffermax * sizeof(XMFLOAT3));

	//������� ��� ��������
	int PositionsI = 0;
	int TexCoordsI = 0;
	int NormalsI = 0;

	//���� ������ �� �����
	WCHAR strCommand[256] = { 0 };
	wifstream InFile(fname);

	if (!InFile) return;

	for (;;)
	{
		InFile >> strCommand;
		if (!InFile) break;

		if (0 == wcscmp(strCommand, L"#"))
		{
			//����������� ������ � ������������
		}
		else if (0 == wcscmp(strCommand, L"v"))
		{
			//���������� ������
			float x, y, z;
			InFile >> x >> y >> z;
			float c = 0.05f;//���������� ������� �� 95%))
			Positions[PositionsI++] = XMFLOAT3(x*c, y*c, z*c);
		}
		else if (0 == wcscmp(strCommand, L"vt"))
		{
			//���������� ����������
			float u, v;
			InFile >> u >> v;
			TexCoords[TexCoordsI++] = XMFLOAT2(u, -v);
		}
		else if (0 == wcscmp(strCommand, L"vn"))
		{
			//�������
			float x, y, z;
			InFile >> x >> y >> z;
			Normals[NormalsI++] = XMFLOAT3(x, y, z);
		}
		else if (0 == wcscmp(strCommand, L"f"))
		{
			//face
			UINT iPosition, iTexCoord, iNormal;
			VERTEX vertex;

			for (UINT iFace = 0; iFace < 3; iFace++)
			{
				ZeroMemory(&vertex, sizeof(VERTEX));

				//obj ������ ���������� ������� � ��������� �� 1
				InFile >> iPosition;
				vertex.Pos = Positions[iPosition - 1];

				if ('/' == InFile.peek())
				{
					InFile.ignore();

					if ('/' != InFile.peek())
					{
						//�������� ���������� ����������
						InFile >> iTexCoord;
						vertex.Tex = TexCoords[iTexCoord - 1];
					}

					if ('/' == InFile.peek())
					{
						InFile.ignore();
						//�������� �������
						InFile >> iNormal;
						vertex.Norm = Normals[iNormal - 1];
					}
				}
				//��������� ������� � ������
				int index = AddVertex(iPosition, vertex);
				AddIndex(index);
			}
		}
	}
	InFile.close();

	//������� ��������� ��������
	free(Positions);
	free(TexCoords);
	free(Normals);
}

void StaticMesh::Draw(CXMMATRIX viewmatrix)
{
	m_RenderBuffers();
	m_SetShaderParameters(viewmatrix);
	m_RenderShader();
}

void StaticMesh::m_RenderBuffers()
{
	//��������� ���������� ������
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	//��������� ���������� ������
	m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//��������� ��������� ������
	m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void StaticMesh::m_SetShaderParameters(CXMMATRIX viewmatrix)
{
	XMMATRIX WVP = m_objMatrix * viewmatrix * m_render->m_Projection;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(WVP);
	m_render->m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);
	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
}

void StaticMesh::m_RenderShader()
{
	m_shader->Draw();
	m_render->m_pImmediateContext->DrawIndexed(m_indexCount, 0, 0);
}

void StaticMesh::Close()
{
	_RELEASE(m_indexBuffer);
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_constantBuffer);
	_CLOSE(m_shader);
}

void StaticMesh::Translate(float x, float y, float z)
{
	m_objMatrix *= XMMatrixTranslation(x, y, z);
}

void StaticMesh::Rotate(float angle, float x, float y, float z)
{
	XMVECTOR v = XMVectorSet(x, y, z, 0.0f);
	m_objMatrix *= XMMatrixRotationAxis(v, angle);
}

void StaticMesh::Scale(float x, float y, float z)
{
	m_objMatrix *= XMMatrixScaling(x, y, z);
}

void StaticMesh::Identity()
{
	m_objMatrix = XMMatrixIdentity();
}