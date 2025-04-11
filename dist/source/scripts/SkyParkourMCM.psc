Scriptname SkyParkourMCM extends Ski_ConfigBase

int customKeyOption
int usePresetKeyOption
int enableModOption
int smartParkourOption
int useStaminaOption
int staminaBlockOption
int staminaSlider
int parkourDelaySlider

int presetKeyOption                 ; NEW - option box for preset key
string[] presetKeyList              ; NEW - list for preset key choices

int function GetVersion()
	return 2 
endFunction

event OnOptionHighlight(int a_option)
	{Called when the user highlights an option}
	
	if (a_option == customKeyOption)
		SetInfoText("Change the Custom Parkour Key")
	elseIf (a_option == usePresetKeyOption)
		SetInfoText("Use a key from game control map. Key will match automatically for keyboard / gamepad. Custom key doesn't support this feature.")
	elseIf (a_option == enableModOption)
		SetInfoText("Turn mod ON/OFF")
	elseIf (a_option == smartParkourOption)
		SetInfoText("Disables Vaulting if you're standing still. Disables High Climbing while moving, so you won't be grabbing roofs unless standing still. Also disables Climb Failing while moving if 'Stamina is Required' option is enabled.")
	elseIf (a_option == useStaminaOption)
		SetInfoText("Parkour Actions Consume Stamina")
	elseIf (a_option == staminaBlockOption)
		SetInfoText("In case of insufficient stamina, the indicator will be replaced with a red one. Climbing actions will fail and play a failed grab animation instead (vault and low grab will work regardless). Disabling this will turn stamina system into more of a cosmetic feature.")
	elseIf (a_option == staminaSlider)
		SetInfoText("Base consumption value (x). Formula is x + (Equipment Weight) * 0.2. The more items you equip, the higher the cost.")
	elseIf (a_option == parkourDelaySlider)
		SetInfoText("Adds a delay to the initial button press, if you don't want instant parkour. Only for the initial press, will not delay if you're holding down the button.")

	endIf
endEvent

Event OnPageReset(string page)


	SetCursorFillMode(LEFT_TO_RIGHT)
	SetCursorPosition(0)

	; Parkour Rules	
	AddHeaderOption("Parkour Settings")
	AddEmptyOption()
	enableModOption = AddToggleOption("Enable Mod", SkyParkour.EnableMod)
	AddEmptyOption()
	smartParkourOption = AddToggleOption("Smart Parkour", SkyParkour.EnableSmartParkour)

	AddHeaderOption("Input Settings")
	AddEmptyOption()
	usePresetKeyOption = AddToggleOption("Preset Key For Parkour", SkyParkour.usePresetKey)
	
		int flagPreset
		int flagCustom
		if (SkyParkour.usePresetKey)
			flagCustom = OPTION_FLAG_DISABLED
			flagPreset = OPTION_FLAG_NONE
		else
			flagCustom = OPTION_FLAG_NONE
			flagPreset = OPTION_FLAG_DISABLED
		endIf
	AddEmptyOption()

	presetKeyList = new string[3]
    	presetKeyList[0] = "Jump Key"
   	presetKeyList[1] = "Sprint Key"
    	presetKeyList[2] = "Activate Key"
	presetKeyOption = AddMenuOption("Preset Key", presetKeyList[SkyParkour.PresetKey], flagPreset)

	AddEmptyOption()
	customKeyOption = AddKeyMapOption("Custom Key", SkyParkour.customKey, flagCustom)

	;Climb Delay
	AddEmptyOption()
	parkourDelaySlider = AddSliderOption("Initial Press Delay", SkyParkour.ButtonDelay, "{1}s")

	;Stamina consumption
	AddHeaderOption("Stamina System")
		
	AddEmptyOption()
	useStaminaOption = AddToggleOption("Enable Stamina System", SkyParkour.ConsumeStamina)
	AddEmptyOption()
	staminaBlockOption = AddToggleOption("Stamina is Required", SkyParkour.StaminaBlocksParkour)
	AddEmptyOption()
	staminaSlider = AddSliderOption("Stamina Consumption", SkyParkour.StaminaDamage)

EndEvent

event OnOptionKeyMapChange(int option, int keyCode, string conflictControl, string conflictName)
	if (option == customKeyOption && SkyParkour.usePresetKey == false)
		bool continue = true
		if (conflictControl != "")
			string msg
			if (conflictName != "")
				msg = "This key is already mapped to:\n\"" + conflictControl + "\"\n(" + conflictName + ")\n\nAre you sure you want to continue?"
			else
				msg = "This key is already mapped to:\n\"" + conflictControl + "\"\n\nAre you sure you want to continue?"
			endIf

			continue = ShowMessage(msg, true, "$Yes", "$No")
		endIf

		if (continue)
			SkyParkour.customKey = keyCode
			SkyParkourPapyrus.RegisterCustomParkourKey(SkyParkour.customKey)
			SetKeyMapOptionValue(customKeyOption, SkyParkour.customKey)
		endIf

	endIf
