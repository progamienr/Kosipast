#include "ConfigManager.h"

#include "../../Vars.h"
#include "../../../SDK/SDK.h"
#include "../../Misc/Notifications/Notifications.h"
#include "../../Visuals/Materials/Materials.h"

boost::property_tree::ptree WriteTree;
boost::property_tree::ptree ReadTree;

boost::property_tree::ptree CConfigManager::ColorToTree(const Color_t& color)
{
	boost::property_tree::ptree colorTree;
	colorTree.put("r", color.r);
	colorTree.put("g", color.g);
	colorTree.put("b", color.b);
	colorTree.put("a", color.a);

	return colorTree;
}

void CConfigManager::TreeToColor(const boost::property_tree::ptree& tree, Color_t& out)
{
	if (auto v = tree.get_optional<byte>("r")) { out.r = *v; }
	if (auto v = tree.get_optional<byte>("g")) { out.g = *v; }
	if (auto v = tree.get_optional<byte>("b")) { out.b = *v; }
	if (auto v = tree.get_optional<byte>("a")) { out.a = *v; }
}

boost::property_tree::ptree CConfigManager::VecToTree(const Vec3& vec)
{
	boost::property_tree::ptree vecTree;
	vecTree.put("x", vec.x);
	vecTree.put("y", vec.y);
	vecTree.put("z", vec.z);

	return vecTree;
}

void CConfigManager::TreeToVec(const boost::property_tree::ptree& tree, Vec3& out)
{
	if (auto v = tree.get_optional<float>("x")) { out.x = *v; }
	if (auto v = tree.get_optional<float>("y")) { out.y = *v; }
	if (auto v = tree.get_optional<float>("z")) { out.z = *v; }
}



void CConfigManager::SaveJson(const char* name, bool val)
{
	WriteTree.put(name, val);
}

void CConfigManager::SaveJson(const char* name, int val)
{
	WriteTree.put(name, val);
}

void CConfigManager::SaveJson(const char* name, float val)
{
	WriteTree.put(name, val);
}

void CConfigManager::SaveJson(const char* name, const std::string& val)
{
	WriteTree.put(name, val);
}

void CConfigManager::SaveJson(const char* name, const Color_t& val)
{
	WriteTree.put_child(name, ColorToTree(val));
}

void CConfigManager::SaveJson(const char* name, const Gradient_t& val)
{
	boost::property_tree::ptree gradientTree;
	gradientTree.put_child("StartColor", ColorToTree(val.StartColor));
	gradientTree.put_child("EndColor", ColorToTree(val.EndColor));

	WriteTree.put_child(name, gradientTree);
}

void CConfigManager::SaveJson(const char* name, const Vec3& val)
{
	WriteTree.put_child(name, VecToTree(val));
}

void CConfigManager::SaveJson(const char* name, const Chams_t& val)
{
	boost::property_tree::ptree chamTree;
	{
		boost::property_tree::ptree vectorTree;
		for (const auto& sMat : val.VisibleMaterial)
		{
			boost::property_tree::ptree child; child.put("", sMat);
			vectorTree.push_back(std::make_pair("", child));
		}
		chamTree.put_child("VisibleMaterial", vectorTree);
	}
	chamTree.put_child("VisibleColor", ColorToTree(val.VisibleColor));
	{
		boost::property_tree::ptree vectorTree;
		for (const auto& sMat : val.OccludedMaterial)
		{
			boost::property_tree::ptree child; child.put("", sMat);
			vectorTree.push_back(std::make_pair("", child));
		}
		chamTree.put_child("OccludedMaterial", vectorTree);
	}
	chamTree.put_child("OccludedColor", ColorToTree(val.OccludedColor));

	WriteTree.put_child(name, chamTree);
}

void CConfigManager::SaveJson(const char* name, const Glow_t& val)
{
	boost::property_tree::ptree glowTree;
	glowTree.put("Stencil", val.Stencil);
	glowTree.put("Blur", val.Blur);
	glowTree.put("StencilScale", val.StencilScale);
	glowTree.put("BlurScale", val.BlurScale);

	WriteTree.put_child(name, glowTree);
}

void CConfigManager::SaveJson(const char* name, const DragBox_t& val)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", val.x);
	dragBoxTree.put("y", val.y);

	WriteTree.put_child(name, dragBoxTree);
}

void CConfigManager::SaveJson(const char* name, const WindowBox_t& val)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", val.x);
	dragBoxTree.put("y", val.y);
	dragBoxTree.put("w", val.w);
	dragBoxTree.put("h", val.h);

	WriteTree.put_child(name, dragBoxTree);
}

