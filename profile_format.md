Profile XML
===========

Persistent profiles use a `<profiles>` root element containing one or more `<profile>` elements. Temporary profiles use only one `<profile>` root element.

The `<profile>` elements contain device specific profile settings,a `<modes>` element containing several `<mode>` elements each with device specific settings, and a `<buttons>` element for button mapping. Available special actions are device specific.

Value format
------------

 - **Color**: 6 digit hexadecimal number: `rrggbb`.
 - **LED vector**: a series of `0` (off) and `1` (on) for each LED (*e.g.* `1100` for the two first LED on, and the two last off).


G9
--

Profile settings:
 - `color` (color): profile color.
 - `default_dpi` (integer): default DPI mode when resetting the mice or changing profile.
 - `default_dpi_bit7` (bool): ?
 - `report_rate` (integer): USB report rate in milliseconds: 1ms for 1000 Hz, 2ms for 500 Hz, ... 8ms for 125 Hz.
 - `unknown0` to `unknown5` (integer): ?

Mode settings:
 - `dpi` (integer): dpi for both directions.
 - `leds` (LED vector): state of the for LEDs for this mode.

Supported special actions: currently set to the same as G500 but actually unknown.


G9x/G500/G500s profiles
-----------------------

Profile settings:
 - `color` (color): profile color.
 - `angle` (integer): unknown meaning, default is 128. 
 - `angle_snapping` (boolean): enable angle correction.
 - `default_dpi` (integer): default DPI mode when resetting the mice or changing profile.
 - `lift_threshold` (integer): a value between -15 and 15 for changing the lift threshold. Default is 0. With a too low lift threshold, the mouse may be always “in the air” depending on the surface, use this setting with care (Logitech has reasons for hiding it).
 - `unknown` (integer): ?
 - `report_rate` (integer): USB report rate in milliseconds: 1ms for 1000 Hz, 2ms for 500 Hz, ... 8ms for 125 Hz.

Mode settings:
 - `dpi_x` (integer): horizontal DPI.
 - `dpi_y` (integer): vertical DPI, default to the same value as `dpi_x`.
 - `leds` (LED vector): state of the for LEDs for this mode.

Supported special actions:
 - *WheelLeft* for horizontal wheel left,
 - *WheelRight* for horizontal wheel right,
 - *ResolutionNext* for switching to the next resolution mode,
 - *ResolutionPrev* for switching to the previous resolution mode,
 - *ProfileNext* for switching to the next profile,
 - *ProfilePrev* for switching to the previous profile,
 - *ProfileSwitch0* for switch to the first profile,
 - *ProfileSwitch1* for switch to the second profile,
 - *ProfileSwitch2* for switch to the third profile,
 - *ProfileSwitch3* for switch to the fourth profile,
 - *ProfileSwitch4* for switch to the fifth profile,


G700/G700s profiles
-------------------

Profile settings:
 - `default_dpi` (integer): default DPI mode when resetting the mice or changing profile.
 - `angle` (integer): unknown meaning, default is 128. 
 - `angle_snapping` (boolean): enable angle correction.
 - `report_rate` (integer): USB report rate in milliseconds: 1ms for 1000 Hz, 2ms for 500 Hz, ... 8ms for 125 Hz.
 - `power_mode` (integer): 50 = *Energy saving*, 100 = *Normal gaming*, 200 = *Max gaming*.
 - `unknown0` to `unknown9` (integer): ?

Mode settings:
 - `dpi_x` (integer): horizontal DPI.
 - `dpi_y` (integer): vertical DPI, default to the same value as `dpi_x`.
 - `leds` (LED vector): state of the for LEDs for this mode.

