// Better Explosions - Big Machines test bed (loads @BigMachines, @BetterExplosions and
// @BetterExplosions_BigMachines, see mods.txt). Players spawn on Better Explosions' test beach
// with its explosives kit at their feet (landmines, grenades, C4 with remote detonators, an
// improvised explosive, an M79 with 40 mm explosive rounds); a fuelled bulldozer, excavator and
// combine harvester stand in a row behind them (laid out on a fresh world only).
// The headless self-test (-missiontest) does what Big Machines does at a server start - puts a
// bulldozer, an excavator and a combine on wreck sites of their own, the map's wreck sunk out
// of sight under each - then sets a 40 mm round off against each machine where it stands and
// checks that it went up into the right wreck, standing (not sunk with the wreck it replaced),
// with its parts flung round it (logs [BXBMTest] lines and quits).

void main()
{
	Hive ce = CreateHive();
	if (ce)
		ce.InitOffline();

	GetGame().GetWorld().SetDate(2026, 9, 20, 10, 0);
}

class BXBMCase
{
	string m_Wreck;    //!< the wreck it should leave
	BM_Machine m_Machine;
	vector m_At;

	void BXBMCase(string wreck, BM_Machine machine)
	{
		m_Wreck = wreck;
		m_Machine = machine;
		if (machine)
			m_At = machine.GetPosition();
	}
}

class CustomMission: MissionServer
{
	//! Better Explosions' test beach on the east coast
	static const vector BEACH = "13480 0 5700";
	static const ref array<string> MACHINES = {"BM_Bulldozer", "BM_Excavator", "BM_Combine"};
	static const ref array<int> KINDS = {BM_WreckSite.DOZER, BM_WreckSite.DIGGER, BM_WreckSite.COMBINE};
	static const ref array<string> WRECKS = {"StaticObj_Wreck_Bulldozer", "StaticObj_Wreck_Digger", "Land_Wreck_Harverster"};
	static const float MACHINE_GAP = 25;
	static const ref array<string> KIT = {"LandMineTrap", "M67Grenade", "RGD5Grenade", "Plastic_Explosive", "RemoteDetonator", "ImprovisedExplosive", "M79", "Ammo_40mm_Explosive"};

	protected bool m_Test;
	protected vector m_Along = "0 0 1";
	protected vector m_Land = "-1 0 0";
	protected ref array<ref BXBMCase> m_Cases = new array<ref BXBMCase>;

	void CustomMission()
	{
		string value;
		m_Test = GetGame().CommandlineGetParam("missiontest", value);
		// after Big Machines has placed its own machines (1 s after the mission starts)
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(LayOut, 10000, false);
	}

	vector Ground(vector p)
	{
		return BetterExplosions.Ground(p);
	}

	float Yaw(vector dir)
	{
		return Math.Atan2(dir[0], dir[2]) * Math.RAD2DEG;
	}

	//! a full tank and a battery: ready to drive (or blow up)
	void Ready(BM_Machine m)
	{
		m.LeakAll(CarFluid.FUEL);
		m.Fill(CarFluid.FUEL, m.GetFluidCapacity(CarFluid.FUEL));
		if (!m.FindAttachmentBySlotName("TruckBattery"))
			m.GetInventory().CreateAttachment("TruckBattery");
	}

	void LayOut()
	{
		if (m_Test)
		{
			SelfTest();
			return;
		}
		vector b = Ground(BEACH);
		array<Object> objs = new array<Object>;
		GetGame().GetObjectsAtPosition(b, 150, objs, null);
		foreach (Object o : objs)
		{
			if (BM_Machine.Cast(o))
			{
				Print("[BXBM] machines from last time on the beach: not laying out more");
				return;
			}
		}
		foreach (int i, string type : MACHINES)
		{
			vector at = b - m_Along * (15 + i * MACHINE_GAP) + m_Land * 16;
			BM_Machine m = BM_Wrecks.PlaceVehicle(type, Ground(at), Yaw(m_Along));
			if (!m)
			{
				Print("[BXBM] could not place a " + type);
				continue;
			}
			Ready(m);
			Print("[BXBM] " + type + " at " + m.GetPosition());
		}
		Print("[BXBM] beach laid out at " + b);
	}