void CConfigManager::LoadJson(const char* name, bool& val)
{
	if (auto getValue = ReadTree.get_optional<bool>(name))
	{
		val = *getValue;
	}
}

void CConfigManager::LoadJson(const char* name, int& val)
{
	if (auto getValue = ReadTree.get_optional<int>(name))
	{
		val = *getValue;
	}
}

void CConfigManager::LoadJson(const char* name, float& val)
{
	if (auto getValue = ReadTree.get_optional<float>(name))
	{
		val = *getValue;
	}
}

void CConfigManager::LoadJson(const char* name, std::string& val)
{
	if (auto getValue = ReadTree.get_optional<std::string>(name))
	{
		val = *getValue;
	}
}

void CConfigManager::LoadJson(const char* name, Color_t& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		TreeToColor(*getChild, val);
	}
}

void CConfigManager::LoadJson(const char* name, Gradient_t& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		if (const auto getStartColor = getChild->get_child_optional("StartColor"))
			TreeToColor(*getStartColor, val.StartColor);
		if (const auto endColor = getChild->get_child_optional("EndColor"))
			TreeToColor(*endColor, val.EndColor);
	}
}

void CConfigManager::LoadJson(const char* name, Vec3& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		TreeToVec(*getChild, val);
	}
}

void CConfigManager::LoadJson(const char* name, Chams_t& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		auto getMaterials = [](std::vector<std::string>& val, const boost::optional<boost::property_tree::ptree&> getVector)
			{
				if (!getVector)
					return;
				
				if (getVector->data() != "") // account for old format
					val = { getVector->data() };
				else
				{
					val.clear();
					for (auto& mat : *getVector)
					{
						std::string sMat = mat.second.data();

						bool bFound = false; // ensure no duplicates are assigned
						for (auto& str : val)
						{
							if (str == sMat)
							{
								bFound = true;
								break;
							}
						}

						if (!bFound)
							val.push_back(mat.second.data());
					}
				}

				// remove invalid materials
				for (auto it = val.begin(); it != val.end();)
				{
					if (*it == "None" || *it != "Original" && !F::Materials.mChamMaterials.contains(*it))
						it = val.erase(it);
					else
						++it;
				}
			};

		getMaterials(val.VisibleMaterial, getChild->get_child_optional("VisibleMaterial"));
		if (const auto getChildColor = getChild->get_child_optional("VisibleColor")) { TreeToColor(*getChildColor, val.VisibleColor); }
		getMaterials(val.OccludedMaterial, getChild->get_child_optional("OccludedMaterial"));
		if (const auto getChildColor = getChild->get_child_optional("OccludedColor")) { TreeToColor(*getChildColor, val.OccludedColor); }
	}
}

void CConfigManager::LoadJson(const char* name, Glow_t& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<bool>("Stencil")) { val.Stencil = *getValue; }
		if (auto getValue = getChild->get_optional<bool>("Blur")) { val.Blur = *getValue; }
		if (auto getValue = getChild->get_optional<int>("StencilScale")) { val.StencilScale = *getValue; }
		if (auto getValue = getChild->get_optional<int>("BlurScale")) { val.BlurScale = *getValue; }
	}
}

void CConfigManager::LoadJson(const char* name, DragBox_t& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { val.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { val.y = *getValue; }
		val.update = true;
	}
}

