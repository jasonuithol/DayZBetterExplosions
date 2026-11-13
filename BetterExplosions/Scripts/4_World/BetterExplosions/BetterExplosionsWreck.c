//! A wreck another mod has added for a car (BetterExplosionsWreck.Add)
class BetterExplosionsWreckType
{
	string m_Car;     //!< car class, its subclasses included
	string m_Wreck;   //!< wreck class, or several "A|B" to pick from at random
	float m_Turn;     //!< degrees the wreck model is turned from the car's
	ref array<string> m_Pieces = new array<string>;   //!< BetterExplosionsDebris classes flung off it
	ref array<bool> m_OnEdge = new array<bool>;       //!< that piece's model stands on edge (a wheel, a door)

	void BetterExplosionsWreckType(string car, string wreck, float turn)
	{
		m_Car = car;
		m_Wreck = wreck;
		m_Turn = turn;
	}
}

//! A car that's gone up is swapped for vanilla's wreck of it (the abandoned-car map
//! props), with what was in its cargo thrown out on the ground round the wreck
class BetterExplosionsWreck
{
	//! wrecks added by other mods (BetterExplosions_Expansion), ahead of our own table
	static ref array<ref BetterExplosionsWreckType> s_Added = new array<ref BetterExplosionsWreckType>;

	//! For other mods: a car of this class (or a subclass) that goes up is swapped for the
	//! wreck, or one of "A|B" at random. turn is how far the wreck's model is turned from
	//! the car's (the vanilla wreck props face backwards: 180). The most specific class
	//! added wins, and it beats our own table. Server side, before anything burns:
	//! MissionServer.OnInit is the place. Adding a class again starts it afresh.
	static void Add(string car, string wreck, float turn = 180)
	{
		foreach (int i, BetterExplosionsWreckType known : s_Added)
		{
			if (known.m_Car == car)
			{
				s_Added.Remove(i);
				break;
			}
		}
		s_Added.Insert(new BetterExplosionsWreckType(car, wreck, turn));
		Print("[BetterExplosions] " + car + " burns down to " + wreck);
	}

	//! For other mods: one more piece flung off that car (Add it first) when it goes up, a
	//! BetterExplosionsDebris class (it must exist on the clients too). The first flies off the side,
	//! the second off the front, the next off the other side, the back, then anywhere.
	//! onEdge: its model stands on edge (a wheel, a door), so it comes down on its side.
	static void Fling(string car, string piece, bool onEdge = false)
	{
		foreach (BetterExplosionsWreckType known : s_Added)
		{
			if (known.m_Car == car)
			{
				known.m_Pieces.Insert(piece);
				known.m_OnEdge.Insert(onEdge);
				return;
			}
		}
		Print("[BetterExplosions] Fling: add a wreck for " + car + " first");
	}

	//! The added wreck for this car, null if none: the deepest class that matches
	static BetterExplosionsWreckType Added(CarScript car)
	{
		BetterExplosionsWreckType best;
		foreach (BetterExplosionsWreckType w : s_Added)
		{
			if (!car.IsKindOf(w.m_Car))
				continue;
			if (!best || GetGame().IsKindOf(w.m_Car, best.m_Car))
				best = w;
		}
		return best;
	}

	//! How far the wreck's model is turned from the car's
	static float Turn(CarScript car)
	{
		BetterExplosionsWreckType added = Added(car);
		if (added)
			return added.m_Turn;
		return 180;   // the vanilla abandoned-car props
	}

	//! car base type, wreck class prefix, its colours (the first is the default)
	static const ref array<string> CARS = {"OffroadHatchback", "Hatchback_02", "CivilianSedan", "Sedan_02", "Offroad_02", "Truck_01_Base"};
	static const ref array<string> WRECKS = {"Land_Wreck_hb01_aban%1_", "Land_Wreck_hb02_aban%1_", "Land_Wreck_sed01_aban%1_", "Land_Wreck_sed02_aban%1_", "Land_Wreck_offroad02_aban%1", "Land_wreck_truck01_aban%1_"};
	//! what's blown off it (BetterExplosionsDebris: BetterExplosions_<part>Wheel / Hood)
	static const ref array<string> DEBRIS = {"Hatchback", "Hatchback_02_", "CivSedan", "Sedan_02_", "Offroad_02_", "Truck_01_"};
	static const float DEBRIS_SPEED = 9;   //!< m/s at 45 degrees: lands ~8 m off
	static const ref array<string> COLOURS = {"green blue white", "red black blue", "white black wine", "yellow red grey", "", "green orange blue"};

	static int Index(CarScript car)
	{
		for (int i = 0; i < CARS.Count(); i++)
		{
			if (car.IsKindOf(CARS[i]))
				return i;
		}
		return -1;
	}

