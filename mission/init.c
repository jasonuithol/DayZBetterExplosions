// Better Explosions test bed (loads @BetterExplosions only, see mods.txt): no Molotov, so
// what sets vehicles off is vanilla's explosives. Players spawn on an empty stretch of the
// east-coast beach (BEACH) with landmines, grenades, C4 with remote detonators, an
// improvised explosive and an M79 with 40 mm explosive rounds at their feet; one of every
// vanilla vehicle, fuelled, stands in a row along the beach behind them, and three boats
// float at the water's edge (laid out on a fresh world only).
// The headless self-test (-missiontest) parks a fuelled vehicle at each of its own spots
// along the beach and sets a different explosive off against each, then checks which went
// up (logs [BXTest] lines and quits).

void main()
{
	Hive ce = CreateHive();
	if (ce)
		ce.InitOffline();

	GetGame().GetWorld().SetDate(2026, 9, 20, 10, 0);
}

class BXCase
{
	string m_Vehicle;
	string m_How;      //!< mine, 40mm, c4, ied, grenade_close, grenade_far
	bool m_WantUp;
	vector m_At;
	Transport m_Spawned;

	void BXCase(string vehicle, string how, bool wantUp)
	{
		m_Vehicle = vehicle;
		m_How = how;
		m_WantUp = wantUp;
	}
}

class CustomMission: MissionServer
{
	//! open beach on the east coast, 250 m from the nearest building (Molotov's test beach)
	static const vector BEACH = "13480 0 5700";
	static const ref array<string> VEHICLES = {"OffroadHatchback", "Hatchback_02", "CivilianSedan", "Sedan_02", "Offroad_02", "Truck_01_Covered", "Motorbike_01_Red", "Motorbike_02_Blue"};
	static const float VEHICLE_GAP = 4;   //!< between bumpers: a C4 by one doesn't reach the next
	static const int BOATS = 3;
	static const float BOAT_APART = 5;
	//! what's at the player's feet: three of each
	static const ref array<string> KIT = {"LandMineTrap", "M67Grenade", "RGD5Grenade", "Plastic_Explosive", "RemoteDetonator", "ImprovisedExplosive", "M79", "Ammo_40mm_Explosive"};

	protected bool m_Test;
	protected vector m_Along;   //!< along the shoreline
	protected vector m_Land;    //!< away from the sea
	protected ref array<ref BXCase> m_Cases = new array<ref BXCase>;

	void CustomMission()
	{
		string value;
		m_Test = GetGame().CommandlineGetParam("missiontest", value);
		Shoreline();
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(LayOut, 3000, false);
	}

	//! which way the sea is from the beach
	void Shoreline()
	{
		vector b = Ground(BEACH);
		m_Land = "1 0 0";
		for (int a = 0; a < 16; a++)
		{
			float ang = a * Math.PI2 / 16;
			vector dir = Vector(Math.Cos(ang), 0, Math.Sin(ang));
			vector q = b + dir * 60;
			if (GetGame().SurfaceIsSea(q[0], q[2]))
			{
				m_Land = -dir;
				break;
			}
		}
		m_Along = Vector(m_Land[2], 0, -m_Land[0]);
	}

	vector Ground(vector p)
	{
		return BetterExplosions.Ground(p);
	}

	void LayOut()
	{
		vector b = Ground(BEACH);
		if (m_Test)
		{
			SelfTest(b);
			return;
		}
		// a fresh world only: after a restart the vehicles from last time are still here
		array<Object> objs = new array<Object>;
		GetGame().GetObjectsAtPosition(b, 150, objs, null);
		foreach (Object o : objs)
		{
			if (Transport.Cast(o))
			{
				Print("[BX] vehicles from last time on the beach: not laying out more");
				return;
			}
		}
		// one of every vanilla vehicle, nose to tail, along the beach behind the spawn
		float behind = 12;
		foreach (string type : VEHICLES)
		{
			vector front;
			vector rear;
			Length(type, front, rear);
			float fz = front[2];
			float rz = rear[2];
			SpawnVehicle(type, Ground(b - m_Along * (behind + fz) + m_Land * 14), m_Along);
			behind = behind + fz - rz + VEHICLE_GAP;
		}
		// boats side by side from the water's edge out
		vector shore = Shore(b, -m_Land);
		for (int i = 0; i < BOATS; i++)
		{
			float seaward = BOAT_APART * i;
			Transport boat = SpawnVehicle("Boat_01_Orange", Ground(shore - m_Land * seaward), m_Along);
		}
		Print("[BX] beach laid out at " + b);
	}

