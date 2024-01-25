#include "Materials.h"

#include <filesystem>
#include <fstream>

#include "../../Menu/ConfigManager/ConfigManager.h"

IMaterial* CMaterials::CreateNRef(char const* szName, void* pKV)
{
	IMaterial* returnMaterial = I::MaterialSystem->Create(szName, pKV);
	returnMaterial->IncrementReferenceCount();
	vMatList.push_back(returnMaterial);
	return returnMaterial;
}

void CMaterials::RemoveMaterials()
{
	// placeholder just in case
	ReloadMaterials();
}

void CMaterials::ReloadMaterials()
{
	// clear existing materials

	for (auto& mat : vChamMaterials)
	{
		if (!mat.pMaterial)
			continue;

		mat.pMaterial->DecrementReferenceCount();
		mat.pMaterial->DeleteIfUnreferenced();
		mat.pMaterial = nullptr;
	}
	for (auto& mat : vGlowMaterials)
	{
		if (!mat.pMaterial)
			continue;

		mat.pMaterial->DecrementReferenceCount();
		mat.pMaterial->DeleteIfUnreferenced();
		mat.pMaterial = nullptr;
	}
	vChamMaterials.clear();
	vGlowMaterials.clear();



	// cham materials

	// default materials
	{	// hacky
		CMaterial mat = { "None" };

		mat.sVMT =	"\"UnlitGeneric\"";
		mat.sVMT +=	"\n{";
		mat.sVMT +=	"\n	$basetexture \"vgui/black\"";
		mat.sVMT +=	"\n	$additive \"1\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "Flat" };

		mat.sVMT =	"\"UnlitGeneric\"";
		mat.sVMT +=	"\n{";
		mat.sVMT +=	"\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "Shaded" };

		mat.sVMT =	"\"VertexLitGeneric\"";
		mat.sVMT +=	"\n{";
		mat.sVMT +=	"\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "Shiny" };

		mat.sVMT =	"\"VertexLitGeneric\"";
		mat.sVMT +=	"\n{";
		mat.sVMT +=	"\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT +=	"\n	$envmap \"cubemaps/cubemap_sheen001\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "Wireframe" };

		mat.sVMT =	"\"UnlitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"vgui/white_additive\"";
		mat.sVMT += "\n	$wireframe \"1\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	{	// make smoother
		CMaterial mat = { "Fresnel" };

		mat.sVMT =	"\"VertexLitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"models/player/shared/ice_player\"";
		mat.sVMT += "\n	$bumpmap \"models/player/shared/shared_normal\"";
		mat.sVMT += "\n	$additive \"1\"";
		mat.sVMT += "\n	$phong \"1\"";
		mat.sVMT += "\n	$phongfresnelranges \"[0 0.025 0.05]\"";
		mat.sVMT += "\n	$envmap \"skybox/sky_dustbowl_01\"";
		mat.sVMT += "\n	$envmapfresnel \"1\"";
		mat.sVMT += "\n	$selfillum \"1\"";
		mat.sVMT += "\n	$selfillumtint \"[0 0 0]\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "Tint" };

		mat.sVMT = "\"VertexLitGeneric\"";
		mat.sVMT += "\n{";
		mat.sVMT += "\n	$basetexture \"models/player/shared/ice_player\"";
		mat.sVMT += "\n	$bumpmap \"models/player/shared/shared_normal\"";
		mat.sVMT += "\n	$additive \"1\"";
		mat.sVMT += "\n	$phong \"1\"";
		mat.sVMT += "\n	$phongfresnelranges \"[0 0.001 0]\"";
		mat.sVMT += "\n	$envmap \"skybox/sky_dustbowl_01\"";
		mat.sVMT += "\n	$envmapfresnel \"1\"";
		mat.sVMT += "\n	$selfillum \"1\"";
		mat.sVMT += "\n	$selfillumtint \"[0 0 0]\"";
		mat.sVMT += "\n}";

		mat.bLocked = true;
		vChamMaterials.push_back(mat);
	}
	// user materials
	for (const auto& entry : std::filesystem::directory_iterator(MaterialFolder))
	{
		// Ignore all non-Material files
		if (!entry.is_regular_file() || entry.path().extension() != std::string(".vmt"))
			continue;

		std::ifstream inStream(entry.path());
		if (!inStream.good())
			continue;

		std::string sName = entry.path().filename().string();
		sName.erase(sName.end() - 4, sName.end());
		const std::string sVMT((std::istreambuf_iterator(inStream)), std::istreambuf_iterator<char>());

		CMaterial mat = { sName };
		mat.sVMT = sVMT;

		vChamMaterials.push_back(mat);
	}
	// create materials
	for (auto& mat : vChamMaterials)
	{
		const auto kv = new KeyValues(mat.sName.c_str());
		g_KeyValUtils.LoadFromBuffer(kv, mat.sName.c_str(), mat.sVMT.c_str());
		mat.pMaterial = CreateNRef(std::format("m_pMat%s", mat.sName).c_str(), kv);
	}

	

	// glow materials

	{
		CMaterial mat = { "GlowColor" };

		mat.pMaterial = I::MaterialSystem->Find("dev/glow_color", TEXTURE_GROUP_OTHER);
		mat.pMaterial->IncrementReferenceCount();

		vGlowMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "BlurX" };

		KeyValues* m_pMatBlurXKV = new KeyValues("BlurFilterX");
		m_pMatBlurXKV->SetString("$basetexture", "glow_buffer_1");
		mat.pMaterial = CreateNRef("m_pMatBlurX", m_pMatBlurXKV);

		vGlowMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "BlurY" };

		KeyValues* m_pMatBlurYKV = new KeyValues("BlurFilterY");
		m_pMatBlurYKV->SetString("$basetexture", "glow_buffer_2");
		mat.pMaterial = CreateNRef("m_pMatBlurY", m_pMatBlurYKV);

		vGlowMaterials.push_back(mat);
	}
	{
		CMaterial mat = { "HaloAddToScreen" };

		KeyValues* m_pMatHaloAddToScreenKV = new KeyValues("UnlitGeneric");
		m_pMatHaloAddToScreenKV->SetString("$basetexture", "glow_buffer_1");
		m_pMatHaloAddToScreenKV->SetString("$additive", "1");
		mat.pMaterial = CreateNRef("m_pMatHaloAddToScreen", m_pMatHaloAddToScreenKV);

		vGlowMaterials.push_back(mat);
	}
}

