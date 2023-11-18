#pragma once
#include "../../Feature.h"

struct ChamInfo
{
	int $flags = 0;
	int $flags_defined = 0;
	int $flags2 = 0;
	int $flags_defined2 = 0;
	int $frame = 0;
};

class CDMEChams
{
private:
	bool ShouldRun();
	IMaterial* GetChamMaterial(const Chams_t& chams);
	IMaterial* GetProxyMaterial(int nIndex);
	void RenderFakeAng(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

public:
	bool Render(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);
	bool m_bRendering;
	std::unordered_map<IMaterial*, ChamInfo> backupInformation;
};

ADD_FEATURE(CDMEChams, Chams)