# SimplePanner Manual Test Procedures

## Setup

### Prerequisites
1. Build the plugin
2. Copy `SimplePanner.vst3` to VST3 folder
3. Launch DAW (Reaper recommended)
4. Prepare test audio files:
   - Pink noise (stereo, -18dBFS)
   - 1kHz sine wave (stereo, -12dBFS)
   - Music sample (stereo, commercial track)

### DAW Setup
1. Create new project at 44.1kHz, 512 samples buffer
2. Create stereo audio track
3. Load test audio file
4. Insert SimplePanner on the track
5. Monitor output with headphones

---

## Test Cases

### TC-1: Default State (Bypass Equivalent)

**Objective**: Verify plugin is transparent in default state

**Steps**:
1. Load plugin on audio track
2. Play pink noise
3. Compare with and without plugin (bypass)

**Expected Result**:
- No audible difference
- Left input → Left output
- Right input → Right output
- No delay, no gain change

**Status**: [ ] Pass [ ] Fail

---

### TC-2: Pan Parameters

**Objective**: Verify panning functionality

**Test 2.1: Left Pan**
1. Set Left Pan = 0 (Center)
2. Play stereo pink noise
3. **Expected**: Left input appears centered

**Test 2.2: Right Pan**
1. Set Right Pan = 0 (Center)
2. Play stereo pink noise
3. **Expected**: Right input appears centered

**Test 2.3: LR Swap**
1. Set Left Pan = +100 (Full Right)
2. Set Right Pan = -100 (Full Left)
3. Play stereo test signal with L=100Hz, R=1kHz tones
4. **Expected**: Channels are swapped

**Test 2.4: Equal Power**
1. Set Left Pan = 0 (Center)
2. Right Pan = +100 (default)
3. Play mono signal on left input only
4. **Expected**: Perceivable loudness stays constant

**Status**: [ ] Pass [ ] Fail

---

### TC-3: Gain Parameters

**Objective**: Verify gain control

**Test 3.1: Unity Gain (0dB)**
1. All gains at 0dB
2. Play pink noise at -18dBFS
3. Measure output level
4. **Expected**: -18dBFS output

**Test 3.2: -6dB Gain**
1. Set Left Gain = -6dB
2. Play pink noise
3. **Expected**: Left channel is half as loud

**Test 3.3: +6dB Gain**
1. Set Right Gain = +6dB
2. Play pink noise
3. **Expected**: Right channel is approximately twice as loud

**Test 3.4: -60dB (Mute)**
1. Set Left Gain = -60dB
2. Play pink noise
3. **Expected**: Left channel is completely silent

**Status**: [ ] Pass [ ] Fail

---

### TC-4: Delay Parameters (Haas Effect)

**Objective**: Verify delay functionality

**Test 4.1: No Delay**
1. All delays = 0ms
2. Play stereo pink noise
3. **Expected**: No phase issues, centered image

**Test 4.2: 10ms Delay**
1. Set Right Delay = 10ms
2. Play stereo pink noise (equal L/R)
3. **Expected**: Phantom source shifts toward left

**Test 4.3: 30ms Delay**
1. Set Left Delay = 30ms
2. Play stereo pink noise
3. **Expected**: Phantom source shifts toward right

**Test 4.4: Maximum Delay (100ms)**
1. Set Right Delay = 100ms
2. Play click or impulse
3. **Expected**: Clear echo/slap-back, ~100ms delay

**Status**: [ ] Pass [ ] Fail

---

### TC-5: Master Gain

**Objective**: Verify master gain control

**Test 5.1: Master -6dB**
1. Set Master Gain = -6dB
2. All other gains at 0dB
3. Play pink noise
4. **Expected**: Both channels are -6dB quieter

**Test 5.2: Master + Individual Gains**
1. Set Left Gain = -3dB
2. Set Master Gain = -3dB
3. Play pink noise on left
4. **Expected**: Total attenuation is -6dB

**Test 5.3: Master Mute**
1. Set Master Gain = -60dB
2. Play pink noise
3. **Expected**: Complete silence

**Status**: [ ] Pass [ ] Fail

---

### TC-6: Link L/R Gain

**Objective**: Verify gain linking functionality

