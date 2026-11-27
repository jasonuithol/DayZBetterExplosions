// Each Big Machines machine leaves the vanilla wreck it was restored from. turn is how far the wreck's
// model is turned from the machine's: the bulldozer wreck's blade is at its back (Big Machines
// turns its dozers 180 from the wreck too), the digger and combine wrecks face the machine's
// way. Then the burnt bits flung off, in order: off one side, the front, the other side, the
// back, anywhere (true: a wheel, standing on edge in its model, so it comes down on its side).
// The tower crane has no fuel, so it never goes up.
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		// a new wreck on the spot of the one Big Machines keeps sunk is moved off it
		BetterExplosions.Event_OnExploded.Insert(BM_Wrecks.BE_OnExploded);

		BetterExplosionsWreck.Add("BM_Bulldozer", "StaticObj_Wreck_Bulldozer", 180);
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMDozerRoller", true);
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMEngineRadiator");	// the engine's at the front
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMDozerRoller", true);
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMEngineExhaust");
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMEngineHoses");
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMEngineBelt");
		BetterExplosionsWreck.Fling("BM_Bulldozer", "BetterExplosions_BMEngineControlUnit");

		BetterExplosionsWreck.Add("BM_Excavator", "StaticObj_Wreck_Digger", 0);
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMExcRoller", true);
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMEngineHoses");		// the arm's hydraulics
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMExcRoller", true);
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMEngineRadiator");	// the engine's at the back
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMEngineExhaust");
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMEngineBelt");
		BetterExplosionsWreck.Fling("BM_Excavator", "BetterExplosions_BMEngineControlUnit");

		BetterExplosionsWreck.Add("BM_Combine", "Land_Wreck_Harverster", 0);
		BetterExplosionsWreck.Fling("BM_Combine", "BetterExplosions_BMCombineWheelFront", true);
		BetterExplosionsWreck.Fling("BM_Combine", "BetterExplosions_BMCombineHeader");
		BetterExplosionsWreck.Fling("BM_Combine", "BetterExplosions_BMCombineWheelFront", true);
		BetterExplosionsWreck.Fling("BM_Combine", "BetterExplosions_BMCombineWheelRear", true);
	}
}
