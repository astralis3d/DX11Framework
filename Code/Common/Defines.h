#ifndef __defines_h__
#define __defines_h__

#pragma once

#include "mathlib.h"

struct SVertexPosColor
{
	Vec3	pos;
	Vec3	color;
};

//----------------------------------------------------------------------
// Constant Buffers
//----------------------------------------------------------------------
struct SCBPerStaticObject
{
	Mat44	matWorld;
};

struct SCBPerFrame
{
	Vec4	time;	// x = time, y=time delta;  zw - unused atm
};

struct SCBPerView
{
	Mat44		mView;
	Mat44		mProj;
	Mat44		mProjInv;
	Mat44		mViewProj;
	Mat44		mViewProjInv;
	Mat44		mViewProjInvViewport;
	Mat44		mViewProjPrevFrame;

	Vec3		vCamPos;
	float		pad;

	Vec4		vViewport;

	float		zNear, zFar;
	Vec2		cameraNearFar;
	Vec2		depthScaleFactors;

	Vec2		pad01;
};

struct SCBScene
{
	Mat44		mViewProjShadow;

	Vec3		vLightDir;
	float		pad01;
};

struct SCBPostProcess
{
	SCBPostProcess()
		: m_VignetteTW3_Weights( 250.f / 255.f )
		, m_VignetteTW3_Color( 2.f / 255.f )
		, m_VignetteTW3_Opacity(0.65f)
	{}

	

	Vec4		m_VignetteTW3_Weights;
	Vec4		m_VignetteTW3_Color;
	float		m_VignetteTW3_Opacity;
	float pad[3];
};


#endif