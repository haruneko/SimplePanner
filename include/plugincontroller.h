#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerController
//------------------------------------------------------------------------
class SimplePannerController : public Vst::EditController
{
public:
    SimplePannerController();
    ~SimplePannerController() SMTG_OVERRIDE;

    // EditController のオーバーライド
    tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
    tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    tresult PLUGIN_API setComponentState(IBStream* state) SMTG_OVERRIDE;

    // Create function
    static FUnknown* createInstance(void* /*context*/)
    {
        return (Vst::IEditController*)new SimplePannerController;
    }
};

} // namespace SimplePanner
} // namespace Steinberg
