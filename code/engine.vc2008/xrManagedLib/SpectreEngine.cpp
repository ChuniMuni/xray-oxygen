#include "stdafx.h"
#include "SpectreEngine.h"
#include "API/Log.h"
#include "API/ClassRegistrator.h"
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Reflection;

SpectreEngine::SpectreEngine()
{
	ModInstances = gcnew List<Object^>();
	ScriptCompiler = gcnew xrScriptCompiler();
}

SpectreEngine^ SpectreEngine::Instance()
{
	if (!SpectreEngine::gInstance)
	{
		SpectreEngine::gInstance = gcnew SpectreEngine();
	}

	return SpectreEngine::gInstance;
}

void SpectreEngine::xrCoreInit(String^ appName)
{
	Debug._initialize();

	msclr::interop::marshal_context^ marshal;
	Core._initialize(marshal->marshal_as<const char*>(appName), nullptr, false, "fs.ltx");
}

void SpectreEngine::xrEngineInit()
{
	HMODULE hManagedEngineLib = GetModuleHandle("xrManagedEngineLib");
	if (hManagedEngineLib == NULL)
	{
		String^ BinFileName = Assembly::GetCallingAssembly()->Location;
		String^ BinDirectory = System::IO::Path::GetDirectoryName(BinFileName);
		String^ LoadFilename = BinDirectory + "\\xrManagedEngineLib.dll";

		Assembly::LoadFrom(LoadFilename);
	}
	hManagedEngineLib = GetModuleHandle("xrManagedEngineLib");
	R_ASSERT(hManagedEngineLib);

	FARPROC pXrEngineInit = GetProcAddress(hManagedEngineLib, "xrEngineInit");
	R_ASSERT(pXrEngineInit);

	pXrEngineInit();
}

void SpectreEngine::xrRenderInit()
{
	HMODULE hManagedRenderLib = GetModuleHandle("xrManagedRenderLib");
	if (hManagedRenderLib == NULL)
	{
		String^ BinFileName = Assembly::GetCallingAssembly()->Location;
		String^ BinDirectory = System::IO::Path::GetDirectoryName(BinFileName);
		String^ LoadFilename = BinDirectory + "\\xrManagedRenderLib.dll";

		Assembly::LoadFrom(LoadFilename);
	}
	hManagedRenderLib = GetModuleHandle("xrManagedRenderLib");
	R_ASSERT(hManagedRenderLib);

	FARPROC pXrRenderInit = GetProcAddress(hManagedRenderLib, "xrRenderInit");
	R_ASSERT(pXrRenderInit);

	pXrRenderInit();
}

void SpectreEngine::GameLibInit()
{
	HMODULE hManagedGameLib = GetModuleHandle("xrManagedGameLib");
	if (hManagedGameLib == NULL)
	{
		String^ BinFileName = Assembly::GetCallingAssembly()->Location;
		String^ BinDirectory = System::IO::Path::GetDirectoryName(BinFileName);
		String^ LoadFilename = BinDirectory + "\\xrManagedGameLib.dll";

		Assembly^ GameAssembly = Assembly::LoadFrom(LoadFilename);

		// Invoke class registrator
		array<Type^>^ GameTypes = GameAssembly->GetTypes();

		for each (Type^ var in GameTypes)
		{
			array<::System::Object^>^ ClassRegistratorAttribs = var->GetCustomAttributes(XRay::ClassRegistratorDecorator::typeid, true);
			if (ClassRegistratorAttribs->Length > 0)
			{
				MethodInfo^ RegisterTypesMethod = var->GetMethod("RegisterTypes", BindingFlags::Public | BindingFlags::Static);
				if (RegisterTypesMethod != nullptr)
				{
					RegisterTypesMethod->Invoke(nullptr, nullptr);
				}
				else
				{
					XRay::Log::Warning("xrManagedGameLib has GameClassRegistrator, but without RegisterTypes method, which is unusual");
				}
			}
		}
	}
	hManagedGameLib = GetModuleHandle("xrManagedGameLib");
	R_ASSERT2(hManagedGameLib, "Can't load xrManagedGameLib.dll");
}

void SpectreEngine::CompileScripts()
{
	if (!ScriptCompiler->CompileScripts())
	{
		XRay::Log::Error("[SPECTRE]: Can't compile scripts");
		return;
	}

	Assembly^ ScriptAssembly = ScriptCompiler->GetAssembly();

	// Create all ModInstance's and invoke 'OnLoad'
	array<Type^>^ Types = ScriptAssembly->GetTypes();

	for (int i = 0; i < Types->Length; i++)
	{
		Type^ type = Types[i];
		
		if (type->IsClass && type->BaseType != nullptr && type->BaseType->Name->Contains("ModInstance"))
		{
			Object^ ModInstanceObj = System::Activator::CreateInstance(type);
			if (ModInstanceObj)
			{
				ModInstances->Add(ModInstanceObj);
				MethodInfo^ OnLoadInfo = type->GetMethod("OnLoad");
				if (OnLoadInfo != nullptr)
				{
					OnLoadInfo->Invoke(ModInstanceObj, nullptr);
				}
			}
		}
	}
}

// Bridge interface for unmanaged libs
struct SpectreCoreServer : ISpectreCoreServer
{
	virtual void CompileScripts() override
	{
		SpectreEngine::Instance()->CompileScripts();
	}

	virtual void LoadGameLib() override
	{
		SpectreEngine::Instance()->GameLibInit();
	}
};

SpectreCoreServer SpectreAPI;

MANAGED_API ISpectreCoreServer* GetCoreInterface()
{
	return &SpectreAPI;
}