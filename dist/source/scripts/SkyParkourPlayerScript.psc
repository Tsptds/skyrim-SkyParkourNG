Scriptname SkyParkourPlayerScript extends ReferenceAlias  


Event OnPlayerLoadGame()
	(GetOwningQuest() as SkyParkourQuestScript).Maintenance()
EndEvent

Event OnInit()
	(GetOwningQuest() as SkyParkourQuestScript).Maintenance()
endevent