Supported special actions:
 - *WheelLeft* for horizontal wheel left,
 - *WheelRight* for horizontal wheel right,
 - *BatteryLevel* for displaying battery level on the device LEDs,
 - *ResolutionNext* for switching to the next resolution mode,
 - *ResolutionCycleNext* for cycling forward through the resolution modes,
 - *ResolutionPrev* for switching to the previous resolution mode,
 - *ResolutionCyclePrev* for cycling backward through the resolution modes,
 - *ProfileNext* for switching to the next profile,
 - *ProfileCycleNext* for cycling forward through the profiles,
 - *ProfilePrev* for switching to the previous profile,
 - *ProfileCyclePrev* for cycling backward through the profiles,
 - *ProfileSwitch0* for switch to the first profile,
 - *ProfileSwitch1* for switch to the second profile,
 - *ProfileSwitch2* for switch to the third profile,
 - *ProfileSwitch3* for switch to the fourth profile,
 - *ProfileSwitch4* for switch to the fifth profile,


HID++ 2.0 or later, with profile format 1, 2, or 3
--------------------------------------------------

Profile settings:
 - `report_rate` (integer): USB report rate in milliseconds: 1ms for 1000 Hz, 2ms for 500 Hz, ... 8ms for 125 Hz.
 - `default_dpi` (integer): default DPI mode when resetting the mice or changing profile.
 - `color` (color): profile color.
 - `angle_snapping` (boolean): enable angle correction.
 - `revision` (integer): metadata used by LGS?
 - `name` (string): profile name.
 - `logo_effect` and `side_effect`: RGB LED effect for logo and DPI/side LEDs. It contains elements:
   - `type` (enum):
     - *Off*: LED is off,
     - *Constant*: LED has a constant color,
     - *Pulse*: brightness varies,
     - *Cycle*: cycles through the color spectrum,
   - `color` (color): used by *Constant* and *Pulse* effects,
   - `period` (integer): in milliseconds for *Pulse* and *Cycle* effects,
   - `brightness` (integer): from 0 to 100, for *Pulse* and *Cycle* effects.
 - `switched_dpi` (bool, if DPI-shift is supported): resolution mode index to switch to when pressing *ResolutionSwitch* button.
 - `power_mode`: unknown accepted values.

Mode settings:
 - `dpi` (integer): dpi for both directions.

Supported special actions:
 - *WheelLeft* for horizontal wheel left,
 - *WheelRight* for horizontal wheel right,
 - *ResolutionNext* for switching to the next resolution mode,
 - *ResolutionPrev* for switching to the previous resolution mode,
 - *ResolutionCycle* for cycling through the resolution modes,
 - *ResolutionDefault* for switching to the default resolution mode,
 - *ResolutionSwitch* for switching to the `switched_dpi` mode while pressed,
 - *ProfileNext* for switching to the next profiles,
 - *ProfilePrev* for switching to the previous profile,
 - *ProfileCycle* for cycling through the profiles,
 - *GShift* for using alternate buttons while pressed,
 - *BatteryLevel* for displaying battery level on the device LEDs,


Buttons
-------

Buttons use different element names depending on the type of mapping:
 - `<mouse-button>` contains the index of the mouse button to map (starting at 0).
 - `<key>` for mapping a keyboard key. A `modifiers` attribute may contains a ‘+’ separated list of modifier keys. Use HID usage code from the keyboard page or names from *src/libhidpp/misc/UsageStrings.cpp*.
 - `<special>` for special actions.
 - `<consumer-control>` for a *consumer control* key. Use HID usage code from the keyboard page or names from *src/libhidpp/misc/UsageStrings.cpp*.
 - `<macro>` has a `type` attribute for several macro types:
   - *simple* (default), the `<macro>` element contains a macro with only simple instruction. This type does not allow loop or conditional wait.
   - *loop* defines complex loops. It can contains three elements `<pre>`, `<loop>`, and `<post>` each containing simple macro instructions respectively corresponding to the pre-loop instructions, looping instruction and post-loop instruction. Any of these child elements can be omitted. *loop* type `<macro>` elements can have a `loop-delay` attribute (default value is 0) for delaying the start of the loop: the loop is not executed if the button is released before that time, a 0 *loop-delay* will always execute the loop instructions at least once.
   - *advanced* can be used for defining more complex macros, it accept advanced macro instructions. The macro must end with an *End* instruction.
 - `<disabled>` for disabling the button.

