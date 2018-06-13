#ifndef __textrenderer_h__
#define __textrenderer_h__

#ifdef _MSC_VER
# pragma once
#endif

#include <DXUT.h>
#include <SpriteFont.h>
#include "vec2.h"
#include "vec3.h"

class CTextRenderer
{
public:
	CTextRenderer() {}

	// In case of calling abort(), please make sure that fonts 
	// you are loading in Initialize() do exist!
	void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDevContext);

	void Release();

	void Begin();
	void End();

	//void DrawString(LPCSTR str, Vec2 pos);
	void DrawStringW(const std::wstring& str, const Vec2& pos, const Vec3& clr = Vec3(1.f, 1.f, 1.f) );
	void DrawStringW(LPCWSTR str, const Vec2& pos, const Vec3& clr = Vec3(1.f, 1.f, 1.f));

private:
	std::unique_ptr<DirectX::SpriteBatch>	m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont>	m_spriteFont;
};


#endif