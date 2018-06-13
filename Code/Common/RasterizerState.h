#pragma once

// Rasterizer state

#include "Viewport.h"
#include "Rect.h"
#include "GraphicsEnums.h"

#include <vector>

class IRasterizerState
{
public:
	virtual void SetFillMode( const FillMode& frontFace = FillMode::Solid, const FillMode& backFace = FillMode::Solid  ) = 0;
	virtual void GetFillMode( FillMode& frontFace, FillMode& backFace ) const = 0;

	virtual void SetCullMode( const CullMode& cullMode = CullMode::Back ) = 0;
	virtual CullMode GetCullMode() const = 0;

	virtual void SetFrontFacing( const FrontFace& frontFace = FrontFace::Clockwise ) = 0;
	virtual FrontFace GetFrontFacing() const = 0;

	/*
	// The depth bias allows to offset the computed depth value of the current pixel.
	// TODO: Research
	//
	// See:
	// Depth Bias, MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/cc308048(v=vs.85).aspx
	//
	*/
	virtual void SetDepthBias( float depthBias = 0.0f, float slopeBias = 0.0f, float biasClamp = 0.0f ) = 0;
	virtual void GetDepthBiast( float& depthBias, float& slopeBias, float& biasClamp ) const = 0;

	/*
	// Enables clipping based on Z-distance.
	// Default value is true.
	*/
	virtual void SetDepthClipEnabled( bool depthClipEnabled = true ) = 0;
	virtual bool GetDepthClipEnabled() const = 0;

	/*
	// Specify an array of viewports to bind to the rasterizer stage.
	// The helper function SetViewport can be used to set the viewport at index 0.
	*/
	virtual void SetViewport( const SViewport& viewport ) = 0;
	virtual void SetViewports( const std::vector<SViewport>& viewports ) = 0;
	virtual const std::vector<SViewport>& GetViewports() = 0;

	/*
	// Enable scissor-rectangle culling.
	// Default value is false.
	*/
	virtual void SetScissorEnabled( bool scissorEnable = false ) = 0;
	virtual bool GetScissorEnabled() const = 0;

	/*
	// If scissor rectangle culling is enabled, then use the rectangles passed
	// in the rects array to perform scissor culling. One scissor rectangle
	// is specified per viewport.
	// If the user sends more than GL_MAX_VIEWPORTS (for OpenGL) or D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE (DX)
	// for DirectX scissor rects then only the first GL_MAX_VIEWPORTS or ?? will be set.
	// Rectangle coordinates are in window coordinates.
	*/
	virtual void SetScissorRect( const SRect& rect ) = 0;
	virtual void SetScissorRects( const std::vector<SRect>& rects ) = 0;
	virtual const std::vector<SRect>& GetScissorRects() const = 0;
	
	/**
	* Specifies whether to use the quadrilateral or alpha line anti-aliasing
	* algorithm on multisample antialiasing (MSAA) render targets. Set to TRUE
	* to use the quadrilateral line anti-aliasing algorithm and to FALSE to use
	* the alpha line anti-aliasing algorithm.
	* See: https://msdn.microsoft.com/en-us/library/windows/desktop/hh404489(v=vs.85).aspx
	*/
	virtual void SetMultisampleEnabled( bool multisampleEnabled = false ) = 0;
	virtual bool GetMultisampleEnabled() const = 0;

	/**
	* Specifies whether to enable line antialiasing; only applies if doing line
	* drawing and MultisampleEnable is FALSE.
	* @see https://msdn.microsoft.com/en-us/library/windows/desktop/hh404489(v=vs.85).aspx
	* @see https://www.opengl.org/sdk/docs/man/html/glEnable.xhtml GL_LINE_SMOOTH
	*/
	virtual void SetAntialiasedLineEnable( bool antialiasedLineEnable ) = 0;
	virtual bool GetAntialiasedLineEnable() const = 0;

	/*
	// The sample count that is forced while UAV rendering or rasterizing.
	// Valid values are 0, 1, 2, 4, 8, and optionally 16. 0 indicates that the
	// sample count is not forced.
	// 
	// see https://msdn.microsoft.com/en-us/library/hh404489(v=vs.85).aspx
	*/
	virtual void SetForcedSampleCount( uint8_t forcedSampleCount = 0 ) = 0;
	virtual uint8_t GetForcedSampleCount() = 0;

	/**
	// Enable conservative rasterization mode.
	// Currently only supported in DirectX >=11.3 and 12.
	// 
	// See:
	// Conservative Rasterization: https://msdn.microsoft.com/en-us/library/windows/desktop/dn903791(v=vs.85).aspx
	*/
	virtual void SetConservativeRasterizationEnabled( bool conservativeRasterizationEnabled = false ) = 0;
	virtual bool GetConservativeRasterizationEnabled() const = 0;
};