	//! The wreck class for this car, "" if there isn't one
	static string For(CarScript car)
	{
		BetterExplosionsWreckType added = Added(car);
		if (added)
		{
			array<string> choices = new array<string>;
			added.m_Wreck.Split("|", choices);
			return choices.GetRandomElement();
		}
		for (int i = 0; i < CARS.Count(); i++)
		{
			if (!car.IsKindOf(CARS[i]))
				continue;
			string wreck = string.Format(WRECKS[i], Math.RandomIntInclusive(1, 2));
			if (COLOURS[i] == "")
				return wreck;
			array<string> colours = new array<string>;
			COLOURS[i].Split(" ", colours);
			string type = car.GetType();
			type.ToLower();
			string colour = colours[0];
			foreach (string c : colours)
			{
				if (type.Contains(c))
					colour = c;
			}
			return wreck + colour;
		}
		return "";
	}

	static const ref array<string> PARTS = {"CarWheel", "CarDoor", "CarBattery", "TruckBattery", "SparkPlug", "GlowPlug", "CarRadiator", "HeadlightH7"};

	//! A vehicle part: something that fits one of the car's own slots, or any car's
	static bool IsPart(EntityAI item, array<string> slots)
	{
		if (!item)
			return false;
		foreach (string kind : PARTS)
		{
			if (item.IsKindOf(kind))
				return true;
		}
		array<string> fits = new array<string>;
		item.ConfigGetTextArray("inventorySlot", fits);
		foreach (string f : fits)
		{
			f.ToLower();
			if (slots.Find(f) >= 0)
				return true;
		}
		return false;
	}