	//! A vehicle type's collision box, measured on a copy made far off and deleted
	void Length(string type, out vector hi, out vector lo)
	{
		hi = "0 0 2.5";
		lo = "0 0 -2.5";
		Object probe = GetGame().CreateObjectEx(type, "100 0 100", ECE_LOCAL);
		vector box[2];
		if (probe && probe.GetCollisionBox(box))
		{
			lo = box[0];
			hi = box[1];
		}
		if (probe)
			GetGame().ObjectDelete(probe);
	}

	//! The first spot of sea about knee deep, walking from from towards dir
	vector Shore(vector from, vector dir)
	{
		for (int d = 0; d < 150; d++)
		{
			float far = d;
			vector p = from + dir * far;
			if (!GetGame().SurfaceIsSea(p[0], p[2]))
				continue;
			p = Ground(p);
			float bottom = GetGame().SurfaceY(p[0], p[2]);
			if (p[1] - bottom >= 0.7)
				return p;
		}
		return from + dir * 20;
	}

	Transport SpawnVehicle(string type, vector at, vector dir)
	{
		int flags = ECE_PLACE_ON_SURFACE;
		if (type.IndexOf("Boat") == 0)
			flags = ECE_CREATEPHYSICS | ECE_TRACE;
		Transport t = Transport.Cast(GetGame().CreateObjectEx(type, at, flags));
		if (!t)
		{
			Print("[BX] could not create a " + type);
			return null;
		}
		t.SetDirection(dir);
		t.SetLifetimeMax(3888000);
		t.SetLifetime(3888000);
		t.OnDebugSpawn();   // wheels, doors, battery, fluids - a full tank
		Print("[BX] " + type + " at " + at);
		return t;
	}

	// ---------------------------------------------------------------- the self-test

