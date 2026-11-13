//! Tunables
class BetterExplosionsSettings
{
	static const string BLAST_AMMO = "LandFuelFeed_Ammo";   //!< vanilla's fuel pump explosion (its blast on objects, and its sound)
	static const float BLAST_HEALTH = 60;        //!< what the blast does to a player (and shock) within BLAST_FULL m of it...
	static const float BLAST_FULL = 2;
	static const float BLAST_REACH = 8;          //!< ...falling to nothing this far out
	static const float BLAST_CREATURES = 2.5;    //!< infected and animals take this many times as much
	static const float BLAST_LOAD_DAMAGE = 0.75; //!< what going up does to everything in a vehicle: this fraction of each item's full health
	static const int FIREBALL_MS = 8000;         //!< the fireball's object is gone after this
}

//! Fuelled vehicles and fuel pumps that go up properly. The library's whole API for other
//! mods (server side):
//!   BetterExplosions.CanExplode(obj)   - a fuelled vehicle, or a working fuel pump
//!   BetterExplosions.Explode(obj)      - it goes up, now
//!   BetterExplosions.BlastPeople(src, at) - just the blast's hit on the living
//!   BetterExplosionsWreck.Add / Fling  - more cars with a wreck and flying bits
//! What going up does: the vehicle is ruined and everything in it badly damaged; vanilla's
//! fuel pump explosion goes off over it (sound, and its blast on objects); a fireball; the
//! living around take a real hit (vanilla's barely touches a player), with a flinch, a cry
//! and the red flash. Then a car becomes the matching vanilla wreck, its cargo thrown out
//! round it and a burnt wheel and its hood flying; a boat's outboard flies off and sinks,
//! and its burnt hull sinks; a motorbike flies apart. A pump is vanilla's ruined pump.
//! Nothing burns afterwards: lighting a fire on the spot is up to the mod that set it off.
class BetterExplosions
{
	//! A vehicle with fuel in it, not ruined
	static bool HasFuel(Transport t)
	{
		return t && !t.IsRuined() && t.GetFluidFractionScript(ETransportFluid.FUEL) > 0;
	}

	//! Would Explode do anything to o?
	static bool CanExplode(Object o)
	{
		Land_FuelStation_Feed pump = Land_FuelStation_Feed.Cast(o);
		if (pump)
			return !pump.IsRuined();
		return HasFuel(Transport.Cast(o));
	}

	//! Server: o (a fuelled vehicle or a working fuel pump) goes up now. False if it can't
	static bool Explode(Object o)
	{
		if (!o || !GetGame().IsServer() || !CanExplode(o))
			return false;
		vector pos = Ground(o.GetPosition());

		Land_FuelStation_Feed pump = Land_FuelStation_Feed.Cast(o);
		if (pump)
		{
			// vanilla blows a pump up when it's destroyed (Land_FuelStation_Feed.EEKilled)
			pump.SetHealth("", "", 0);
			Print("[BetterExplosions] fuel pump went up at " + pos + " ruined=" + pump.IsRuined());
			BlastPeople(pump, pos + "0 1 0");
			return true;
		}

		Transport t = Transport.Cast(o);
		CarScript car = CarScript.Cast(t);
		if (car)
		{
			car.LeakAll(CarFluid.FUEL);
			car.SetHealth("Engine", "Health", 0);
			car.SetHealth("FuelTank", "Health", 0);
		}
		BoatScript boat = BoatScript.Cast(t);
		if (boat)
			boat.LeakAll(BoatFluid.FUEL);
		if (t.GetMaxHealth("Engine", "Health") > 0)
			t.SetHealth("Engine", "Health", 0);
		t.SetHealth("", "", 0);

		// everything in it - parts and cargo - takes a beating: pristine comes out badly
		// damaged, anything already worn is ruined
		array<EntityAI> load = new array<EntityAI>;
		t.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, load);
		foreach (EntityAI item : load)
		{
			if (item != t)
				item.AddHealth("", "Health", -item.GetMaxHealth("", "Health") * BetterExplosionsSettings.BLAST_LOAD_DAMAGE);
		}
		Print("[BetterExplosions] " + t.GetType() + " went up at " + pos + ", " + (load.Count() - 1) + " items in it damaged");
		GetGame().CreateObjectEx("BetterExplosionsFireball", pos, ECE_CREATEPHYSICS);
		// the blast goes off just over the top of the vehicle: from inside, a car's own body
		// shields everyone standing round it from the explosion
		vector at = pos + "0 0.5 0";
		vector box[2];
		if (t.GetCollisionBox(box))
		{
			vector lo = box[0];
			vector hi = box[1];
			at = t.ModelToWorld(Vector((lo[0] + hi[0]) * 0.5, hi[1] + 0.3, (lo[2] + hi[2]) * 0.5));
		}
		DamageSystem.ExplosionDamage(t, null, BetterExplosionsSettings.BLAST_AMMO, at, DamageType.EXPLOSION);
		BlastPeople(t, pos + "0 1 0");