G-Shift devices will have twice the number of buttons. The first buttons are used in normal mode, the last buttons are used in shifted mode (when the *GShift* special action is pressed). The alternate action for the G-shift button is not used.


Macro language
==============

Macro instructions are separated by semi-colons (;). A instruction may be prepended by a label if it is the destination of a jump, the label is a identifier followed by a colon: `label_name:`.

Values that are empty or contain white spaces or semi-colons must surrounded by double-quotes.

Macro instruction are split in two categories. Simple instructions are input events (mouse buttons, keyboard keys, ...) and delays; advanced instructions are all other instructions: jumps, waiting and looping instructions, ...


Simple instructions
-------------------

 - **NoOp** does nothing. It is used internally and should be hidden when printing macros but is accepted as a simple instruction.
 - **KeyPress** *key* and **KeyRelease** *key* press or release *key* (see *src/libhidpp/misc/UsageStrings.cpp* for key names or use HID keyboard page usage codes).
 - **ModifiersPress** *modifier+...* and **ModifiersRelease** *modifier+...* press or release the ‘+’-separated list of modifier keys.
 - **ModifiersKeyPress** *modifier+...* *key* and **ModifiersKeyRelease** *modifier+...* *key* press or release both modifiers and key.
 - **MouseWheel** *value* or **MouseHWheel** *value* move the mouse wheel (or horizontal wheel) by *value* (a positive or negative integer).
 - **MouseButtonPress** *button+...* and **MouseButtonRelease** *button+...* press or release the ‘+’-separated list of buttons. First button is *0*.
 - **ConsumerControl** *control* sets the current pressed consumer control to *control* (see *src/libhidpp/misc/UsageStrings.cpp* for names or use HID consumer control page usage codes). Set to *0* for releasing the current control.
 - **ConsumerControlPress** *control* or **ConsumerControlRelease** *control* press or release the HID consumer control usage.
 - **Delay** *duration* waits for *duration* milliseconds.
 - **ShortDelay** *duration* waits for *duration* milliseconds. **ShortDelay** take less memory space than **Delay** but is limited to some predefined values between 8 ms and 1892 ms with steps from 4 ms for shorter delays to 32 ms for longer ones.
 - **MousePointer** *x* *y* moves the mouse pointer of *(x, y)* units.


Advanced instructions
---------------------

These instructions are limited to advanced macros.

 - **WaitRelease** waits until the button is released.
 - **RepeatUntilRelease** repeats the macro from the beginning if the button is still pressed.
 - **Repeat** repeats the macro from the beginning. This seems to create infinite loops.
 - **Jump** *label-name* jumps to the instruction following the *label-name* label.
 - **JumpIfPressed** *label-name* jumps to *label-name* if the button is still pressed, go to the next instruction otherwise.
 - **JumpIfReleased** *delay* *label-name* jumps to *label-name* if the button is released before *delay* milliseconds or go to the next instruction if it times out.
 - **End** terminate the execution of the current macro. Any macro must end with this instruction or it will execute whatever is next in the memory as a macro.

Device support
--------------

### HID++ 1.0

**MouseHWheel**, **ConsumerControlPress** and **ConsumerControlRelease** are unsupported.

**ModifiersKeyPress** and **ModifiersKeyRelease** are unsupported by hardware but are replaced with a combination of modifiers and key instructions.

### HID++ 2.0 or later with macro format 1

**ConsumerControl** and **ShortDelay** are unsupported.

It also lacks support for **JumpIfPressed** and **JumpIfReleased**, thus not all loops are supported: a `type="loop"` macro must either have a only a `<loop>` element (`<pre>` and `<post>` must be absent or empty): a simple loop without prologue or epilogue, or only `<pre>` and `<post>` elements (`<loop>` element must be absent or empty): not actually a loop but a macro that wait the button release to play the epilogue.

**ModifersPress**, **ModifiersRelease**, **KeyPress** and **KeyRelease** are unsupported by hardware but are replaced with *ModifiersKey* instructions.