endEvent

event OnOptionSelect(int option)
	if (option == usePresetKeyOption)
		SkyParkour.usePresetKey = !SkyParkour.usePresetKey
		
		if SkyParkour.usePresetKey == false
			SkyParkourPapyrus.RegisterCustomParkourKey(SkyParkour.customKey)
		else
			SkyParkourPapyrus.RegisterPresetParkourKey(SkyParkour.PresetKey)
		endif
		SetToggleOptionValue(option, SkyParkour.usePresetKey)
		
		ForcePageReset()
	
	elseif (option == enableModOption)
		SkyParkour.EnableMod = !SkyParkour.EnableMod
		SetToggleOptionValue(option, SkyParkour.EnableMod)

	elseif (option == smartParkourOption)
		SkyParkour.EnableSmartParkour = !SkyParkour.EnableSmartParkour
		SetToggleOptionValue(option, SkyParkour.EnableSmartParkour)

	elseif (option == staminaBlockOption)
		SkyParkour.StaminaBlocksParkour = !SkyParkour.StaminaBlocksParkour
		SetToggleOptionValue(option, SkyParkour.StaminaBlocksParkour)
		SkyParkourPapyrus.RegisterStaminaDamage(SkyParkour.ConsumeStamina, SkyParkour.StaminaBlocksParkour, SkyParkour.StaminaDamage)

	elseif (option == useStaminaOption)
		SkyParkour.ConsumeStamina = !SkyParkour.ConsumeStamina
		SetToggleOptionValue(option, SkyParkour.ConsumeStamina)
		SkyParkourPapyrus.RegisterStaminaDamage(SkyParkour.ConsumeStamina, SkyParkour.StaminaBlocksParkour, SkyParkour.StaminaDamage)

		if !SkyParkour.ConsumeStamina
			SetOptionFlags(staminaBlockOption, OPTION_FLAG_DISABLED)
			SetOptionFlags(staminaSlider, OPTION_FLAG_DISABLED)
		else 
			SetOptionFlags(staminaBlockOption, OPTION_FLAG_NONE)
			SetOptionFlags(staminaSlider, OPTION_FLAG_NONE)
		endif

	endIf

	SkyParkourPapyrus.RegisterParkourSettings(SkyParkour.usePresetKey, SkyParkour.EnableMod, SkyParkour.EnableSmartParkour)
endEvent

event OnOptionSliderOpen(int a_option)
	{Called when the user selects a slider option}

	if (a_option == staminaSlider)
		SetSliderDialogStartValue(SkyParkour.StaminaDamage)
		SetSliderDialogDefaultValue(20)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(1)
	endIf

	if (a_option == parkourDelaySlider)
		SetSliderDialogStartValue(SkyParkour.ButtonDelay)
		SetSliderDialogDefaultValue(0.0)
		SetSliderDialogRange(0.0, 0.5)
		SetSliderDialogInterval(0.1)
	endif
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	{Called when the user accepts a new slider value}
		
	if (a_option == staminaSlider)
		SkyParkour.StaminaDamage = a_value
		SetSliderOptionValue(a_option, a_value)
		SkyParkourPapyrus.RegisterStaminaDamage(SkyParkour.ConsumeStamina, SkyParkour.StaminaBlocksParkour, a_value)
	endIf

	if (a_option == parkourDelaySlider)
		SkyParkour.ButtonDelay = a_value
		SetSliderOptionValue(a_option, a_value, "{1}s")
		SkyParkourPapyrus.RegisterParkourDelay(a_value)
	endif
endEvent

event OnOptionMenuOpen(int a_option)
	if (a_option == presetKeyOption)
        ; Set the current index based on the current CK property value.
        ; SkyParkour.PresetKey should be 0 (Jump Key), 1 (Sprint Key) or 2 (Activate Key)
        SetMenuDialogStartIndex(SkyParkour.PresetKey)
        SetMenuDialogDefaultIndex(0)
        SetMenuDialogOptions(presetKeyList)
    endIf
endEvent

; --- New event to handle the menu option for preset key ---
event OnOptionMenuAccept(int a_option, int a_index)
    if (a_option == presetKeyOption)
        SkyParkour.PresetKey = a_index  ; 0 = Jump Key, 1 = Sprint Key, 2 = Activate Key
        SetMenuOptionValue(presetKeyOption, presetKeyList[a_index])
    endIf
	SkyParkourPapyrus.RegisterPresetParkourKey(SkyParkour.PresetKey)
endEvent


SkyParkourQuestScript Property SkyParkour Auto
