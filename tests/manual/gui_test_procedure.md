# SimplePanner GUI Manual Test Procedure

## Task 5.9: GUI Testing

This document covers manual GUI testing procedures for SimplePanner. These tests complement the automated tests and require visual inspection and interactive testing.

---

## Test Environment Setup

### Prerequisites
1. Build the plugin with GUI support enabled
   ```bash
   cd build
   cmake -DSMTG_ENABLE_VSTGUI_SUPPORT=ON ..
   cmake --build .
   ```
2. Install plugin to system VST3 folder (already done by build)
3. Launch a DAW with VST3 support (Reaper, Ardour, etc.)
4. Load SimplePanner on an audio track

### Test Configuration
- **DAW**: Reaper 7.x (or equivalent)
- **Platform**: Linux (Primary), Windows/macOS (Optional)
- **Display**: 1920x1080 or higher
- **Input Method**: Mouse and keyboard

---

## Automated Test Coverage Summary

Before manual testing, verify all automated tests pass:

```bash
cd build
ctest --output-on-failure
```

**Automated GUI Tests** (8 tests total):
- `test_gui_formatting` (18 tests) - Value formatting functions
  ✓ formatPanValue() - "L100", "C", "R50" formatting
  ✓ formatGainValue() - "-∞ dB", "+3.0 dB" formatting
  ✓ formatDelayValue() - "10.0 ms" formatting

- `test_gui_parameter_sync` (13 tests) - Parameter synchronization
  ✓ controlBeginEdit() notifications
  ✓ controlEndEdit() notifications
  ✓ GUI → Controller synchronization
  ✓ Controller → GUI synchronization

- `test_gui_visual` (8 tests) - GUI stability and resource management
  ✓ Editor creation/destruction cycles (10+ times)
  ✓ Null controller handling
  ✓ Default parameter validation
  ✓ Stress tests (rapid changes, extreme values)

**Status**: All 8 test suites passing (100% coverage for automated testable functionality)

---

## Manual Test Cases

### TC-GUI-1: GUI Display and Layout

**Objective**: Verify GUI displays correctly with proper layout and styling

**Steps**:
1. Open SimplePanner GUI in DAW
2. Inspect overall layout
3. Check all controls are visible
4. Verify color scheme consistency

