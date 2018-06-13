#include "PCH.h"
#include "TextRenderer.h"


void CTextRenderer::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDevContext)
{
	m_spriteBatch.reset( new DirectX::SpriteBatch(pDevContext) );
	//m_spriteFont.reset( new DirectX::SpriteFont(pDevice, L"..//!external//Fonts//consolas.spritefont") );
	m_spriteFont.reset(new DirectX::SpriteFont(pDevice, L"..//!external//Fonts//tahoma.spritefont"));
}

void CTextRenderer::Release()
{
	m_spriteBatch.reset();
	m_spriteFont.reset();
}

void CTextRenderer::Begin()
{
	m_spriteBatch->Begin();
}

void CTextRenderer::End()
{
	m_spriteBatch->End();
}

void CTextRenderer::DrawStringW(LPCWSTR str, const Vec2& pos, const Vec3& clr)
{
	DirectX::XMVECTORF32 Clr = {clr.x, clr.y, clr.z, 1.f};

	m_spriteFont->DrawString(m_spriteBatch.get(), str, DirectX::XMFLOAT2(pos.x, pos.y), Clr);
}

void CTextRenderer::DrawStringW(const std::wstring& str, const Vec2& pos, const Vec3& clr)
{
	DrawStringW(str.c_str(), pos, clr);
}
