#include "config.h"
#include "Renderer/PSOManager/PSOKeys.h"

/// const設定
std::string kDefaultClientTitle_ = "kEngine";

/// インスタンス設定
std::string config::clientTitle_{};
int config::clientWidth_ = kDefaultClientWidth_;
int config::clientHeight_ = kDefaultClientHeight_;
WindowMode config::clientWindowMode_ = kDefaultWindowMode_;

std::string config::psoCacheFolderPath_ = "./kEngine/EngineAssets/Shader/";
std::string config::psoCacheFolderName_ = "PSOCache.json";

bool config::isConnectKeyboard_ = false;
bool config::isConnectMouse_ = false;
bool config::isConnectGamePad_ = false;
gamepadType config::gamePadType_ = NODEVICE;
int config::XInputDriverIP_ = -1;

int config::default_White5x5_TextureHandle_ = 0;

int config::default_Plane_MeshBufferHandle_     = 0;
int config::default_Triangle_MeshBufferHandle_	= 0;
int config::default_Sprite2D_MeshBufferHandle_	= 0;
int config::default_Cube_MeshBufferHandle_		= 0;
int config::default_Sphere_MeshBufferHandle_	= 0;
int config::default_SkyCube_MeshBufferHandle_   = 0;
int config::default_Ring_MeshBufferHandle_		= 0;
int config::default_Cylinder_MeshBufferHandle_	= 0;

int config::default_LightModel_				= (int)LightModelType::			BlinnPhongReflection;
int config::default_RenderModelTypes_		= (int)RenderModelType::		Static;
int config::default_RasterizerMode_			= (int)RasterizerMode::			CullBack;
int config::default_BlendMode_				= (int)BlendModeType::			AlphaBlend;
int config::default_DepthStenctilState_		= (int)DepthStencilType::		Default;
int config::default_RenderTargetFormatType_ = (int)RenderTargetFormatType:: BackBuffer;

void config::SetClientWindowMode(WindowMode mode) {
	if(mode < 0 || mode >= Num_Of_Type) return;
	clientWindowMode_ = mode; 
}
