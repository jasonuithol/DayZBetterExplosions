// Add-on: Big Machines' bulldozer, excavator and combine harvester go up (BetterExplosions.Explode)
// into the vanilla wreck each was restored from, flinging burnt parts off - the crawlers' track
// sprockets and engine bits, the combine's wheels and its header.
// Server and client (-mod), after BetterExplosions and BigMachines: the flying pieces are our
// own classes, built from Big Machines' (and vanilla's) part models.
class CfgPatches
{
	class BetterExplosions_BigMachines
	{
		units[] = {"BetterExplosions_BMDozerRoller", "BetterExplosions_BMExcRoller", "BetterExplosions_BMCombineWheelFront", "BetterExplosions_BMCombineWheelRear", "BetterExplosions_BMCombineHeader", "BetterExplosions_BMEngineRadiator", "BetterExplosions_BMEngineExhaust", "BetterExplosions_BMEngineHoses", "BetterExplosions_BMEngineControlUnit", "BetterExplosions_BMEngineBelt"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"BetterExplosions", "BigMachines"};
	};
};

class CfgMods
{
	class BetterExplosions_BigMachines
	{
		dir = "BetterExplosions_BigMachines";
		name = "Better Explosions - Big Machines";
		type = "mod";
		dependencies[] = {"World", "Mission"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"BetterExplosions_BigMachines/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"BetterExplosions_BigMachines/Scripts/5_Mission"};
			};
		};
	};
};

// Flung off a machine that goes up (BetterExplosionsDebris): models only, no physics of their
// own. Gone at restart. The combine's header is the one lying beside vanilla's combine wrecks;
// the engine bits are vanilla's (unused) truck parts, burnt.
class CfgVehicles
{
	class HouseNoDestruct;
	class BetterExplosions_BMDozerRoller: HouseNoDestruct
	{
		scope = 1;
		model = "\BigMachines\bulldozer\proxy\bm_dozer_roller.p3d";
	};
	class BetterExplosions_BMExcRoller: HouseNoDestruct
	{
		scope = 1;
		model = "\BigMachines\excavator\proxy\bm_exc_roller.p3d";
	};
	class BetterExplosions_BMCombineWheelFront: HouseNoDestruct
	{
		scope = 1;
		model = "\BigMachines\combine\proxy\bm_combine_wheel_front.p3d";
	};
	class BetterExplosions_BMCombineWheelRear: HouseNoDestruct
	{
		scope = 1;
		model = "\BigMachines\combine\proxy\bm_combine_wheel_rear.p3d";
	};
	class BetterExplosions_BMCombineHeader: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\structures\Wrecks\Vehicles\Proxy\Wreck_Harvester_Head.p3d";
	};
	class BetterExplosions_BMEngineRadiator: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\parts\radiator_truck.p3d";
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsMaterials[] = {"DZ\vehicles\parts\data\radiator_truck_destruct.rvmat"};
	};
	class BetterExplosions_BMEngineExhaust: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\parts\exhaust_truck.p3d";
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsMaterials[] = {"DZ\vehicles\parts\data\exhaust_v3s_destruct.rvmat"};
	};
	class BetterExplosions_BMEngineHoses: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\parts\hydraulic_hoses.p3d";
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsMaterials[] = {"DZ\vehicles\parts\data\hydraulic_hoses_destruct.rvmat"};
	};
	class BetterExplosions_BMEngineControlUnit: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\parts\control_unit.p3d";
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsMaterials[] = {"DZ\vehicles\parts\data\control_unit_destruct.rvmat"};
	};
	class BetterExplosions_BMEngineBelt: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\parts\belt.p3d";
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsMaterials[] = {"DZ\vehicles\parts\data\belt_destruct.rvmat"};
	};
};
