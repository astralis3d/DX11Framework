#pragma once


/*
// FillMode determines how primitives are rendered.
//
// * Wireframe - primitives rendered as lines.
// * Solid - primitives rendered as solid objects.
//
// DX11 does not distinguish between frontface and backface fill modes.
// In this case, only frontface fill mode is considered.
// OpenGL allows you to set the front- and backface fill modes independently.
//
// See:
// D3D11_FILL_MODE: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476131(v=vs.85).aspx
// D3D12_FILL_MODE: https://msdn.microsoft.com/en-us/library/windows/desktop/dn770366(v=vs.85).aspx
*/
enum class FillMode
{
	Wireframe,
	Solid
};


/*
// CullMode controls which polygons facing the specified direction are not drawn.
// 
// * None - Always draw all polygons (no culling performed).
// * Front - Do not draw front-facing polygons.
// * Back - Do not draw back-facing polygons.
// * FrontAndBack - Both front and back-facing polygons are culled (OpenGL/Vulkan only).
//
// See:
// D3D11_CULL_MODE: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476108(v=vs.85).aspx
// D3D12_CULL_MODE: https://msdn.microsoft.com/en-us/library/windows/desktop/dn770354(v=vs.85).aspx
*/

enum class CullMode
{
	None,
	Front,
	Back,
	FrontAndBack		// OpenGL/Vulkan only
};

/*
// FrontFace determines which polygons are treated as "front facing" when applying clipping.
// The order refers to the order of vertices of a triangle when looking at the "front"
// of the primitive.
//
//	* Clockwise - Vertices of a triangle that are arranged in a clockwise winding order are considered front-facing.
//	Default value for DirectX and left-handed coordinate systems in general.
// *  CounterClockwise - Vertices of a triangle that are arranged in a counter-clockwise winding order are considered front-facing.
//	Default value for OpenGL and right-handed coordinate systems in general.
*/
enum class FrontFace
{
	Clockwise,
	CounterClockwise
};