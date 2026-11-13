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

	// What sets a fuelled vehicle off: vanilla explosives that hit it. Landmines (stepped
	// on, or driven over), the 40 mm grenade launcher's explosive round, C4 and the
	// improvised explosive go off whatever part they hit; a hand grenade only when it goes
	// off right against the vehicle or under it
	static const ref array<string> TRIGGER_AMMO = {"LandMineExplosion_Ammo", "LandMineExplosion_CarWheel", "Explosion_40mm_Ammo", "Plastic_Explosive_Ammo"};
	static const ref array<string> GRENADE_AMMO = {"RGD5Grenade_Ammo", "M67Grenade_Ammo"};
	static const float GRENADE_REACH = 1.5;      //!< m from the vehicle's sides (its collision box)
	static const int TRIGGER_MIN_MS = 150;       //!< from the hit to going up, up to
	static const int TRIGGER_MAX_MS = 500;

	// The burning wreck left behind, unless another mod lights its own fire (Molotov does)
	static const int BURN_MS = 60000;
	static const float BURN_RADIUS = 3;
	static const float BURN_HEIGHT = 2.5;
	static const float BURN_HEALTH_PS = 8;       //!< to a player standing in it, and shock
	static const float BURN_CREATURES_PS = 20;
}

//! Fuelled vehicles and fuel pumps that go up properly. The library's whole API for other
//! mods (server side):
//!   BetterExplosions.CanExplode(obj)   - a fuelled vehicle, or a working fuel pump
//!   BetterExplosions.Explode(obj)      - it goes up, now
//!   BetterExplosions.BlastPeople(src, at) - just the blast's hit on the living
//!   BetterExplosionsWreck.Add / Fling  - more cars with a wreck and flying bits
//!   BetterExplosions.Event_OnExploded  - (Object what, vector where) after anything goes up
//!   BetterExplosions.s_OwnFire = false - don't leave our burning wreck: you light your own
//! On its own it sets vehicles off when vanilla explosives hit them (TRIGGER_AMMO).
//! What going up does: the vehicle is ruined and everything in it badly damaged; vanilla's
//! fuel pump explosion goes off over it (sound, and its blast on objects); a fireball; the
//! living around take a real hit (vanilla's barely touches a player), with a flinch, a cry
//! and the red flash. Then a car becomes the matching vanilla wreck, its cargo thrown out
//! round it and a burnt wheel and its hood flying; a boat's outboard flies off and sinks,
//! and its burnt hull sinks; a motorbike flies apart. A pump is vanilla's ruined pump.
//! Then it burns for a minute (BetterExplosionsBurning) - or not, if another mod sets
//! s_OwnFire off to light its own fire from Event_OnExploded.
class BetterExplosions
{
	//! Called after anything goes up (server): (Object what - it may be gone a moment later,
	//! vector where - the ground, or the water's surface, where it stood)
	static ref ScriptInvoker Event_OnExploded = new ScriptInvoker();
	//! Leave our own burning wreck where something went up. A mod that lights its own fire
	//! from Event_OnExploded sets this false (MissionServer.OnInit)
	static bool s_OwnFire = true;

