#pragma once

// BLEND STATE

// BlendOperand determines how the final output pixel value will be calculated.
// The Source (s) value is the pixel that is generated from pixel shader.
// The Destination (d) values is the value currently in backbuffer.
// The Source and Destination values are first scaled by the source and destination
// blend factors before the blending is applied.

enum class BlendOperand
{
	Add,				// Add source and destination ( s + d )
	Subtract,			// Subtract source from destination ( d - s )
	ReverseSubtract,	// Subtract destination from source ( s - d )
	Min,				// The minimum of source and destination ( min( s, d ) )
	Max					// The maximum of source and destination ( max( s, d ) )
};

// BlendFactor is used to scale the pixel operand (either Source or Destination
// pixel value) before the blend operation is applied.
enum class BlendFactor
{
	Zero,				
	One,
	SrcColor,
	OneMinusSrcColor,	
};

class IBlendState
{
public:

};