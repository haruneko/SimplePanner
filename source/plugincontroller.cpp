#include "plugincontroller.h"
#include "plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"

namespace Steinberg {
namespace SimplePanner {

//------------------------------------------------------------------------
// SimplePannerController
//------------------------------------------------------------------------
SimplePannerController::SimplePannerController()
{
}

//------------------------------------------------------------------------
SimplePannerController::~SimplePannerController()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::initialize(FUnknown* context)
{
    tresult result = EditController::initialize(context);
    if (result != kResultOk)
        return result;

    // TODO: 8つのパラメータを登録
    // これは Task 1.5 で実装します

    return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::terminate()
{
    return EditController::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API SimplePannerController::setComponentState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    // TODO: Processor状態からパラメータ値を復元
    // これは Task 1.5 で実装します

    return kResultOk;
}

} // namespace SimplePanner
} // namespace Steinberg
