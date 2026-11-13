// The Expansion vehicles that have a vanilla wreck of the same model. The vehicles are named
// as strings, so nothing here needs Expansion's scripts: without Expansion they never match.
// turn is how far the wreck's model is turned from the vehicle's (the vanilla wreck props
// all face backwards: 180, checked in game). Then the burnt bits flung off, in order: off one
// side, the front, the other side, the back, anywhere (true: a wheel or door, standing on
// edge in its model, so it comes down on its side).
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		BetterExplosionsWreck.Add("ExpansionTractor", "Land_Wreck_Tractor", 180);
		BetterExplosionsWreck.Fling("ExpansionTractor", "BetterExplosions_ExpTractorWheel", true);
		BetterExplosionsWreck.Fling("ExpansionTractor", "BetterExplosions_ExpTractorFrontWheel", true);
		BetterExplosionsWreck.Fling("ExpansionTractor", "BetterExplosions_ExpTractorDoor", true);

		BetterExplosionsWreck.Add("ExpansionBus", "Land_Wreck_Ikarus", 180);
		BetterExplosionsWreck.Fling("ExpansionBus", "BetterExplosions_ExpBusWheel", true);
		BetterExplosionsWreck.Fling("ExpansionBus", "BetterExplosions_ExpBusDoorsLeft", true);
		BetterExplosionsWreck.Fling("ExpansionBus", "BetterExplosions_ExpBusRearWheel", true);
		BetterExplosionsWreck.Fling("ExpansionBus", "BetterExplosions_ExpBusEngineCover");
		BetterExplosionsWreck.Fling("ExpansionBus", "BetterExplosions_ExpBusDoorsRight", true);
		BetterExplosionsWreck.Fling("ExpansionBus", "BetterExplosions_ExpBusWheel", true);

		BetterExplosionsWreck.Add("ExpansionUAZBase", "Land_Wreck_Uaz", 180);
		BetterExplosionsWreck.Fling("ExpansionUAZBase", "BetterExplosions_ExpUAZWheel", true);
		BetterExplosionsWreck.Fling("ExpansionUAZBase", "BetterExplosions_ExpUAZHood");
		BetterExplosionsWreck.Fling("ExpansionUAZBase", "BetterExplosions_ExpUAZWheel", true);
		BetterExplosionsWreck.Fling("ExpansionUAZBase", "BetterExplosions_ExpUAZDoor", true);

		BetterExplosionsWreck.Add("Expansion_Landrover_Base", "Land_Wreck_LR_TK", 180);
		BetterExplosionsWreck.Fling("Expansion_Landrover_Base", "BetterExplosions_ExpLandroverWheel", true);
		BetterExplosionsWreck.Fling("Expansion_Landrover_Base", "BetterExplosions_ExpLandroverHood");
		BetterExplosionsWreck.Fling("Expansion_Landrover_Base", "BetterExplosions_ExpLandroverWheel", true);
		BetterExplosionsWreck.Fling("Expansion_Landrover_Base", "BetterExplosions_ExpLandroverDoor", true);

		BetterExplosionsWreck.Add("ExpansionUh1h", "Wreck_UH1Y", 180);
		BetterExplosionsWreck.Fling("ExpansionUh1h", "BetterExplosions_ExpUh1hDoor", true);
		BetterExplosionsWreck.Fling("ExpansionUh1h", "BetterExplosions_RotorBlade");
		BetterExplosionsWreck.Fling("ExpansionUh1h", "BetterExplosions_ExpUh1hDoor2", true);
		BetterExplosionsWreck.Fling("ExpansionUh1h", "BetterExplosions_RotorBlade");
	}
}
