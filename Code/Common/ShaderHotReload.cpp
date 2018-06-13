#include "PCH.h"
#include "ShaderHotReload.h"
#include "D3DShaders.h"

CShaderHotReload* CShaderHotReload::Get()
{
	static CShaderHotReload inst;
	return &inst;
}

//------------------------------------------------------------------------
void CShaderHotReload::AddShader(IShader* pShader)
{
	m_shaders.push_back(pShader);
}

//------------------------------------------------------------------------
void CShaderHotReload::ReloadShaders(bool bReloadAll)
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	if (bReloadAll)
	{
		DebugPrintf("\n\n[SHADER COMPILER] %02d-%02d-%02d %02d:%02d:%02d:%03d\tRELOADING ALL SHADERS...\n",
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds
					);
	}
	else
	{
		DebugPrintf("\n\n[SHADER COMPILER] %02d-%02d-%02d %02d:%02d:%02d:%03d\tRELOADING CHANGED SHADERS...\n",
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds
					);
	}

	ID3D11Device* pDevice = DXUTGetD3D11Device();

	for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it)
	{
		IShader* p = *it;

		if (!p)
			continue;

		const std::string& entrypoint = p->GetShaderEntrypoint();
		const std::string& filename = p->GetShaderFilename();
		const auto& defines = p->GetDefines();
		uint32 oldTimestamp = p->GetTimestamp();
		

		if (bReloadAll == false)
		{
			// Last modified date
			HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				FILETIME lastWriteTime;
				::GetFileTime(hFile, nullptr, nullptr, &lastWriteTime);
				::CloseHandle(hFile);

				SYSTEMTIME fileTime;
				::FileTimeToSystemTime(&lastWriteTime, &fileTime);

				uint32 newTimestamp = CalculateShaderTimestamp(fileTime.wSecond, fileTime.wMilliseconds);
				//unsigned int newTimestamp  = static_cast<unsigned int>(fileTime.wSecond * fileTime.wMilliseconds + fileTime.wMilliseconds + 1);

				if (newTimestamp == oldTimestamp)
					continue;
			}
		}

		// Flush existing shader
		p->Release();

		
		GetLocalTime(&time);

		DebugPrintf("[SHADER COMPILER] %02d-%02d-%02d %02d:%02d:%02d:%03d\tRecompiling shader %s in %s...\n", 
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
					entrypoint.c_str(), filename.c_str());

		if (defines.empty())
		{
			p->Initialize(pDevice, filename.c_str(), entrypoint.c_str(), nullptr, true);
		}
		else
		{

			CD3D11ShaderMacro shaderMacros;
			for (unsigned int i = 0; i < defines.size(); ++i)
			{
				shaderMacros.AddMacro( defines[i].first.c_str(), defines[i].second.c_str() );
			}			
			shaderMacros.Finish();

			p->Initialize(pDevice, filename.c_str(), entrypoint.c_str(), &shaderMacros, true);
		}
	}
}

//------------------------------------------------------------------------
void CShaderHotReload::Flush()
{
	m_shaders.clear();
}

//------------------------------------------------------------------------
CShaderHotReload::CShaderHotReload()
{

}

CShaderHotReload::~CShaderHotReload()
{

}