void CMaterials::SetColor(IMaterial* material, Color_t color)
{
	if (material)
	{
		if (auto $phongtint = material->FindVar("$phongtint", nullptr, false))
			$phongtint->SetVecValue( Color::TOFLOAT(color.r), Color::TOFLOAT(color.g), Color::TOFLOAT(color.b) );
		if (auto $envmaptint = material->FindVar("$envmaptint", nullptr, false))
			$envmaptint->SetVecValue( Color::TOFLOAT(color.r), Color::TOFLOAT(color.g), Color::TOFLOAT(color.b) );
	}
	I::RenderView->SetColorModulation( Color::TOFLOAT(color.r), Color::TOFLOAT(color.g), Color::TOFLOAT(color.b) );
	I::RenderView->SetBlend(Color::TOFLOAT(color.a));
}



IMaterial* CMaterials::GetMaterial(std::string sName)
{
	if (sName == "Original")
		return nullptr;
	for (auto& mat : vChamMaterials)
	{
		if (sName == mat.sName)
			return mat.pMaterial;
	}
	for (auto& mat : vGlowMaterials)
	{
		if (sName == mat.sName)
			return mat.pMaterial;
	}
	return nullptr;
}

std::string CMaterials::GetVMT(std::string sName)
{
	for (auto& mat : vChamMaterials)
	{
		if (sName == mat.sName)
			return mat.sVMT;
	}
	return "";
}

void CMaterials::AddMaterial(std::string sName)
{
	if (sName == "Original" || std::filesystem::exists(MaterialFolder + "\\" + sName + ".vmt"))
		return;
	for (auto& mat : vChamMaterials)
	{
		if (sName == mat.sName)
			return;
	}
	for (auto& mat : vGlowMaterials)
	{
		if (sName == mat.sName)
			return;
	}

	CMaterial mat = { sName };
	mat.sVMT = "\"VertexLitGeneric\"\n{\n\t\n}";

	const auto kv = new KeyValues(mat.sName.c_str());
	g_KeyValUtils.LoadFromBuffer(kv, mat.sName.c_str(), mat.sVMT.c_str());
	mat.pMaterial = CreateNRef(std::format("m_pMat%s", mat.sName).c_str(), kv);

	vChamMaterials.push_back(mat);

	std::ofstream outStream(MaterialFolder + "\\" + sName + ".vmt");
	outStream << mat.sVMT;
	outStream.close();
}

void CMaterials::EditMaterial(std::string sName, std::string sVMT)
{
	if (!std::filesystem::exists(MaterialFolder + "\\" + sName + ".vmt"))
		return;

	for (auto& mat : vChamMaterials)
	{
		if (sName != mat.sName)
			continue;

		if (mat.bLocked)
			return;

		mat.sVMT = sVMT;

		const auto kv = new KeyValues(mat.sName.c_str());
		g_KeyValUtils.LoadFromBuffer(kv, mat.sName.c_str(), mat.sVMT.c_str());
		mat.pMaterial = CreateNRef(std::format("m_pMat%s", mat.sName).c_str(), kv);

		std::ofstream outStream(MaterialFolder + "\\" + sName + ".vmt");
		outStream << mat.sVMT;
		outStream.close();

		break;
	}
}

void CMaterials::RemoveMaterial(std::string sName)
{
	if (!std::filesystem::exists(MaterialFolder + "\\" + sName + ".vmt"))
		return;

	for (auto mat = vChamMaterials.begin(); mat != vChamMaterials.end(); mat++)
	{
		if (sName != mat->sName)
			continue;

		if (mat->bLocked)
			return;

		if (mat->pMaterial)
		{
			mat->pMaterial->DecrementReferenceCount();
			mat->pMaterial->DeleteIfUnreferenced();
			mat->pMaterial = nullptr;
		}

		vChamMaterials.erase(mat);

		break;
	}

	std::filesystem::remove(MaterialFolder + "\\" + sName + ".vmt");
}