	static void Swap(CarScript car)
	{
		if (!car)
			return;
		string type = For(car);
		vector pos = car.GetPosition();
		vector ori = car.GetOrientation();
		vector box[2];
		car.GetCollisionBox(box);
		vector lo = box[0];
		vector hi = box[1];
		float reach = Math.Sqrt((hi[0] - lo[0]) * (hi[0] - lo[0]) + (hi[2] - lo[2]) * (hi[2] - lo[2])) * 0.5;
		// the middle of the car (a helicopter's origin is up by its nose)
		vector mid = car.ModelToWorld(Vector((lo[0] + hi[0]) * 0.5, 0, (lo[2] + hi[2]) * 0.5));

		EntityAI wreck = EntityAI.Cast(GetGame().CreateObjectEx(type, pos, ECE_CREATEPHYSICS | ECE_UPDATEPATHGRAPH));
		if (!wreck)
		{
			Print("[BetterExplosions] no wreck " + type + " for " + car.GetType());
			return;
		}
		// the vanilla wreck models face the other way to the cars
		vector wori = ori;
		wori[0] = wori[0] + Turn(car);
		while (wori[0] > 180)
			wori[0] = wori[0] - 360;
		wreck.SetOrientation(wori);
		// the wreck's middle where the car's was, its bottom on the ground
		if (wreck.GetCollisionBox(box))
		{
			vector wlo = box[0];
			vector whi = box[1];
			vector wmid = wreck.VectorToParent(Vector((wlo[0] + whi[0]) * 0.5, 0, (wlo[2] + whi[2]) * 0.5));
			pos[0] = mid[0] - wmid[0];
			pos[2] = mid[2] - wmid[2];
			pos[1] = GetGame().SurfaceY(pos[0], pos[2]) - wlo[1] - 0.05;
			wreck.SetPosition(pos);
		}
		wreck.Update();

		// its cargo scattered round the wreck. Car parts - wheels, doors, battery, plugs,
		// radiator, headlights - go with the car, fitted or spare
		array<string> slots = new array<string>;
		car.ConfigGetTextArray("attachments", slots);
		foreach (int si, string sl : slots)
		{
			sl.ToLower();
			slots[si] = sl;
		}
		array<EntityAI> items = new array<EntityAI>;
		int parts;
		CargoBase cargo = car.GetInventory().GetCargo();
		if (cargo)
		{
			for (int c = 0; c < cargo.GetItemCount(); c++)
			{
				EntityAI ci = cargo.GetItem(c);
				if (IsPart(ci, slots))
					parts++;
				else
					items.Insert(ci);
			}
		}
		vector m[3];
		float quat[4];
		Math3D.YawPitchRollMatrix(ori, m);
		Math3D.MatrixToQuat(m, quat);
		foreach (EntityAI item : items)
		{
			if (!item)
				continue;
			float ang = Math.RandomFloat(0, Math.PI2);
			float d = reach + Math.RandomFloat(0.3, 2);
			vector at = mid + Vector(Math.Cos(ang) * d, 0, Math.Sin(ang) * d);
			at[1] = GetGame().SurfaceY(at[0], at[2]) + 0.3;
			MiscGameplayFunctions.ThrowEntityFromInventory(item, at, quat, vector.Zero, ThrowEntityFlags.NONE);
		}
		string thrown = "";
		foreach (EntityAI ti : items)
		{
			if (ti)
				thrown += " " + ti.GetType();
		}
		// burnt bits flying: a wheel off the side and the hood off the front (and whatever
		// else an added wreck has) at 45 degrees
		array<string> pieces = new array<string>;
		array<bool> onEdge = new array<bool>;
		BetterExplosionsWreckType added = Added(car);
		if (added)
		{
			pieces.Copy(added.m_Pieces);
			onEdge.Copy(added.m_OnEdge);
		}
		else
		{
			int ci2 = Index(car);
			if (ci2 >= 0)
			{
				pieces.Insert("BetterExplosions_" + DEBRIS[ci2] + "Wheel");
				onEdge.Insert(true);
				pieces.Insert("BetterExplosions_" + DEBRIS[ci2] + "Hood");
				onEdge.Insert(false);
			}
		}
		vector fwd = car.GetDirection();
		fwd[1] = 0;
		fwd.Normalize();
		vector right = Vector(fwd[2], 0, -fwd[0]);
		float sideSign = 1;
		if (Math.RandomFloat01() < 0.5)
			sideSign = -1;
		foreach (int pi, string piece : pieces)
		{
			vector start;
			vector dir;
			if (pi == 0 || pi == 2)        // off one side, then the other
			{
				float sgn = sideSign;
				if (pi == 2)
					sgn = -sideSign;
				start = car.ModelToWorld(Vector(hi[0] * sgn, 0.5, hi[2] * 0.55 - (pi / 2) * (hi[2] - lo[2]) * 0.5));
				dir = vector.RotateAroundZeroDeg(right * sgn, vector.Up, Math.RandomFloat(-25, 25));
			}
			else if (pi == 1)              // off the front
			{
				start = car.ModelToWorld(Vector(0, hi[1], hi[2] * 0.6));
				dir = vector.RotateAroundZeroDeg(fwd, vector.Up, Math.RandomFloat(-35, 35));
			}
			else if (pi == 3)              // off the back
			{
				start = car.ModelToWorld(Vector(0, hi[1], lo[2] * 0.6));
				dir = vector.RotateAroundZeroDeg(fwd * -1, vector.Up, Math.RandomFloat(-35, 35));
			}
			else                           // anywhere
			{
				start = mid + Vector(0, hi[1] - lo[1], 0);
				dir = vector.RotateAroundZeroDeg(fwd, vector.Up, Math.RandomFloat(0, 360));
			}
			vector rest = Vector(Math.RandomFloat(-180, 180), Math.RandomFloat(-8, 8), Math.RandomFloat(-8, 8));
			if (onEdge[pi])
				rest = Vector(Math.RandomFloat(-180, 180), 0, 90);
			BetterExplosionsDebris.Throw(piece, start, dir, DEBRIS_SPEED * Math.RandomFloat(0.85, 1.25), rest);
		}

		// anyone in the wreck's footprint is put down beside it: a body under a wreck can't
		// be got at. Anyone in a seat stays in it (see BetterExplosionsHulk)
		Clear(wreck);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Clear, 1500, false, wreck);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Clear, 5000, false, wreck);
		car.EjectBECrew(BetterExplosionsSettings.EJECT_CHANCE);
		int seated = 0;
		for (int cr = 0; cr < car.CrewSize(); cr++)
		{
			if (car.CrewMember(cr))
				seated++;
		}

		Print("[BetterExplosions] " + car.GetType() + " is now " + type + " at " + wreck.GetPosition() + ", " + parts + " spare parts gone with it, " + seated + " still in it, thrown out:" + thrown);
		if (seated > 0)
			car.MakeBEHulk();
		else
			GetGame().ObjectDelete(car);
	}

	//! put the people and creatures (living or dead) inside a wreck's footprint down beside it
	static void Clear(EntityAI wreck)
	{
		if (!wreck)
			return;
		vector box[2];
		if (!wreck.GetCollisionBox(box))
			return;
		vector lo = box[0];
		vector hi = box[1];
		float reach = vector.Distance(lo, hi);
		array<Object> near = new array<Object>;
		GetGame().GetObjectsAtPosition3D(wreck.GetPosition(), reach, near, null);
		foreach (Object o : near)
		{
			if (!Man.Cast(o) && !DayZCreatureAI.Cast(o))
				continue;
			EntityAI body = EntityAI.Cast(o);
			if (body.GetParent())
				continue;
			vector at = wreck.WorldToModel(body.GetPosition());
			float pad = 0.3;
			if (at[0] < lo[0] - pad || at[0] > hi[0] + pad || at[2] < lo[2] - pad || at[2] > hi[2] + pad)
				continue;
			if (at[1] > hi[1] + 0.5)
				continue;   // standing on the roof
			// out through the nearest side
			float dl = at[0] - lo[0];
			float dr = hi[0] - at[0];
			float db = at[2] - lo[2];
			float df = hi[2] - at[2];
			float least = Math.Min(Math.Min(dl, dr), Math.Min(db, df));
			float clear = 0.7;
			if (least == dl)
				at[0] = lo[0] - clear;
			else if (least == dr)
				at[0] = hi[0] + clear;
			else if (least == db)
				at[2] = lo[2] - clear;
			else
				at[2] = hi[2] + clear;
			vector to = wreck.ModelToWorld(at);
			to = BetterExplosions.Ground(to);
			body.SetPosition(to);
			body.Update();
			string state = "dead";
			if (body.IsAlive())
				state = "alive";
			Print("[BetterExplosions] " + body.GetType() + " (" + state + ") moved out from under " + wreck.GetType() + " to " + to);
		}
	}
}
