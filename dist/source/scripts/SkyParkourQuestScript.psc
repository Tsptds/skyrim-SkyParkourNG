Scriptname SkyParkourQuestScript extends Quest  

Event OnInit()
EndEvent

function Maintenance()
	;Debug.MessageBox("Maintenance")

	UnregisterForAllKeys()
		

	SkyParkourPapyrus.RegisterCustomParkourKey(CustomKey)
	SkyParkourPapyrus.RegisterPresetParkourKey(PresetKey)


	SkyParkourPapyrus.RegisterParkourSettings(UsePresetKey, EnableMod, EnableSmartParkour)

	SkyParkourPapyrus.RegisterParkourDelay(ButtonDelay)

	SkyParkourPapyrus.RegisterReferences(indicatorRef_Blue, indicatorRef_Red)

	SkyParkourPapyrus.RegisterStaminaDamage(ConsumeStamina, StaminaBlocksParkour, StaminaDamage)
	
endFunction

ObjectReference Property indicatorRef_Blue Auto
ObjectReference Property indicatorRef_Red Auto

Bool Property EnableMod Auto
Bool Property EnableSmartParkour Auto

Bool Property UsePresetKey Auto
Int Property CustomKey Auto
Int Property PresetKey Auto

Bool Property ConsumeStamina auto
Bool Property StaminaBlocksParkour auto
float Property StaminaDamage auto

float Property ButtonDelay auto