	// ---------------------------------------------------------------- the self-test

	void SelfTest()
	{
		string world = GetGame().GetWorldName();
		world.ToLower();
		array<ref BM_WreckSite> sites = new array<ref BM_WreckSite>;
		BM_WreckSites.Get(world, sites);
		foreach (int i, int kind : KINDS)
		{
			BM_Machine machine = null;
			foreach (BM_WreckSite site : sites)
			{
				if (site.Kind != kind || BM_Wrecks.IsHidden(site.Match, site.Pos))
					continue;
				machine = BM_Wrecks.Replace(site);
				if (machine)
					break;
			}
			if (!machine)
			{
				Print("[BXBMTest] FAIL no site for a " + MACHINES[i]);
				continue;
			}
			Ready(machine);
			m_Cases.Insert(new BXBMCase(WRECKS[i], machine));
			Print("[BXBMTest] " + machine.GetType() + " on its wreck site at " + machine.GetPosition());
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(SetOff, 5000, false);
	}

	void SetOff()
	{
		foreach (BXBMCase c : m_Cases)
		{
			BM_Machine m = c.m_Machine;
			if (!m)
				continue;
			c.m_At = m.GetPosition();
			vector box[2];
			float side = 2;
			if (m.GetCollisionBox(box))
			{
				vector hi = box[1];
				side = hi[0] + 0.5;
			}
			vector at = Ground(m.ModelToWorld(Vector(side, 0, 0)));
			Print("[BXBMTest] " + m.GetType() + " fuel " + m.GetFluidFraction(CarFluid.FUEL) + ", a 40 mm round off by it");
			EntityAI round = EntityAI.Cast(GetGame().CreateObjectEx("Ammo_40mm_Explosive", at, ECE_PLACE_ON_SURFACE));
			DamageSystem.ExplosionDamage(round, null, "Explosion_40mm_Ammo", at + "0 0.3 0", DamageType.EXPLOSION);
			if (round)
				GetGame().ObjectDelete(round);
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Check, 8000, false);
	}

	void Check()
	{
		int good = 0;
		foreach (BXBMCase c : m_Cases)
		{
			bool gone = !c.m_Machine;
			string wreck = "none";
			bool standing = false;
			int flung = 0;
			array<Object> near = new array<Object>;
			GetGame().GetObjectsAtPosition(c.m_At, 30, near, null);
			foreach (Object o : near)
			{
				string type = o.GetType();
				vector p = o.GetPosition();
				if (type == c.m_Wreck && vector.Distance(p, c.m_At) < 12)
				{
					wreck = type + " at " + p;
					standing = p[1] > GetGame().SurfaceY(p[0], p[2]) - 5;
				}
				else if (type.IndexOf("BetterExplosions_BM") == 0)
				{
					flung++;
				}
			}
			bool ok = gone && standing && flung > 0;
			if (ok)
				good++;
			string verdict = "";
			if (!ok)
				verdict = "  <-- FAIL";
			Print("[BXBMTest] -> " + c.m_Wreck + ": machine gone " + gone + ", wreck " + wreck + " standing " + standing + ", " + flung + " parts flung" + verdict);
		}
		Print("[BXBMTest] " + good + " of " + KINDS.Count() + " good");
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
		player.SetDirection(-m_Along);
		for (int i = 0; i < KIT.Count(); i++)
		{
			for (int n = 0; n < 3; n++)
			{
				vector p = Ground(pos - side * (1.5 + n * 0.6) + m_Along * (i * 0.8 - 1));
				ItemBase item = ItemBase.Cast(GetGame().CreateObjectEx(KIT[i], p, ECE_PLACE_ON_SURFACE));
				if (item && item.HasQuantity())
					item.SetQuantityMax();
			}
		}
		Print("[BXBM] player kitted at " + pos);
	}
}

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}
