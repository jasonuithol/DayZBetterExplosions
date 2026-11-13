// Add-on: DayZ Expansion's vehicles that go up (BetterExplosions.Explode) leave the matching
// vanilla wreck, like the vanilla cars do, and fling burnt wheels, doors and hoods off.
// Server and client (-mod), after BetterExplosions and DayZ-Expansion-Vehicles: the flying
// pieces are our own classes, built from Expansion's (and vanilla's) part models.
class CfgPatches
{
	class BetterExplosions_Expansion
	{
		units[] = {"BetterExplosions_ExpTractorWheel", "BetterExplosions_ExpTractorFrontWheel", "BetterExplosions_ExpTractorDoor", "BetterExplosions_ExpBusWheel", "BetterExplosions_ExpBusRearWheel", "BetterExplosions_ExpBusDoorsLeft", "BetterExplosions_ExpBusDoorsRight", "BetterExplosions_ExpBusEngineCover", "BetterExplosions_ExpUAZWheel", "BetterExplosions_ExpUAZHood", "BetterExplosions_ExpUAZDoor", "BetterExplosions_ExpLandroverWheel", "BetterExplosions_ExpLandroverHood", "BetterExplosions_ExpLandroverDoor", "BetterExplosions_ExpUh1hDoor", "BetterExplosions_ExpUh1hDoor2"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"BetterExplosions"};
	};
};

class CfgMods
{
	class BetterExplosions_Expansion
	{
		dir = "BetterExplosions_Expansion";
		name = "Better Explosions - Expansion Vehicles";
		type = "mod";
		dependencies[] = {"World", "Mission"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"BetterExplosions_Expansion/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"BetterExplosions_Expansion/Scripts/5_Mission"};
			};
		};
	};
};

// Flung off a vehicle that goes up (BetterExplosionsDebris): models only, no physics of
// their own. Gone at restart. Expansion's part models, the bus's from vanilla's unreleased
// Ikarus (the same bus). The burnt rotor blade is BetterExplosions' own.
class CfgVehicles
{
	class HouseNoDestruct;
	class BetterExplosions_ExpTractorWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\Tractor\proxy\tractor2_back_wheel_destroyed.p3d";
	};
	class BetterExplosions_ExpTractorFrontWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\Tractor\proxy\tractor2_front_wheel_destroyed.p3d";
	};
	class BetterExplosions_ExpTractorDoor: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\Tractor\proxy\ExpansionTractorDoorsDriver.p3d";
	};
	class BetterExplosions_ExpBusWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\TransitBus\proxy\ikarusWheel_destroyed.p3d";
	};
	class BetterExplosions_ExpBusRearWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\TransitBus\proxy\ikarusWheel_rear_destroyed.p3d";
	};
	class BetterExplosions_ExpBusDoorsLeft: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\TransitBus\proxy\IkarusDoors_Left.p3d";
	};
	class BetterExplosions_ExpBusDoorsRight: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\TransitBus\proxy\IkarusDoors_Right.p3d";
	};
	class BetterExplosions_ExpBusEngineCover: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\TransitBus\proxy\ikarusHood.p3d";
	};
	class BetterExplosions_ExpUAZWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\UAZ\proxy\UAZWheel_Destroyed.p3d";
	};
	class BetterExplosions_ExpUAZHood: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\UAZ\proxy\UAZHoodDoor_Wreck.p3d";
	};
	class BetterExplosions_ExpUAZDoor: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\UAZ\proxy\uazdriverdoor.p3d";
	};
	class BetterExplosions_ExpLandroverWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\LandRover\proxy\landrover_wheel_destroyed.p3d";
	};
	class BetterExplosions_ExpLandroverHood: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\LandRover\proxy\expansion_landrover_hood.p3d";
		hiddenSelections[] = {"body_outdoor"};
		hiddenSelectionsMaterials[] = {"DayZExpansion\Vehicles\Ground\LandRover\data\damage\lr_body_outdoor_destruct.rvmat"};
	};
	class BetterExplosions_ExpLandroverDoor: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Ground\LandRover\proxy\expansion_landrover_driverdoor.p3d";
	};
	class BetterExplosions_ExpUh1hDoor: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Air\Uh1h\proxy\uh1hdoor_1_1.p3d";
	};
	class BetterExplosions_ExpUh1hDoor2: HouseNoDestruct
	{
		scope = 1;
		model = "\DayZExpansion\Vehicles\Air\Uh1h\proxy\uh1hdoor_2_1.p3d";
	};
};
