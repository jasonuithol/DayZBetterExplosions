class CfgPatches
{
	class BetterExplosions
	{
		units[] = {"BetterExplosionsFireball", "BetterExplosions_BoatHull", "BetterExplosions_BoatEngine", "BetterExplosions_HatchbackWheel", "BetterExplosions_HatchbackHood", "BetterExplosions_Hatchback_02_Wheel", "BetterExplosions_Hatchback_02_Hood", "BetterExplosions_CivSedanWheel", "BetterExplosions_CivSedanHood", "BetterExplosions_Sedan_02_Wheel", "BetterExplosions_Sedan_02_Hood", "BetterExplosions_Offroad_02_Wheel", "BetterExplosions_Offroad_02_Hood", "BetterExplosions_Truck_01_Wheel", "BetterExplosions_Truck_01_Hood", "BetterExplosions_Motorbike_01_Wheel_1", "BetterExplosions_Motorbike_01_Wheel_2", "BetterExplosions_Motorbike_01_Fender", "BetterExplosions_Motorbike_01_Exhaust", "BetterExplosions_Motorbike_02_Wheel_1", "BetterExplosions_Motorbike_02_Wheel_2", "BetterExplosions_Motorbike_02_FenderFront", "BetterExplosions_Motorbike_02_FenderRear", "BetterExplosions_Motorbike_02_Exhaust", "BetterExplosions_Motorbike_02_Windshield", "BetterExplosions_Motorbike_02_ShieldLeft", "BetterExplosions_Motorbike_02_ShieldRight", "BetterExplosions_RotorBlade"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Vehicles_Wheeled", "DZ_Vehicles_Singletrack", "DZ_Vehicles_Water"};
	};
};

class CfgMods
{
	class BetterExplosions
	{
		dir = "BetterExplosions";
		name = "Better Explosions";
		type = "mod";
		dependencies[] = {"World", "Mission"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"BetterExplosions/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"BetterExplosions/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class HouseNoDestruct;

	//! A vehicle going up: the fireball (particles on the clients), invisible itself
	class BetterExplosionsFireball: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\data\lightpoint.p3d";
	};

	//! What's left of a boat that's gone up, sinking to the bottom (BetterExplosionsDebris.c). Not
	//! stored: gone at restart
	class BetterExplosions_BoatHull: HouseNoDestruct
	{
		scope = 1;
		model = "\BetterExplosions\data\boat_hull.p3d";
	};
	class BetterExplosions_BoatEngine: HouseNoDestruct
	{
		scope = 1;
		model = "\BetterExplosions\data\boat_engine.p3d";
	};

	//! Bits blown off a car going up (BetterExplosionsWreck, BetterExplosionsDebris.c): its ruined wheel, and its
	//! hood with the paint burnt off. Not stored: gone at restart
	class BetterExplosions_HatchbackWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\OffroadHatchback\proxy\nivaWheel_destroyed.p3d";
	};
	class BetterExplosions_HatchbackHood: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\OffroadHatchback\proxy\nivaDoors_Hood.p3d";
		hiddenSelections[] = {"dmgZone_doors"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\wheeled\offroadhatchback\data\green\niva_hood_destruct.rvmat"};
	};
	class BetterExplosions_Hatchback_02_Wheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Hatchback_02\proxy\Hatchback_02_Wheel_ruined.p3d";
	};
	class BetterExplosions_Hatchback_02_Hood: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Hatchback_02\proxy\Hatchback_02_Hood.p3d";
		hiddenSelections[] = {"dmgZone_doors"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\wheeled\hatchback_02\data\red\hatchback_02_hood_destruct.rvmat"};
	};
	class BetterExplosions_CivSedanWheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\civiliansedan\proxy\sedanwheel_destroyed.p3d";
	};
	class BetterExplosions_CivSedanHood: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\civilianSedan\proxy\sedanhood.p3d";
		hiddenSelections[] = {"dmgZone_doors"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\wheeled\civiliansedan\data\white\civiliansedan_destruct.rvmat"};
	};
	class BetterExplosions_Sedan_02_Wheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Sedan_02\proxy\Sedan_02_Wheel_ruined.p3d";
	};
	class BetterExplosions_Sedan_02_Hood: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Sedan_02\proxy\Sedan_02_Hood.p3d";
		hiddenSelections[] = {"dmgZone_doors"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\wheeled\sedan_02\data\yellow\sedan_02_hood_destruct.rvmat"};
	};
	class BetterExplosions_Offroad_02_Wheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Offroad_02\proxy\Offroad_02_Wheel_destroyed.p3d";
	};
	class BetterExplosions_Offroad_02_Hood: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Offroad_02\proxy\Offroad_02_Hood.p3d";
		hiddenSelections[] = {"dmgZone_doors"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\wheeled\offroad_02\data\offroad_02_base_multi_destruct.rvmat"};
	};
	class BetterExplosions_Truck_01_Wheel: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Truck_01\proxy\Truck_01_Wheel_ruined.p3d";
	};
	class BetterExplosions_Truck_01_Hood: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\wheeled\Truck_01\proxy\Truck_01_Hood.p3d";
		hiddenSelections[] = {"dmgZone_doors", "camo_door"};
		hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(0.08,0.07,0.06,1.0,co)", "#(argb,8,8,3)color(0.08,0.07,0.06,1.0,co)"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\wheeled\Truck_01\data\Truck_01_hood_destruct.rvmat", "dz\vehicles\wheeled\Truck_01\data\Truck_01_hood_destruct.rvmat"};
	};

	//! A motorbike flying apart (BetterExplosionsDebris.Scatter): its ruined wheels, fenders, exhaust and
	//! shields (burnt black). Not stored: gone at restart
	class BetterExplosions_Motorbike_01_Wheel_1: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_01\proxy\Motorbike_01_wheel_1_ruined.p3d";
	};
	class BetterExplosions_Motorbike_01_Wheel_2: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_01\proxy\Motorbike_01_wheel_2_ruined.p3d";
	};
	class BetterExplosions_Motorbike_01_Fender: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_01\proxy\Motorbike_01_fender.p3d";
	};
	class BetterExplosions_Motorbike_01_Exhaust: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_01\proxy\Motorbike_01_exhaust.p3d";
	};
	class BetterExplosions_Motorbike_02_Wheel_1: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_wheel_1_ruined.p3d";
	};
	class BetterExplosions_Motorbike_02_Wheel_2: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_wheel_2_ruined.p3d";
	};
	class BetterExplosions_Motorbike_02_FenderFront: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_fenderfront.p3d";
	};
	class BetterExplosions_Motorbike_02_FenderRear: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_fenderrear.p3d";
	};
	class BetterExplosions_Motorbike_02_Exhaust: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_exhaust.p3d";
	};
	class BetterExplosions_Motorbike_02_Windshield: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_windshield.p3d";
		hiddenSelections[] = {"camo"};
		hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(0.08,0.07,0.06,1.0,co)"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\singletrack\motorbike_02\data\motorbike_02_windshield_destruct.rvmat"};
	};
	class BetterExplosions_Motorbike_02_ShieldLeft: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_shieldleft.p3d";
		hiddenSelections[] = {"camo"};
		hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(0.08,0.07,0.06,1.0,co)"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\singletrack\motorbike_02\data\Motorbike_02_shieldleft_destruct.rvmat"};
	};
	class BetterExplosions_Motorbike_02_ShieldRight: HouseNoDestruct
	{
		scope = 1;
		model = "\DZ\vehicles\singletrack\Motorbike_02\proxy\Motorbike_02_shieldright.p3d";
		hiddenSelections[] = {"camo"};
		hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(0.08,0.07,0.06,1.0,co)"};
		hiddenSelectionsMaterials[] = {"dz\vehicles\singletrack\motorbike_02\data\Motorbike_02_shieldright_destruct.rvmat"};
	};

	//! A helicopter's main rotor blade, burnt and snapped off near the hub, for add-ons to
	//! fling (BetterExplosions_Expansion's UH-1H)
	class BetterExplosions_RotorBlade: HouseNoDestruct
	{
		scope = 1;
		model = "\BetterExplosions\data\rotor_blade.p3d";
	};
};