**Test 6.1: Link OFF**
1. Set Link L/R Gain = OFF
2. Change Left Gain to -6dB
3. **Expected**: Right Gain stays at 0dB

**Test 6.2: Link ON - Adjust Left**
1. Set Link L/R Gain = ON
2. Change Left Gain to -6dB
3. **Expected**: Right Gain also changes to -6dB

**Test 6.3: Link ON - Adjust Right**
1. Link L/R Gain = ON
2. Change Right Gain to -12dB
3. **Expected**: Left Gain also changes to -12dB

**Test 6.4: Link Toggle**
1. Set Left Gain = -3dB, Right Gain = -6dB
2. Toggle Link ON
3. Adjust Left Gain to -9dB
4. **Expected**: Both gains become -9dB

**Status**: [ ] Pass [ ] Fail

---

### TC-7: Parameter Smoothing

**Objective**: Verify no clicks/pops on parameter changes

**Test 7.1: Rapid Gain Changes**
1. Play pink noise
2. Rapidly change Left Gain between 0dB and -20dB
3. **Expected**: No audible clicks or pops

**Test 7.2: Rapid Pan Changes**
1. Play pink noise
2. Rapidly sweep Left Pan from -100 to +100
3. **Expected**: Smooth transition, no clicks

**Test 7.3: Master Gain Sweep**
1. Play music
2. Slowly fade Master Gain from 0dB to -60dB over 2 seconds
3. **Expected**: Smooth fade, no artifacts

**Status**: [ ] Pass [ ] Fail

---

### TC-8: Automation

**Objective**: Verify automation works correctly

**Steps**:
1. Record automation for Left Pan (sweep L100 → R100)
2. Record automation for Right Delay (0ms → 50ms → 0ms)
3. Playback with automation
4. **Expected**: Parameters follow automation curves smoothly

**Status**: [ ] Pass [ ] Fail

---

### TC-9: State Persistence

**Objective**: Verify parameter values are saved

**Steps**:
1. Set all parameters to non-default values:
   - Left Pan = -50
   - Left Gain = -3dB
   - Left Delay = 10ms
   - Right Pan = 50
   - Right Gain = -6dB
   - Right Delay = 20ms
   - Master Gain = -3dB
   - Link Gain = ON
2. Save project
3. Close DAW
4. Reopen project
5. **Expected**: All parameters retain their values

**Status**: [ ] Pass [ ] Fail

---

### TC-10: CPU Usage

**Objective**: Verify reasonable CPU usage

**Steps**:
1. Create 10 tracks with SimplePanner
2. Play back with all instances active
3. Monitor CPU usage in DAW
4. **Expected**: Total CPU usage < 10% (on modern CPU)

**Status**: [ ] Pass [ ] Fail

---

### TC-11: Sample Rate Changes

**Objective**: Verify plugin handles sample rate changes

**Steps**:
1. Set Right Delay = 10ms at 44.1kHz
2. Check actual delay (should be 441 samples)
3. Change project to 48kHz
4. **Expected**:
   - No crash
   - Delay remains 10ms (480 samples)

**Status**: [ ] Pass [ ] Fail

---

### TC-12: Multiple Instances

**Objective**: Verify multiple instances don't interfere

**Steps**:
1. Insert SimplePanner on Track 1
2. Insert SimplePanner on Track 2
3. Set different parameters on each
4. Play both tracks simultaneously
5. **Expected**: Each instance operates independently

**Status**: [ ] Pass [ ] Fail

---

## Test Results Summary

| Test Case | Status | Notes |
|-----------|--------|-------|
| TC-1: Default State | [ ] | |
| TC-2: Pan | [ ] | |
| TC-3: Gain | [ ] | |
| TC-4: Delay | [ ] | |
| TC-5: Master Gain | [ ] | |
| TC-6: Link Gain | [ ] | |
| TC-7: Smoothing | [ ] | |
| TC-8: Automation | [ ] | |
| TC-9: State | [ ] | |
| TC-10: CPU | [ ] | |
| TC-11: Sample Rate | [ ] | |
| TC-12: Multiple | [ ] | |

**Total**: ___/12 Passed

**Tested By**: ____________
**Date**: ____________
**DAW**: ____________
**Platform**: ____________
