//! A machine that goes up where Big Machines put it leaves its wreck right on the spot of the
//! map wreck it replaced - which Big Machines keeps sunk out of sight, sinking anything of that
//! kind found within 0.3 m of the spot every second (BM_Wrecks.Sweep). So a new wreck that
//! close is moved off the spot as soon as it's there: the wreck comes 200 ms after the
//! explosion (BetterExplosions.Event_OnExploded), and this is queued after it, so it runs in
//! the same tick, before the next sweep.
modded class BM_Wrecks
{
	static const float BE_CLEARANCE = 0.6;

	static void BE_OnExploded(Object gone, vector pos)
	{
		if (GetGame().IsServer())
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(BE_Clear, 200, false, pos);
	}

	static void BE_Clear(vector pos)
	{
		array<Object> near = new array<Object>;
		GetGame().GetObjectsAtPosition(pos, 15, near, null);
		foreach (Object o : near)
		{
			if (!o || BM_Machine.Cast(o))
				continue;
			for (int i = 0; i < s_Matches.Count(); i++)
			{
				vector site = s_Positions[i];
				vector at = o.GetPosition();
				if (!Matches(o, s_Matches[i]) || vector.Distance(at, site) >= BE_CLEARANCE)
					continue;
				vector away = at - site;
				away[1] = 0;
				if (away.Length() < 0.01)
					away = o.GetDirection();
				away[1] = 0;
				away.Normalize();
				vector to = site + away * BE_CLEARANCE;
				to[1] = at[1];
				o.SetPosition(to);
				o.Update();
				Print("[BetterExplosions] " + o.GetType() + " moved off the sunk " + s_Matches[i] + " at " + site + " (it would have been sunk with it)");
			}
		}
	}
}
