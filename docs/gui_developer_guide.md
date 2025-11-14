# SimplePanner GUI Developer Guide

**Version**: 1.0.0
**Last Updated**: 2025-01-14
**Target Audience**: Developers extending or customizing SimplePanner GUI

---

## Table of Contents

1. [Overview](#overview)
2. [GUI Architecture](#gui-architecture)
3. [VSTGUI Basics](#vstgui-basics)
4. [SimplePannerEditor Class](#simplepa

nnereditor-class)
5. [Adding New Controls](#adding-new-controls)
6. [Styling and Theming](#styling-and-theming)
7. [Testing GUI Changes](#testing-gui-changes)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)

---

## Overview

### What is SimplePannerEditor?

`SimplePannerEditor` is the GUI component of SimplePanner, built on top of Steinberg's **VSTGUI** framework. It provides a graphical interface for users to interact with the plugin's 8 parameters.

### Key Features

- ✅ **600x400 pixel window** - Compact, fixed-size layout
- ✅ **3 section design** - Left Channel, Right Channel, Master
- ✅ **8 controls** - 2 sliders, 5 knobs, 1 toggle button
- ✅ **Real-time value display** - Labels update as parameters change
- ✅ **Bidirectional sync** - GUI ↔ Controller parameter synchronization
- ✅ **Special operations** - Double-click reset, Shift+drag fine-tune

### Technology Stack

- **VSTGUI 4.x**: Cross-platform GUI framework from Steinberg
- **VST3 SDK**: Audio plugin API
- **C++17**: Modern C++ with RAII and smart pointers
- **CMake**: Build system with VSTGUI support

---

## GUI Architecture

### Class Hierarchy

```
Vst::VSTGUIEditor (VST3 SDK base class)
    ↑
    └── SimplePannerEditor (Our implementation)
            ↓ implements
        VSTGUI::IControlListener (Listener interface)
```

### File Structure

```
SimplePanner/
├── include/
│   └── plugineditor.h         # SimplePannerEditor class declaration
├── source/
│   └── plugineditor.cpp       # GUI implementation
└── tests/
    └── unit/
        ├── test_gui_formatting.cpp      # Value formatting tests
        ├── test_gui_parameter_sync.cpp  # Parameter sync tests
        └── test_gui_visual.cpp          # GUI stability tests
```

### Key Components

1. **SimplePannerEditor**: Main GUI editor class
2. **CFrame**: Root container for all GUI elements
3. **CViewContainer**: Group containers (Left, Right, Master sections)
4. **CSlider**: Horizontal sliders for Pan parameters
5. **CKnob**: Rotary knobs for Gain/Delay parameters
6. **CTextButton**: Toggle button for Link L/R Gain
7. **CTextLabel**: Value display labels

---

## VSTGUI Basics

### What is VSTGUI?

VSTGUI is Steinberg's cross-platform GUI framework for VST plugins. It provides:
- Platform abstraction (Windows, macOS, Linux)
- Standard controls (sliders, knobs, buttons)
- Drawing primitives and colors
- Event handling (mouse, keyboard)

### Include Headers

```cpp
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace VSTGUI;
```

### Key VSTGUI Concepts

#### 1. CFrame - Root Container

Every GUI must have exactly one CFrame (main window):

```cpp
CFrame* newFrame = new CFrame(
    CRect(0, 0, kEditorWidth, kEditorHeight),  // Size
    this                                         // Editor
);
```

#### 2. Coordinates

VSTGUI uses **top-left origin** coordinates:
- (0, 0) = Top-left corner
- X increases to the right
- Y increases downward

```cpp
// Rectangle: left, top, right, bottom
CRect rect(20, 40, 300, 220);
//         x1  y1  x2   y2
```

#### 3. Colors

VSTGUI uses RGBA color format (0-255):

```cpp
CColor darkGray(60, 60, 60, 255);  // R, G, B, Alpha
//               R   G   B   A (255 = opaque)
```

#### 4. Control Lifecycle

VSTGUI uses **reference counting**:
- `new CControl(...)` → refCount = 1
- `container->addView(control)` → refCount++
- `container->removeView(control)` → refCount--
- When refCount == 0 → object deleted

**Important**: Never manually `delete` controls. Use `forget()` instead:

```cpp
CControl* control = new CControl(...);
// ... use control ...
control->forget();  // Decrements ref count
```

---

## SimplePannerEditor Class

### Class Declaration

**File**: `include/plugineditor.h`

```cpp
class SimplePannerEditor : public Vst::VSTGUIEditor, public IControlListener
{
public:
    SimplePannerEditor(void* controller);
    ~SimplePannerEditor();

    // VSTGUIEditor overrides
    bool PLUGIN_API open(void* parent, const PlatformType& platformType) SMTG_OVERRIDE;
    void PLUGIN_API close() SMTG_OVERRIDE;

    // IControlListener implementation
    void valueChanged(CControl* control) SMTG_OVERRIDE;
    void controlBeginEdit(CControl* control) SMTG_OVERRIDE;
    void controlEndEdit(CControl* control) SMTG_OVERRIDE;

protected:
    bool createUI();
    void updateValueDisplay(Vst::ParamID tag);

    // Value formatting helpers
    std::string formatPanValue(float normalized);
    std::string formatGainValue(float normalized);
    std::string formatDelayValue(float normalized);

private:
    // Control pointers (managed by CFrame)
    CSlider* mLeftPanSlider;
    CSlider* mRightPanSlider;
    CKnob* mLeftGainKnob;
    CKnob* mRightGainKnob;
    CKnob* mLeftDelayKnob;
    CKnob* mRightDelayKnob;
    CKnob* mMasterGainKnob;
    CTextButton* mLinkToggle;

    // Label pointers
    CTextLabel* mLeftPanLabel;
    CTextLabel* mRightPanLabel;
    CTextLabel* mLeftGainLabel;
    CTextLabel* mRightGainLabel;
    CTextLabel* mLeftDelayLabel;
    CTextLabel* mRightDelayLabel;
    CTextLabel* mMasterGainLabel;
};
```

### Implementation Overview

**File**: `source/plugineditor.cpp`

#### 1. Constructor

```cpp
SimplePannerEditor::SimplePannerEditor(void* controller)
: Vst::VSTGUIEditor(controller)
, mLeftPanSlider(nullptr)
// ... initialize all pointers to nullptr
{
    ViewRect viewRect(0, 0, kEditorWidth, kEditorHeight);
    setRect(viewRect);  // Set window size
}
```

#### 2. open() - Create GUI

```cpp
bool PLUGIN_API SimplePannerEditor::open(void* parent, const PlatformType& platformType)
{
    // 1. Create CFrame
    CFrame* newFrame = new CFrame(CRect(0, 0, kEditorWidth, kEditorHeight), this);

    // 2. Open frame (platform-specific window creation)
    if (!newFrame->open(parent, platformType)) {
        newFrame->forget();
        return false;
    }

    // 3. Set frame
    frame = newFrame;

    // 4. Create UI elements
    if (!createUI()) {
        close();
        return false;
    }

    return true;
}
```

#### 3. createUI() - Build Control Hierarchy

```cpp
bool SimplePannerEditor::createUI()
{
    CFrame* frm = getFrame();

    // Set background
    frm->setBackgroundColor(CColor(60, 60, 60, 255));

    // Create Left Channel section
    CViewContainer* leftGroup = new CViewContainer(CRect(20, 40, 300, 220));
    leftGroup->setBackgroundColor(CColor(80, 80, 80, 255));
    frm->addView(leftGroup);

    // Add controls to leftGroup
    // ...

    return true;
}
```

#### 4. close() - Cleanup

```cpp
void PLUGIN_API SimplePannerEditor::close()
{
    // Clear all pointers (CFrame handles deletion)
    mLeftPanSlider = nullptr;
    mRightPanSlider = nullptr;
    // ... clear all other pointers

    // Close and release frame
    if (frame) {
        frame->close();
        frame->forget();
        frame = nullptr;
    }
}
```

#### 5. IControlListener Methods

**valueChanged()** - Called when control value changes:

```cpp
void SimplePannerEditor::valueChanged(CControl* control)
{
    if (!control) return;

    int32 tag = control->getTag();           // Parameter ID
    float value = control->getValueNormalized();  // 0.0 - 1.0

    // Update controller
    if (getController()) {
        getController()->setParamNormalized(tag, value);
        getController()->performEdit(tag, value);
    }

    // Update value display
    updateValueDisplay(tag);
}
```

**controlBeginEdit()** - Called when user starts editing:

```cpp
void SimplePannerEditor::controlBeginEdit(CControl* control)
{
    if (!control) return;

    int32 tag = control->getTag();

    // Notify controller (for DAW automation recording)
    if (getController()) {
        getController()->beginEdit(tag);
    }
}
```

**controlEndEdit()** - Called when user finishes editing:

```cpp
void SimplePannerEditor::controlEndEdit(CControl* control)
{
    if (!control) return;

    int32 tag = control->getTag();

    // Notify controller
    if (getController()) {
        getController()->endEdit(tag);
    }
}
```

---

## Adding New Controls

### Example: Adding a New Slider

Let's add a hypothetical "Width" parameter slider.

#### Step 1: Add to plugids.h

```cpp
enum ParamID : Vst::ParamID
{
    // ... existing parameters
    kParamWidth = 8,  // New parameter
    kNumParameters = 9
};
```

#### Step 2: Register in Controller

**File**: `source/plugincontroller.cpp`

```cpp
parameters.addParameter(
    STR16("Width"),
    STR16("%"),
    0,
    0.5,  // Default: 50%
    Vst::ParameterInfo::kCanAutomate,
    kParamWidth
);
```

#### Step 3: Add Control Pointer

**File**: `include/plugineditor.h`

```cpp
private:
    // ... existing controls
    CSlider* mWidthSlider;      // New control
    CTextLabel* mWidthLabel;    // New label
```

#### Step 4: Initialize in Constructor

**File**: `source/plugineditor.cpp`

```cpp
SimplePannerEditor::SimplePannerEditor(void* controller)
: Vst::VSTGUIEditor(controller)
// ... existing initializers
, mWidthSlider(nullptr)
, mWidthLabel(nullptr)
{
    // ...
}
```

#### Step 5: Create Control in createUI()

```cpp
bool SimplePannerEditor::createUI()
{
    // ... existing code

    // Width Label
    CRect widthLabelRect(250, 370, 350, 385);
    mWidthLabel = new CTextLabel(widthLabelRect, "50%");
    mWidthLabel->setFont(kNormalFont);
    mWidthLabel->setFontColor(CColor(255, 255, 255, 255));
    mWidthLabel->setBackColor(CColor(80, 80, 80, 0));
    mWidthLabel->setStyle(CTextLabel::kNoDrawStyle);
    mWidthLabel->setHoriAlign(CHoriTxtAlign::kCenterText);
    masterGroup->addView(mWidthLabel);

    // Width Slider
    CRect widthSliderRect(250, 390, 350, 400);
    mWidthSlider = new CSlider(widthSliderRect, this, kParamWidth, 250, 350, nullptr, nullptr);
    mWidthSlider->setMin(0.0f);
    mWidthSlider->setMax(1.0f);
    mWidthSlider->setDefaultValue(0.5f);  // 50%
    mWidthSlider->setBackColor(CColor(100, 100, 100, 255));
    mWidthSlider->setFrameColor(CColor(60, 60, 60, 255));
    mWidthSlider->setValueColor(CColor(33, 150, 243, 255));
    masterGroup->addView(mWidthSlider);

    // Initialize value
    if (getController()) {
        float widthValue = getController()->getParamNormalized(kParamWidth);
        mWidthSlider->setValue(widthValue);
        mWidthLabel->setText(formatWidthValue(widthValue).c_str());
    }

    return true;
}
```

#### Step 6: Add Formatting Function

```cpp
std::string SimplePannerEditor::formatWidthValue(float normalized)
{
    float percent = normalized * 100.0f;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << percent << "%";
    return oss.str();
}
```

#### Step 7: Update updateValueDisplay()

```cpp
void SimplePannerEditor::updateValueDisplay(Vst::ParamID tag)
{
    // ... existing cases

    case kParamWidth:
        text = formatWidthValue(normalized);
        if (mWidthLabel)
            mWidthLabel->setText(text.c_str());
        break;
}
```

#### Step 8: Clear Pointer in close()

```cpp
void PLUGIN_API SimplePannerEditor::close()
{
    // ... existing clears
    mWidthSlider = nullptr;
    mWidthLabel = nullptr;

    // ...
}
```

#### Step 9: Write Tests

**File**: `tests/unit/test_gui_formatting.cpp`

```cpp
TEST(GUIFormatting, WidthValue)
{
    TestableEditor editor(nullptr);

    EXPECT_EQ(editor.testFormatWidthValue(0.0f), "0%");
    EXPECT_EQ(editor.testFormatWidthValue(0.5f), "50%");
    EXPECT_EQ(editor.testFormatWidthValue(1.0f), "100%");
}
```

---

## Styling and Theming

### Color Palette

SimplePanner uses a consistent dark theme:

```cpp
// Background colors
const CColor kBackgroundColor(60, 60, 60, 255);      // #3C3C3C - Dark gray
const CColor kGroupBoxColor(80, 80, 80, 255);        // #505050 - Medium gray
const CColor kSliderTrackColor(100, 100, 100, 255);  // #646464 - Slider background

// Text colors
const CColor kTitleColor(176, 176, 176, 255);        // #B0B0B0 - Light gray
const CColor kValueColor(255, 255, 255, 255);        // #FFFFFF - White
const CColor kLabelColor(176, 176, 176, 255);        // #B0B0B0 - L/R labels

// Accent colors
const CColor kSliderHandleColor(33, 150, 243, 255);  // #2196F3 - Blue
const CColor kLinkOnColor(76, 175, 80, 255);         // #4CAF50 - Green
const CColor kLinkOffColor(96, 96, 96, 255);         // #606060 - Gray
```

### Font Styles

```cpp
const CFontRef kNormalFont = kSystemFont;
const CFontRef kNormalFontBig = kSystemFont;      // Titles (14pt)
const CFontRef kNormalFontSmall = kSystemFont;    // L/R labels (10pt)
```

### Control Sizing Guidelines

```
Sliders (Pan):     200x15 px (horizontal)
Knobs (Gain/Delay): 60x60 px
Buttons (Link):     80x25 px
Labels (Values):    60x15 px (centered above control)
Labels (L/R):       10x15 px (small, centered)
Group Boxes:        280x180 px (Left/Right), 560x120 px (Master)
```

### Applying Consistent Styling

When creating a new control, always apply the standard style:

```cpp
// Slider example
CSlider* slider = new CSlider(rect, this, tag, ...);
slider->setBackColor(CColor(100, 100, 100, 255));    // Track
slider->setFrameColor(CColor(60, 60, 60, 255));      // Border
slider->setValueColor(CColor(33, 150, 243, 255));    // Handle (blue)

// Label example
CTextLabel* label = new CTextLabel(rect, "0.0 dB");
label->setFont(kNormalFont);
label->setFontColor(CColor(255, 255, 255, 255));     // White
label->setBackColor(CColor(80, 80, 80, 0));          // Transparent
label->setFrameColor(CColor(0, 0, 0, 0));            // No border
label->setStyle(CTextLabel::kNoDrawStyle);
label->setHoriAlign(CHoriTxtAlign::kCenterText);
```

---

## Testing GUI Changes

### Automated Tests

SimplePanner includes comprehensive GUI tests:

#### 1. Value Formatting Tests

**File**: `tests/unit/test_gui_formatting.cpp` (18 tests)

```cpp
TEST(GUIFormatting, PanValue)
{
    TestableEditor editor(nullptr);
    EXPECT_EQ(editor.testFormatPanValue(0.0f), "L100");
    EXPECT_EQ(editor.testFormatPanValue(0.5f), "C");
    EXPECT_EQ(editor.testFormatPanValue(1.0f), "R100");
}
```

#### 2. Parameter Synchronization Tests

**File**: `tests/unit/test_gui_parameter_sync.cpp` (13 tests)

```cpp
TEST(GUIParameterSync, GUIToControllerSync)
{
    // Tests bidirectional parameter sync
    MockControl control(kParamLeftGain);
    editor.valueChanged(&control);
    // Verify controller updated
}
```

#### 3. GUI Stability Tests

**File**: `tests/unit/test_gui_visual.cpp` (8 tests)

```cpp
TEST(GUIVisual, MultipleCreationCycles)
{
    // Stress test: 100 open/close cycles
    for (int i = 0; i < 100; ++i) {
        SimplePannerEditor* editor = new SimplePannerEditor(&mockController);
        editor->open(nullptr, platformType);
        editor->close();
        delete editor;
    }
    // No crashes, no leaks
}
```

### Running Tests

```bash
cd build
ctest --output-on-failure

# Run specific GUI tests
ctest -R test_gui_formatting
ctest -R test_gui_parameter_sync
ctest -R test_gui_visual
```

### Manual Testing

Use the comprehensive manual test procedure:

```bash
# View manual test procedure
cat tests/manual/gui_test_procedure.md
```

Key manual test areas:
- Visual appearance and layout
- Double-click reset functionality
- Shift+drag fine-tuning
- DAW compatibility

---

## Best Practices

### 1. Memory Management

✅ **DO**:
```cpp
CControl* control = new CControl(...);
container->addView(control);  // addView() increments ref count
// control is now managed by container
```

❌ **DON'T**:
```cpp
CControl* control = new CControl(...);
delete control;  // WRONG! Use forget() instead
```

### 2. Null Checks

✅ **DO**:
```cpp
void SimplePannerEditor::valueChanged(CControl* control)
{
    if (!control) return;  // Always check for null

    // Safe to use control
    int32 tag = control->getTag();
}
```

### 3. Controller Access

✅ **DO**:
```cpp
if (getController()) {
    getController()->setParamNormalized(tag, value);
}
```

❌ **DON'T**:
```cpp
getController()->setParamNormalized(tag, value);  // May crash if null
```

### 4. Value Display Updates

✅ **DO**:
```cpp
// Update display immediately after value change
void SimplePannerEditor::valueChanged(CControl* control)
{
    // ... update controller ...
    updateValueDisplay(tag);  // Update label
}
```

### 5. Platform Independence

✅ **DO**:
- Use VSTGUI abstractions (CColor, CRect, CFontRef)
- Test on multiple platforms if possible
- Follow VSTGUI best practices

❌ **DON'T**:
- Use platform-specific APIs directly
- Assume window coordinates or DPI

### 6. Test-Driven Development

✅ **DO**:
1. Write test first (e.g., test_gui_formatting.cpp)
2. Implement feature (e.g., formatPanValue())
3. Verify test passes

### 7. Consistent Styling

✅ **DO**:
- Use color constants defined in the palette
- Follow sizing guidelines
- Maintain visual consistency across controls

---

## Troubleshooting

### Problem: Control Not Responding

**Checklist**:
- [ ] Control added to container? `container->addView(control)`
- [ ] Listener set? `control->setListener(this)`
- [ ] Tag set correctly? `control->setTag(kParamXXX)`
- [ ] Controller pointer valid? `getController() != nullptr`

### Problem: Value Not Displaying

**Checklist**:
- [ ] Label pointer initialized? `mLabel = new CTextLabel(...)`
- [ ] Label added to container? `container->addView(mLabel)`
- [ ] updateValueDisplay() called? In valueChanged()
- [ ] Format function working? Test formatPanValue() etc.

### Problem: Memory Leak

**Common causes**:
- Forgetting to set control pointers to nullptr in close()
- Manually deleting controls (use forget() instead)
- Not calling parent class methods

**Solution**:
```cpp
void PLUGIN_API SimplePannerEditor::close()
{
    // Clear all control pointers
    mLeftPanSlider = nullptr;
    // ... clear all others

    // Close frame
    if (frame) {
        frame->close();
        frame->forget();  // Decrements ref count
        frame = nullptr;
    }
}
```

### Problem: Crash on Open/Close

**Checklist**:
- [ ] Calling parent class open/close?
- [ ] Checking frame != nullptr before use?
- [ ] Not accessing controls after close()?

### Problem: GUI Doesn't Update from Automation

**Solution**:
VSTGUIEditor has a built-in timer that polls for parameter changes. Ensure:
1. Controller's `getParamNormalized()` returns correct values
2. Parameter is marked as `kCanAutomate`
3. Frame is valid during playback

---

## Advanced Topics

### Custom Drawing

To create a fully custom control, inherit from CControl and override draw():

```cpp
class MyCustomKnob : public CControl
{
public:
    void draw(CDrawContext* context) override
    {
        // Custom drawing code
        context->setFillColor(CColor(255, 0, 0, 255));  // Red
        context->drawRect(getViewSize(), kDrawFilled);
    }
};
```

### Custom Mouse Handling

Override mouse methods for custom interactions:

```cpp
CMouseEventResult MyControl::onMouseDown(CPoint& where, const CButtonState& buttons) override
{
    if (buttons.isLeftButton()) {
        // Handle left click
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}
```

### Animation

Use a CVSTGUITimer for animated effects:

```cpp
class MyAnimatedControl : public CControl, public CVSTGUITimerListener
{
    void onTimer(CVSTGUITimer* timer) override
    {
        // Update animation state
        invalid();  // Trigger redraw
    }
};
```

---

## References

### Official Documentation

- [VSTGUI Documentation](https://steinbergmedia.github.io/vst3_doc/vstgui/html/)
- [VST3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)
- [VSTGUI GitHub](https://github.com/steinbergmedia/vstgui)

### SimplePanner Documentation

- **[requirements.md](requirements.md)**: GUI requirements (Section 7.2)
- **[design.md](design.md)**: GUI design specifications (Section 5)
- **[tasks.md](tasks.md)**: GUI implementation tasks (Phase 5)
- **[gui_test_report.md](gui_test_report.md)**: GUI test results
- **[gui_test_procedure.md](../tests/manual/gui_test_procedure.md)**: Manual testing guide

### Code Examples

- **SimplePanner GUI**: `source/plugineditor.cpp` - Complete GUI implementation
- **VST3 SDK Samples**: `vst3sdk/public.sdk/samples/vst/` - Official examples
- **VSTGUI Tests**: `tests/unit/test_gui_*.cpp` - Unit test examples

---

**SimplePanner GUI Developer Guide v1.0.0**
**© 2025 SimplePanner Project**
**License: MIT**