		// what's left: a moment later, once the blast is done with the vehicle
		if (boat)
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(BetterExplosionsDebris.Sink, 200, false, boat);
		MotorbikeScript bike = MotorbikeScript.Cast(t);
		if (bike)
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(BetterExplosionsDebris.Scatter, 200, false, bike);
		if (car && BetterExplosionsWreck.For(car) != "")
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(BetterExplosionsWreck.Swap, 200, false, car);
		return true;
	}

	//! Server: the blast's hit on everyone round at. The ammo's own damage hardly touches a
	//! player, so the bite on the living is ours: full close in, none at the edge
	static void BlastPeople(EntityAI src, vector at)
	{
		array<Object> near = new array<Object>;
		GetGame().GetObjectsAtPosition(at, BetterExplosionsSettings.BLAST_REACH, near, null);
		foreach (Object o : near)
		{
			EntityAI e = EntityAI.Cast(o);
			if (!e || !e.IsAlive())
				continue;
			float d = vector.Distance(at, e.GetPosition() + "0 1 0");
			float f = 1 - Math.Clamp((d - BetterExplosionsSettings.BLAST_FULL) / (BetterExplosionsSettings.BLAST_REACH - BetterExplosionsSettings.BLAST_FULL), 0, 1);
			if (f <= 0)
				continue;
			float hit = BetterExplosionsSettings.BLAST_HEALTH * f;
			PlayerBase pb = PlayerBase.Cast(e);
			if (pb)
			{
				BetterExplosionsPain.Hit(pb, src, hit, hit);
				Print("[BetterExplosions] blast hit " + pb.GetType() + " " + d + " m off for " + hit);
			}
			else if (e.IsInherited(ZombieBase) || e.IsInherited(AnimalBase))
			{
				e.AddHealth("", "Health", -hit * BetterExplosionsSettings.BLAST_CREATURES);
			}
		}
	}

	//! Ground level - or the water's surface (the sea with its tide and waves, or a pond)
	static vector Ground(vector p)
	{
		p[1] = GetGame().SurfaceY(p[0], p[2]);
		float depth = GetGame().GetWaterDepth(p);   // how far that point is under a pond
		if (depth > 0)
			p[1] = p[1] + depth;
		float sea = SeaLevel(p);                    // (GetWaterDepth doesn't see the sea)
		if (sea > p[1])
			p[1] = sea;
		return p;
	}

	//! The sea's surface over p (it rises and falls), or far below if p isn't over the sea
	static float SeaLevel(vector p)
	{
		if (GetGame().SurfaceIsSea(p[0], p[2]))
			return GetGame().SurfaceGetSeaLevel();
		return -1000;
	}
}

//! The fireball of a vehicle going up (clients); the object itself goes soon after
class BetterExplosionsFireball: House
{
	override void EEInit()
	{
		super.EEInit();
		if (GetGame().IsServer())
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GetGame().ObjectDelete, BetterExplosionsSettings.FIREBALL_MS, false, this);
		if (!GetGame().IsDedicatedServer())
		{
			vector c = GetPosition();
			ParticleManager.GetInstance().PlayInWorld(ParticleList.EXPLOSION_LANDMINE, c + "0 0.5 0");
			ParticleManager.GetInstance().PlayInWorld(ParticleList.EXPLOSION_LANDMINE, c + "0 1.5 0");
		}
	}
}