	void SelfTest(vector b)
	{
		m_Cases.Insert(new BXCase("OffroadHatchback", "mine", true));
		m_Cases.Insert(new BXCase("Hatchback_02", "40mm", true));
		m_Cases.Insert(new BXCase("CivilianSedan", "c4", true));
		m_Cases.Insert(new BXCase("Sedan_02", "ied", true));
		m_Cases.Insert(new BXCase("Offroad_02", "grenade_close", true));
		m_Cases.Insert(new BXCase("Truck_01_Covered", "grenade_far", false));
		m_Cases.Insert(new BXCase("Motorbike_01_Red", "mine", true));
		m_Cases.Insert(new BXCase("Boat_01_Orange", "40mm", true));
		// each on its own stretch of beach, 30 m apart, well out of each other's reach
		vector shore = Shore(b, -m_Land);
		foreach (int i, BXCase c : m_Cases)
		{
			float along = 30 * i - 100;
			if (c.m_Vehicle.IndexOf("Boat") == 0)
				c.m_At = Ground(shore + m_Along * along);
			else
				c.m_At = Ground(b + m_Along * along + m_Land * 14);
			c.m_Spawned = SpawnVehicle(c.m_Vehicle, c.m_At, m_Along);
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(SetOff, 5000, false);
	}

	void SetOff()
	{
		foreach (BXCase c : m_Cases)
		{
			Transport t = c.m_Spawned;
			if (!t)
			{
				Print("[BXTest] FAIL no " + c.m_Vehicle);
				continue;
			}
			c.m_At = t.GetPosition();
			Print("[BXTest] " + c.m_Vehicle + " fuel " + t.GetFluidFractionScript(ETransportFluid.FUEL) + ", setting off a " + c.m_How + " by it");
			// beside it: just off its right side (a grenade_far 4 m off)
			vector box[2];
			float side = 1.2;
			if (t.GetCollisionBox(box))
			{
				vector hi = box[1];
				side = hi[0] + 0.5;
			}
			if (c.m_How == "grenade_far")
				side = side + 3.5;
			vector at = Ground(t.ModelToWorld(Vector(side, 0, 0)));
			if (c.m_How == "mine")
			{
				EntityAI mine = EntityAI.Cast(GetGame().CreateObjectEx("LandMineTrap", at, ECE_PLACE_ON_SURFACE));
				if (mine)
					mine.Explode(DamageType.EXPLOSION);
			}
			else if (c.m_How == "40mm")
			{
				EntityAI round = EntityAI.Cast(GetGame().CreateObjectEx("Ammo_40mm_Explosive", at, ECE_PLACE_ON_SURFACE));
				DamageSystem.ExplosionDamage(round, null, "Explosion_40mm_Ammo", at + "0 0.3 0", DamageType.EXPLOSION);
				if (round)
					GetGame().ObjectDelete(round);
			}
			else if (c.m_How == "c4" || c.m_How == "ied")
			{
				// an IED has no charge of its own: it sets off what's attached (C4 here)
				EntityAI bomb = EntityAI.Cast(GetGame().CreateObjectEx("Plastic_Explosive", at, ECE_PLACE_ON_SURFACE));
				if (c.m_How == "ied")
				{
					EntityAI ied = EntityAI.Cast(GetGame().CreateObjectEx("ImprovisedExplosive", at, ECE_PLACE_ON_SURFACE));
					if (ied)
					{
						GetGame().ObjectDelete(bomb);
						bomb = ied.GetInventory().CreateAttachment("Plastic_Explosive");
					}
				}
				if (bomb)
				{
					bomb.Explode(DamageType.EXPLOSION, "Plastic_Explosive_Ammo");
					GetGame().ObjectDelete(bomb);
				}
			}
			else
			{
				Grenade_Base grenade = Grenade_Base.Cast(GetGame().CreateObjectEx("M67Grenade", at + "0 0.2 0", ECE_PLACE_ON_SURFACE));
				if (grenade)
					grenade.ActivateImmediate();
			}
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Check, 6000, false);
	}

	void Check()
	{
		int good;
		foreach (BXCase c : m_Cases)
		{
			bool up = !c.m_Spawned || c.m_Spawned.IsRuined();
			string left = "";
			int burning = 0;
			array<Object> near = new array<Object>;
			GetGame().GetObjectsAtPosition(c.m_At, 20, near, null);
			foreach (Object o : near)
			{
				string type = o.GetType();
				if (type == "BetterExplosionsBurning" && vector.Distance(o.GetPosition(), c.m_At) < 6)
					burning++;
				else if (type.IndexOf("Land_Wreck_") == 0 || type.IndexOf("Land_wreck_") == 0 || type.IndexOf("BetterExplosions_") == 0)
					left += " " + type;
			}
			bool ok = up == c.m_WantUp && (!up || burning > 0);
			if (ok)
				good++;
			string verdict = "";
			if (!ok)
				verdict = "  <-- FAIL";
			Print("[BXTest] " + c.m_Vehicle + " + " + c.m_How + ": went up " + up + " (want " + c.m_WantUp + "), burning " + burning + ", left:" + left + verdict);
		}
		Print("[BXTest] " + good + " of " + m_Cases.Count() + " good");
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Quit, 2000, false);
	}

	void Quit()
	{
		GetGame().RequestExit(0);
	}

	// ---------------------------------------------------------------- players

	override void StartingEquipSetup(PlayerBase player, bool clothesChosen)
	{
		vector pos = Ground(BEACH);
		vector side = Vector(m_Along[2], 0, -m_Along[0]);
		player.SetPosition(pos);
		player.SetDirection(m_Along);
		for (int i = 0; i < KIT.Count(); i++)
		{
			for (int n = 0; n < 3; n++)
			{
				vector p = Ground(pos - side * (1.5 + n * 0.6) + m_Along * (i * 0.8 - 1));
				ItemBase item = ItemBase.Cast(GetGame().CreateObjectEx(KIT[i], p, ECE_PLACE_ON_SURFACE));
				if (!item)
				{
					Print("[BX] unknown kit item " + KIT[i]);
					break;
				}
				if (item.HasQuantity())
					item.SetQuantityMax();
			}
		}
		Print("[BX] player kitted at " + pos);
	}
}

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}
