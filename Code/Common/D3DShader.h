#ifndef __common_d3dshader_h__
#define __common_d3dshader_h__

#ifdef _MSC_VER
# pragma once
#endif

// Forward decls
struct ID3D11DeviceContext;
struct ID3D11Device;
struct ID310DBlob;		// NOT ID3DBlob! - ID3DBlob is typedef of ID3D10Blob.


// Class for defining macros with definitions in shaders
// Example usage:
//
// CD3D11ShaderMacro macros;
// macros.AddMacro("USE_NORMAL_MAPS", "1");
// macros.AddMacro("MOTION_BLUR_TYPE", "3");
// macros.Finish();		<- don't forget about this!
class CD3D11ShaderMacro
{
public:
	void AddMacro(const char* name, const char* definition);
	void Finish();

	void Reset();

	const D3D_SHADER_MACRO* GetShaderMacrosPtr() const;
	const D3D_SHADER_MACRO& GetShaderMacro(int i) const;

private:
	std::vector<D3D_SHADER_MACRO>	m_ShaderMacro;
};


enum class eShaderType
{
	eShaderType_Unknown = -1,
	eShaderType_Pixel,
	eShaderType_Vertex,
	eShaderType_Geometry,
	eShaderType_Hull,
	eShaderType_Domain,
	eShaderType_Compute
};

class IShader
{
	typedef std::pair<std::string, std::string>		TStringsPair;
	typedef std::vector<TStringsPair>				TShaderDefinitions;

public:
	IShader();
	IShader(const eShaderType& type);
	virtual ~IShader();

	void Initialize(ID3D11Device* pDevice, LPCSTR pFilename, const char* pFunction, const CD3D11ShaderMacro* pDefines = nullptr, bool bHotReload = false);
	virtual void Release();

	void Bind(ID3D11DeviceContext* pDevContext);

	void* GetHandle() const;
	ID3D10Blob*	GetBlob() const;
	eShaderType GetShaderType() const;

	const std::string& GetShaderFilename() const { return m_filename; }
	const std::string& GetShaderEntrypoint() const { return m_entrypoint; }
	const TShaderDefinitions& GetDefines() const { return m_defines; }

	unsigned int GetTimestamp() const { return m_timestamp; }

private:
	void*			m_pHandle;
	eShaderType		m_shaderType;
	ID3D10Blob*		m_pBlob;

	std::string		m_filename;
	std::string		m_entrypoint;

	unsigned int	m_timestamp;

	TShaderDefinitions	m_defines;
};

#endif