void CConfigManager::LoadJson(const char* name, WindowBox_t& val)
{
	if (const auto getChild = ReadTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { val.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { val.y = *getValue; }
		if (auto getValue = getChild->get_optional<int>("w")) { val.w = *getValue; }
		if (auto getValue = getChild->get_optional<int>("h")) { val.h = *getValue; }
		val.update = true;
	}
}

CConfigManager::CConfigManager()
{
	ConfigPath = std::filesystem::current_path().string() + "\\FedCfgs";
	VisualsPath = ConfigPath + "\\Visuals";

	// Create 'FedCfgs' folder (not FedFigs as I want to keep those separate)
	if (!std::filesystem::exists(ConfigPath))
		std::filesystem::create_directory(ConfigPath);

	if (!std::filesystem::exists(VisualsPath))
		std::filesystem::create_directory(VisualsPath);

	// Create 'Core' folder for Attribute-Changer & Playerlist
	if (!std::filesystem::exists(ConfigPath + "\\Core"))
		std::filesystem::create_directory(ConfigPath + "\\Core");

	// Create 'Materials' folder for custom materials
	if (!std::filesystem::exists(ConfigPath + "\\Materials"))
		std::filesystem::create_directory(ConfigPath + "\\Materials");
}

#define IsType(type) var->m_iType == typeid(type).hash_code()
#define SaveType(type) SaveJson(var->m_sName.c_str(), var->GetVar<type>()->Value)
#define SaveT(type) if (IsType(type)) SaveType(type);
#define LoadType(type) LoadJson(var->m_sName.c_str(), var->GetVar<type>()->Value)
#define LoadT(type) if (IsType(type)) LoadType(type);

bool CConfigManager::SaveConfig(const std::string& configName, bool bNotify)
{
	try
	{
		WriteTree.clear();

		for (const auto var : g_Vars)
		{
			if (var->m_iFlags & NOSAVE)
				continue;

			SaveT(bool)
			else SaveT(int)
			else SaveT(float)
			else SaveT(std::string)
			else SaveT(Color_t)
			else SaveT(Gradient_t)
			else SaveT(Vec3)
			else SaveT(Chams_t)
			else SaveT(Glow_t)
			else SaveT(DragBox_t)
			else SaveT(WindowBox_t)
		}

		write_json(ConfigPath + "\\" + configName + ConfigExtension, WriteTree);
		CurrentConfig = configName; CurrentVisuals = "";
		if (bNotify)
			F::Notifications.Add("Config " + configName + " saved");
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool CConfigManager::LoadConfig(const std::string& configName, bool bNotify)
{
	// Check if the config exists
	if (!std::filesystem::exists(g_CFG.GetConfigPath() + "\\" + configName + ConfigExtension))
	{
		// Save default config if one doesn't yet exist
		if (configName == std::string("default"))
			SaveConfig("default", false);

		return false;
	}

	// Read ptree from json
	try
	{
		ReadTree.clear();
		read_json(ConfigPath + "\\" + configName + ConfigExtension, ReadTree);

		for (const auto var : g_Vars)
		{
			if (var->m_iFlags & NOSAVE)
				continue;

			LoadT(bool)
			else LoadT(int)
			else LoadT(float)
			else LoadT(std::string)
			else LoadT(Color_t)
			else LoadT(Gradient_t)
			else LoadT(Vec3)
			else LoadT(Chams_t)
			else LoadT(Glow_t)
			else LoadT(DragBox_t)
			else LoadT(WindowBox_t)
		}

		g_Draw.RemakeFonts();

		CurrentConfig = configName; CurrentVisuals = "";
		if (bNotify)
			F::Notifications.Add("Config " + configName + " loaded");
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool CConfigManager::SaveVisual(const std::string& configName, bool bNotify)
{
	try
	{
		WriteTree.clear();

		for (const auto var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || var->m_iFlags & NOSAVE)
				continue;

			SaveT(bool)
			else SaveT(int)
			else SaveT(float)
			else SaveT(std::string)
			else SaveT(Color_t)
			else SaveT(Gradient_t)
			else SaveT(Vec3)
			else SaveT(Chams_t)
			else SaveT(Glow_t)
			else SaveT(DragBox_t)
			else SaveT(WindowBox_t)
		}

		write_json(ConfigPath + "\\Visuals\\" + configName + ConfigExtension, WriteTree);
		if (bNotify)
			F::Notifications.Add("Visual config " + configName + " saved");
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool CConfigManager::LoadVisual(const std::string& configName, bool bNotify)
{
	// Check if the visual config exists
	if (!std::filesystem::exists(g_CFG.GetVisualsPath() + "\\" + configName + ConfigExtension))
	{
		//if (configName == std::string("default"))
		//	SaveVisual("default");
		return false;
	}

	try
	{
		ReadTree.clear();
		read_json(ConfigPath + "\\Visuals\\" + configName + ConfigExtension, ReadTree);

		for (const auto var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || var->m_iFlags & NOSAVE)
				continue;

			LoadT(bool)
			else LoadT(int)
			else LoadT(float)
			else LoadT(std::string)
			else LoadT(Color_t)
			else LoadT(Gradient_t)
			else LoadT(Vec3)
			else LoadT(Chams_t)
			else LoadT(Glow_t)
			else LoadT(DragBox_t)
			else LoadT(WindowBox_t)
		}

		g_Draw.RemakeFonts();

		CurrentVisuals = configName;
		if (bNotify)
			F::Notifications.Add("Visual config " + configName + " loaded");
	}
	catch (...)
	{
		return false;
	}
	return true;
}

void CConfigManager::RemoveConfig(const std::string& configName)
{
	std::filesystem::remove(ConfigPath + "\\" + configName + ConfigExtension);
}

void CConfigManager::RemoveVisual(const std::string& configName)
{
	std::filesystem::remove(VisualsPath + "\\" + configName + ConfigExtension);
}