**Expected Results**:
- Window size: 600x400 pixels
- Background: Dark gray (#3C3C3C)
- Three sections visible:
  - Left Channel (top-left, medium gray #505050)
  - Right Channel (top-right, medium gray #505050)
  - Master (bottom, medium gray #505050)
- Section titles: Light gray (#B0B0B0) color
- Value labels: White text
- All controls fully visible, not clipped

**Checklist**:
- [ ] Window opens without errors
- [ ] Correct window dimensions
- [ ] All 3 sections visible
- [ ] Colors match specification
- [ ] No visual artifacts or corruption
- [ ] Text is readable

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

### TC-GUI-2: Left Channel Controls

**Objective**: Verify all Left Channel controls function correctly

#### Test 2.1: Left Pan Slider
**Steps**:
1. Locate Left Pan slider (horizontal, 200x15px)
2. Observe "L" and "R" labels on sides
3. Drag slider from left to right
4. Observe value label above slider

**Expected**:
- Slider moves smoothly
- Value updates: "L100" → "C" → "R100"
- L/R labels visible (#B0B0B0 gray)
- Slider handle: Blue (#2196F3)

**Checklist**:
- [ ] Slider responds to mouse drag
- [ ] Value label updates correctly
- [ ] L/R labels visible
- [ ] Smooth visual feedback

#### Test 2.2: Left Gain Knob
**Steps**:
1. Locate Left Gain knob (60x60px)
2. Drag knob in circular motion
3. Try Shift+drag for fine adjustment
4. Double-click to reset to 0dB

**Expected**:
- Knob rotates smoothly
- Value: "-∞ dB" at minimum, "+6.0 dB" at maximum
- Shift+drag: slower, finer adjustment
- Double-click: resets to 0dB

**Checklist**:
- [ ] Knob responds to drag
- [ ] Value display updates
- [ ] Shift+drag works
- [ ] Double-click reset works
- [ ] Range: -60dB to +6dB

#### Test 2.3: Left Delay Knob
**Steps**:
1. Locate Left Delay knob (60x60px)
2. Drag knob
3. Observe value label

**Expected**:
- Value: "0.0 ms" to "100.0 ms"
- Smooth updates

**Checklist**:
- [ ] Knob responds to drag
- [ ] Value updates correctly
- [ ] Range: 0ms to 100ms

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

### TC-GUI-3: Right Channel Controls

**Objective**: Verify all Right Channel controls (mirror of Left Channel)

#### Test 3.1: Right Pan Slider
- [ ] Slider functions correctly
- [ ] Value displays correctly
- [ ] L/R labels visible
- [ ] Visual feedback appropriate

#### Test 3.2: Right Gain Knob
- [ ] Knob functions correctly
- [ ] Value range: -60dB to +6dB
- [ ] Shift+drag works
- [ ] Double-click reset works

#### Test 3.3: Right Delay Knob
- [ ] Knob functions correctly
- [ ] Value range: 0ms to 100ms
- [ ] Value updates correctly

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

### TC-GUI-4: Master Section Controls

**Objective**: Verify Master Gain and Link L/R Gain controls

#### Test 4.1: Master Gain Knob
**Steps**:
1. Locate Master Gain knob in Master section
2. Drag knob through full range
3. Test Shift+drag fine adjustment
4. Test double-click reset

**Expected**:
- Value range: -60dB to +6dB
- Displays "-∞ dB" at minimum
- Displays "0.0 dB" as default
- Smooth rotation and updates

**Checklist**:
- [ ] Knob functions correctly
- [ ] Value range correct
- [ ] Shift+drag works
- [ ] Double-click reset works

#### Test 4.2: Link L/R Gain Toggle
**Steps**:
1. Locate "LINKED" button (80x25px)
2. Click to toggle OFF → ON
3. Observe visual state change
4. Click to toggle ON → OFF

**Expected**:
- OFF state: Gray (#606060), light gray text
- ON state: Green (#4CAF50), white text
- Toggle behavior works smoothly
- Visual feedback is clear

**Checklist**:
- [ ] Button toggles correctly
- [ ] OFF state: Gray color
- [ ] ON state: Green color
- [ ] Text color changes
- [ ] Visual feedback clear

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

### TC-GUI-5: Special Operations

**Objective**: Verify special mouse operations work correctly

#### Test 5.1: Double-Click Reset
**Steps**:
1. Change Left Gain to -10dB
2. Double-click on Left Gain knob
3. Verify it resets to 0dB
4. Repeat for all knobs and sliders

**Expected**:
- All knobs reset to default on double-click
- Pan sliders reset to default (Left=-100, Right=+100)
- Visual feedback is immediate

**Checklist**:
- [ ] Left Pan resets to -100 (L100)
- [ ] Right Pan resets to +100 (R100)
- [ ] All Gain knobs reset to 0dB
- [ ] All Delay knobs reset to 0ms

#### Test 5.2: Shift+Drag Fine Adjustment
**Steps**:
1. Click Left Gain knob with Shift held
2. Drag slowly
3. Observe value changes
4. Compare with normal drag (no Shift)

**Expected**:
- Shift+drag: Slower, finer adjustment (10x zoom factor)
- Normal drag: Normal speed
- Clear difference in sensitivity

**Checklist**:
- [ ] Shift+drag is noticeably slower
- [ ] Fine adjustment is precise
- [ ] Works on all knobs
- [ ] No visual glitches

#### Test 5.3: Mouse Wheel (if supported)
**Steps**:
1. Hover over a knob
2. Scroll mouse wheel up/down
3. Observe if value changes

**Expected**:
- Mouse wheel may or may not be supported
- If supported: value should change smoothly

**Checklist**:
- [ ] Mouse wheel support: Yes / No / N/A
- [ ] If yes: works smoothly

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

### TC-GUI-6: GUI Open/Close Stability

**Objective**: Verify GUI can be opened/closed repeatedly without crashes

**Steps**:
1. Close SimplePanner GUI
2. Reopen GUI from DAW
3. Repeat 10 times
4. Monitor for crashes, freezes, or errors

**Expected**:
- No crashes
- No memory leaks
- GUI opens quickly each time
- All controls initialize correctly

**Checklist**:
- [ ] Cycle 1-5: No issues
- [ ] Cycle 6-10: No issues
- [ ] No slowdowns observed
- [ ] No error messages

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

**Note**: This test case is also covered by automated test `test_gui_visual.cpp` with 100 cycles.

---

### TC-GUI-7: Parameter Value Display Updates

**Objective**: Verify value displays update correctly when parameters change

#### Test 7.1: GUI-Initiated Changes
**Steps**:
1. Drag Left Pan slider
2. Verify value label updates in real-time
3. Repeat for all controls

**Expected**:
- Value labels update immediately
- No lag or flicker
- Values are accurate

**Checklist**:
- [ ] Pan labels update (L100, C, R50, etc.)
- [ ] Gain labels update (-∞ dB, +3.0 dB, etc.)
- [ ] Delay labels update (0.0 ms, 50.0 ms, etc.)
- [ ] Updates are smooth

#### Test 7.2: Automation-Initiated Changes
**Steps**:
1. Record automation for Left Pan
2. Play back with GUI open
3. Observe GUI updates during playback

**Expected**:
- Controls move to reflect automation
- Value labels update
- No visual glitches

**Checklist**:
- [ ] Controls follow automation
- [ ] Value labels update correctly
- [ ] Smooth visual feedback

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

### TC-GUI-8: Platform-Specific Tests

**Objective**: Verify GUI works correctly on different platforms

#### Test 8.1: Linux (Primary Platform)
**Environment**:
- OS: Linux (kernel 4.4.0+)
- Wayland/X11: _______________
- DAW: _______________

**Checklist**:
- [ ] GUI opens correctly
- [ ] All controls respond to mouse
- [ ] No rendering artifacts
- [ ] Colors display correctly
- [ ] Font rendering is clear
- [ ] Window resizing (if applicable)

**Status**: [ ] Pass [ ] Fail

#### Test 8.2: Windows (Optional)
**Environment**:
- OS: Windows 10/11
- DAW: _______________

**Checklist**:
- [ ] GUI opens correctly
- [ ] All controls function
- [ ] No rendering issues
- [ ] HiDPI scaling correct

**Status**: [ ] Pass [ ] Fail [ ] N/A

#### Test 8.3: macOS (Optional)
**Environment**:
- OS: macOS _______________
- DAW: _______________

**Checklist**:
- [ ] GUI opens correctly
- [ ] All controls function
- [ ] Retina display support
- [ ] No rendering issues

**Status**: [ ] Pass [ ] Fail [ ] N/A

**Notes**: _______________________________________________________________

---

### TC-GUI-9: DAW Compatibility

**Objective**: Verify GUI works in different DAWs

#### Test 9.1: Reaper
**Version**: _______________

**Checklist**:
- [ ] GUI opens/closes correctly
- [ ] Plugin parameter window displays
- [ ] All controls function
- [ ] No crashes or freezes

**Status**: [ ] Pass [ ] Fail [ ] N/A

#### Test 9.2: Ardour (Linux)
**Version**: _______________

**Checklist**:
- [ ] GUI opens/closes correctly
- [ ] Controls function
- [ ] No issues observed

**Status**: [ ] Pass [ ] Fail [ ] N/A

#### Test 9.3: Other DAWs
**DAW**: _______________

**Checklist**:
- [ ] GUI opens/closes correctly
- [ ] All controls function
- [ ] No issues observed

**Status**: [ ] Pass [ ] Fail [ ] N/A

**Notes**: _______________________________________________________________

---

### TC-GUI-10: Visual Polish

**Objective**: Verify visual styling and polish

**Checklist**:
- [ ] Color scheme is consistent
  - Background: #3C3C3C
  - Group boxes: #505050
  - Title labels: #B0B0B0
  - Value text: White
  - Slider handles: Blue #2196F3
  - Link button ON: Green #4CAF50
  - Link button OFF: Gray #606060

- [ ] Fonts are consistent
  - Titles: kNormalFontBig
  - Labels: kNormalFont
  - L/R labels: kNormalFontSmall

- [ ] Layout is aligned
  - Controls are evenly spaced
  - Labels are centered
  - Groups are aligned

- [ ] No visual artifacts
  - No clipping
  - No overlapping controls
  - No rendering glitches

**Status**: [ ] Pass [ ] Fail
**Notes**: _______________________________________________________________

---

## Test Execution Summary

### Test Results

| Test Case | Status | Critical? | Notes |
|-----------|--------|-----------|-------|
| TC-GUI-1: Display and Layout | [ ] | Yes | |
| TC-GUI-2: Left Channel | [ ] | Yes | |
| TC-GUI-3: Right Channel | [ ] | Yes | |
| TC-GUI-4: Master Section | [ ] | Yes | |
| TC-GUI-5: Special Operations | [ ] | No | |
| TC-GUI-6: Open/Close Stability | [ ] | Yes | |
| TC-GUI-7: Value Updates | [ ] | Yes | |
| TC-GUI-8: Platform Tests | [ ] | Yes | |
| TC-GUI-9: DAW Compatibility | [ ] | No | |
| TC-GUI-10: Visual Polish | [ ] | No | |

**Total Passed**: _____ / 10
**Critical Passed**: _____ / 6

### Automated Test Results

```
Run automated tests before manual testing:
$ cd build && ctest --output-on-failure

Expected: 8/8 tests passed (100%)
Actual: _____ / 8 passed
```

### VST3 Validator Results

```
$ cd build && bin/Debug/validator VST3/Debug/SimplePanner.vst3

Expected: 47/47 tests passed
Actual: _____ / 47 passed
```

---

## Test Sign-Off

**Tester Name**: _______________________________________________

**Date**: ______________________________________________________

**Platform**: __________________________________________________

**DAW Used**: __________________________________________________

**Overall Assessment**: [ ] Pass [ ] Fail [ ] Partial Pass

**Critical Issues Found**:
___________________________________________________________________
___________________________________________________________________
___________________________________________________________________

**Recommendations**:
___________________________________________________________________
___________________________________________________________________
___________________________________________________________________

**Approved for Release**: [ ] Yes [ ] No

**Approver**: _______________________________________________

**Signature**: _______________________________________________

---

## Known Limitations

1. **Automated GUI Tests**: Cannot test visual appearance or mouse interactions
2. **Platform Coverage**: Primary testing on Linux, Windows/macOS optional
3. **DAW Coverage**: Focused on Reaper, other DAWs optional
4. **Display Scaling**: HiDPI/Retina testing requires appropriate hardware

## References

- **Design Document**: `docs/design.md` (Section 5: GUI Design)
- **Requirements**: `docs/requirements.md` (Section 7.2: GUI Requirements)
- **Manual Test Procedure**: `tests/manual/test_procedure.md` (Audio functionality)
- **Automated Tests**:
  - `tests/unit/test_gui_formatting.cpp`
  - `tests/unit/test_gui_parameter_sync.cpp`
  - `tests/unit/test_gui_visual.cpp`