	//! vehicles hit by a trigger, counting down to going up
	protected static ref array<Object> s_Pending = new array<Object>;

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
			Aftermath(pump, pos);
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
		Aftermath(t, pos);
		return true;
	}

	//! The fire, and telling whoever's listening
	protected static void Aftermath(Object o, vector pos)
	{
		if (s_OwnFire)
			GetGame().CreateObjectEx("BetterExplosionsBurning", pos, ECE_CREATEPHYSICS);
		Event_OnExploded.Invoke(o, pos);
	}

	//! Server: a vehicle took a hit (its EEHitBy). A vanilla explosive sets it off if it has fuel
	static void OnHit(Transport t, int damageType, EntityAI source, string ammo)
	{
		if (damageType != DamageType.EXPLOSION || !GetGame().IsServer() || s_Pending.Find(t) >= 0 || !HasFuel(t))
			return;
		bool trigger = BetterExplosionsSettings.TRIGGER_AMMO.Find(ammo) >= 0;
		if (!trigger && BetterExplosionsSettings.GRENADE_AMMO.Find(ammo) >= 0)
			trigger = !source || Gap(t, source.GetPosition()) <= BetterExplosionsSettings.GRENADE_REACH;
		if (!trigger)
		{
			Print("[BetterExplosions] " + t.GetType() + " hit by " + ammo + ": not enough to set it off");
			return;
		}
		Print("[BetterExplosions] " + t.GetType() + " hit by " + ammo + ": it goes up");
		s_Pending.Insert(t);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ExplodePending, Math.RandomIntInclusive(BetterExplosionsSettings.TRIGGER_MIN_MS, BetterExplosionsSettings.TRIGGER_MAX_MS), false, t);
	}

	static void ExplodePending(Transport t)
	{
		s_Pending.RemoveItem(t);
		Explode(t);
	}

	//! How far p is from o's sides (its collision box), 0 inside it
	static float Gap(Object o, vector p)
	{
		vector box[2];
		if (!o.GetCollisionBox(box))
			return vector.Distance(o.GetPosition(), p);
		vector inModel = o.WorldToModel(p);
		vector lo = box[0];
		vector hi = box[1];
		float dx = Math.Max(0, Math.Max(lo[0] - inModel[0], inModel[0] - hi[0]));
		float dy = Math.Max(0, Math.Max(lo[1] - inModel[1], inModel[1] - hi[1]));
		float dz = Math.Max(0, Math.Max(lo[2] - inModel[2], inModel[2] - hi[2]));
		return Math.Sqrt(dx * dx + dy * dy + dz * dz);
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

//! A vehicle that's gone up, burning for a minute: flames, smoke, light and the roar of it,
//! burning whoever stands in it (players flinch and cry out). Doesn't spread. Not left
//! when another mod lights its own fire (BetterExplosions.s_OwnFire)
class BetterExplosionsBurning: House
{
	static const float TICK_S = 0.5;

	protected ref array<Particle> m_Flames;
	protected ScriptedLightBase m_Light;
	protected EffectSound m_Sound;

	override void EEInit()
	{
		super.EEInit();
		if (GetGame().IsServer())
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Burn, TICK_S * 1000, true);
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GoOut, BetterExplosionsSettings.BURN_MS, false);
		}
		if (!GetGame().IsDedicatedServer())
			StartEffects();
	}

	protected void Burn()
	{
		vector c = GetPosition();
		array<Object> near = new array<Object>;
		GetGame().GetObjectsAtPosition(c, BetterExplosionsSettings.BURN_RADIUS, near, null);
		foreach (Object o : near)
		{
			EntityAI e = EntityAI.Cast(o);
			if (!e || !e.IsAlive())
				continue;
			vector ep = e.GetPosition();
			float up = ep[1] - c[1];
			if (up < -1 || up > BetterExplosionsSettings.BURN_HEIGHT)
				continue;
			PlayerBase pb = PlayerBase.Cast(e);
			if (pb)
				BetterExplosionsPain.Hit(pb, this, BetterExplosionsSettings.BURN_HEALTH_PS * TICK_S, BetterExplosionsSettings.BURN_HEALTH_PS * TICK_S);
			else if (e.IsInherited(ZombieBase) || e.IsInherited(AnimalBase))
				e.AddHealth("", "Health", -BetterExplosionsSettings.BURN_CREATURES_PS * TICK_S);
		}
	}

	protected void GoOut()
	{
		GetGame().ObjectDelete(this);
	}

	protected void StartEffects()
	{
		vector c = GetPosition();
		m_Flames = new array<Particle>;
		m_Flames.Insert(ParticleManager.GetInstance().PlayInWorld(ParticleList.BONFIRE_FIRE, c));
		m_Flames.Insert(ParticleManager.GetInstance().PlayInWorld(ParticleList.BONFIRE_SMOKE, c + "0 1 0"));
		m_Flames.Insert(ParticleManager.GetInstance().PlayInWorld(ParticleList.SMOKE_GENERIC_WRECK, c + "0 0.5 0"));
		float r = BetterExplosionsSettings.BURN_RADIUS * 0.6;
		for (int i = 0; i < 6; i++)
		{
			float a = i * Math.PI2 / 6 + Math.RandomFloat(-0.3, 0.3);
			float d = r * Math.RandomFloat(0.5, 1.1);
			vector p = BetterExplosions.Ground(c + Vector(Math.Cos(a) * d, 0, Math.Sin(a) * d));
			m_Flames.Insert(ParticleManager.GetInstance().PlayInWorld(ParticleList.CAMP_NORMAL_FIRE, p));
		}
		m_Light = ScriptedLightBase.CreateLight(FireplaceLight, c + "0 1.5 0");
		m_Light.SetRadiusTo(20);
		m_Light.SetBrightnessTo(1.5);
		PlaySoundSetLoop(m_Sound, "HeavyFire_SoundSet", 0.2, 1.0);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Burn);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GoOut);
		if (m_Flames)
		{
			foreach (Particle f : m_Flames)
			{
				if (f)
					f.Stop();
			}
		}
		if (m_Light)
			m_Light.FadeOut(1);
		if (m_Sound)
			StopSoundSet(m_Sound);
	}
}
