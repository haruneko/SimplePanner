#include "public.sdk/source/main/pluginfactory.h"
#include "pluginprocessor.h"
#include "plugincontroller.h"
#include "plugids.h"

#define PLUGIN_NAME "SimplePanner"
#define PLUGIN_VENDOR "Example Company"
#define PLUGIN_VERSION "1.0.0"
#define PLUGIN_EMAIL "info@example.com"
#define PLUGIN_URL "https://www.example.com"

//------------------------------------------------------------------------
BEGIN_FACTORY_DEF(PLUGIN_VENDOR, PLUGIN_URL, PLUGIN_EMAIL)

    DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::SimplePanner::ProcessorUID),
               PClassInfo::kManyInstances,
               kVstAudioEffectClass,
               PLUGIN_NAME,
               Vst::kDistributable,
               Vst::PlugType::kFx,
               PLUGIN_VERSION,
               kVstVersionString,
               Steinberg::SimplePanner::SimplePannerProcessor::createInstance)

    DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::SimplePanner::ControllerUID),
               PClassInfo::kManyInstances,
               kVstComponentControllerClass,
               PLUGIN_NAME "Controller",
               0,
               "",
               PLUGIN_VERSION,
               kVstVersionString,
               Steinberg::SimplePanner::SimplePannerController::createInstance)

END_FACTORY

//------------------------------------------------------------------------
bool InitModule()
{
    return true;
}

//------------------------------------------------------------------------
bool DeinitModule()
{
    return true;
}
