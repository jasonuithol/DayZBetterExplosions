/**
 * Whoever was sitting in a car when it went up stays in their seat, so the vanilla
 * "pull out body" works on them as it does in any car. The car itself is kept on as a
 * hulk - out of sight inside its wreck, held still, not to be got into - until its
 * seats are empty, and is then deleted. A body that has slumped out of its seat isn't
 * seated any more: it is let go, to lie where it fell.
 */
modded class CarScript
{
	protected bool m_BEHulk;

	void CarScript()
	{
		RegisterNetSyncVariableBool("m_BEHulk");
	}

	bool IsBEHulk()
	{
		return m_BEHulk;
	}

	//! keep this car (with someone in it) on, hidden, inside the wreck it turned into
	void MakeBEHulk()
	{
		m_BEHulk = true;
		HideBEHulk();
		// held where it is: it sits in the wreck, which would otherwise shove it about
		// (and the bodies in it)
		dBodyActive(this, ActiveState.INACTIVE);
		dBodyDynamic(this, false);
		// marked as a hulk in a way that's kept over a restart (a car's fluids are saved)
		int marker = BEMarkerFluid();
		if (marker >= 0)
		{
			LeakAll(marker);
			Fill(marker, GetFluidCapacity(marker) * BE_HULK_MARK);
		}
		SetSynchDirty();
		for (int c = 0; c < CrewSize(); c++)
		{
			Human crew = CrewMember(c);
			if (!crew)
				continue;
			vector seat[4];
			CrewTransformWS(c, seat);
			Print("[BetterExplosions] " + crew.GetType() + " in seat " + c + " of the " + GetType() + " hulk, alive " + crew.IsAlive() + ", " + vector.Distance(crew.GetPosition(), seat[3]) + " m off the seat");
		}
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(WatchBEHulk, 2000, true);
	}

	//! as it goes up, each of the dead in it may be thrown out (chance 0..1): out through
	//! their own side, 1.5-3 m, to come down beside the wreck
	void EjectBECrew(float chance)
	{
		vector box[2];
		GetCollisionBox(box);
		vector hi = box[1];
		for (int i = 0; i < CrewSize(); i++)
		{
			Human body = CrewMember(i);
			if (!body || body.IsAlive())
				continue;
			if (Math.RandomFloat01() >= chance)
				continue;
			vector seat[4];
			CrewTransform(i, seat);
			vector at = seat[3];
			float side = 1;
			if (at[0] < 0)
				side = -1;
			at[0] = side * (hi[0] + Math.RandomFloat(1.5, 3));
			at[2] = at[2] + Math.RandomFloat(-0.8, 0.8);
			vector to = BetterExplosions.Ground(ModelToWorld(at));
			to[1] = to[1] + 0.5;
			CrewGetOut(i);
			body.UnlinkFromLocalSpace();
			body.SetPosition(to);
			body.SetOrientation(Vector(Math.RandomFloat(-180, 180), 0, 0));
			Print("[BetterExplosions] " + body.GetType() + " thrown out of the " + GetType() + " to " + to);
		}
	}

	//! the car and what's on it (doors, wheels) out of sight
	protected void HideBEHulk()
	{
		SetInvisible(true);
		for (int i = 0; i < GetInventory().AttachmentCount(); i++)
		{
			EntityAI part = GetInventory().GetAttachmentFromIndex(i);
			if (part)
				part.SetInvisible(true);
		}
	}

	//! whether the body in seat i is still sitting in it (not slumped out of the car)
	bool IsBESeated(int i)
	{
		Human body = CrewMember(i);
		if (!body)
			return false;
		vector seat[4];
		CrewTransformWS(i, seat);
		return vector.Distance(body.GetPosition(), seat[3]) < 1.0;
	}

	protected void WatchBEHulk()
	{
		int seated = 0;
		for (int i = 0; i < CrewSize(); i++)
		{
			Human body = CrewMember(i);
			if (!body)
				continue;
			if (!body.IsAlive() && !IsBESeated(i))
			{
				// out of the seat: let go of it, to lie where it fell
				Print("[BetterExplosions] " + body.GetType() + " fell out of the " + GetType() + " hulk, let go at " + body.GetPosition());
				CrewGetOut(i);
				body.UnlinkFromLocalSpace();
				continue;
			}
			seated++;
		}
		if (seated > 0)
			return;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(WatchBEHulk);
		Print("[BetterExplosions] " + GetType() + " hulk empty, gone");
		// whoever was pulled out lands by the door, which may be inside the wreck
		vector pos = GetPosition();
		array<Object> near = new array<Object>;
		GetGame().GetObjectsAtPosition(pos, 3, near, null);
		foreach (Object o : near)
		{
			string type = o.GetType();
			if (type.IndexOf("Land_Wreck_") == 0 || type.IndexOf("Land_wreck_") == 0)
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(BetterExplosionsWreck.Clear, 3000, false, EntityAI.Cast(o));
		}
		GetGame().ObjectDelete(this);
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		if (m_BEHulk)
			HideBEHulk();
	}

	override bool CanGetIn()
	{
		if (m_BEHulk)
			return false;
		return super.CanGetIn();
	}

	override bool IsInventoryVisible()
	{
		if (m_BEHulk)
			return false;
		return super.IsInventoryVisible();
	}

	//! the hulk mark: this odd a fraction of a fluid (no car is left with exactly that)
	static const float BE_HULK_MARK = 0.0137;

	//! the fluid the hulk mark goes in: the first of brake fluid, oil, coolant it has
	protected int BEMarkerFluid()
	{
		array<int> fluids = {CarFluid.BRAKE, CarFluid.OIL, CarFluid.COOLANT};
		foreach (int f : fluids)
		{
			if (GetFluidCapacity(f) > 0)
				return f;
		}
		return -1;
	}

	//! after a restart a hulk is back as a plain ruined car (the bodies, and the wreck,
	//! don't come back with it): it goes
	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		if (!GetGame().IsServer() || !IsRuined())
			return;
		int marker = BEMarkerFluid();
		if (marker < 0 || Math.AbsFloat(GetFluidFraction(marker) - BE_HULK_MARK) > 0.0005)
			return;
		Print("[BetterExplosions] " + GetType() + " hulk left over from before the restart, gone");
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().ObjectDelete, 1000, false, this);
	}
}

//! the vanilla pull-out wants a way through the door: a hulk's doors are shut but out of
//! sight, so a body still sitting in one can be pulled out regardless
modded class ActionPullBodyFromTransport
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		PlayerBase body = PlayerBase.Cast(target.GetObject());
		if (body)
		{
			CarScript hulk = CarScript.Cast(body.GetParent());
			if (hulk && hulk.IsBEHulk())
			{
				if (body.IsAlive())
					return false;
				int seat = hulk.CrewMemberIndex(body);
				return seat >= 0 && hulk.IsBESeated(seat);
			}
		}
		return super.ActionCondition(player, target, item);
	}
}
