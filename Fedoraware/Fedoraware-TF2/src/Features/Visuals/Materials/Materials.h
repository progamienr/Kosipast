#pragma once
#include "../../Feature.h"

#define MaterialFolder (g_CFG.GetConfigPath() + "\\Materials")

struct CMaterial
{
	std::string sName = "";
	IMaterial* pMaterial;

	std::string sVMT = ""; // will be shown to user through material editor, for glow this won't matter
	bool bLocked = false;

};

class CMaterials
{
public:
	IMaterial* CreateNRef(char const* szName, void* pKV);

	void ReloadMaterials();
	void RemoveMaterials();

	IMaterial* GetMaterial(std::string sName);
	std::string GetVMT(std::string sName);

	void AddMaterial(std::string sName);
	void EditMaterial(std::string sName, std::string sVMT);
	void RemoveMaterial(std::string sName);

	void SetColor(IMaterial* material, Color_t color);

	std::vector<CMaterial> vChamMaterials;
	std::vector<CMaterial> vGlowMaterials;
	std::vector<IMaterial*> vMatList;
};

ADD_FEATURE(CMaterials, Materials)