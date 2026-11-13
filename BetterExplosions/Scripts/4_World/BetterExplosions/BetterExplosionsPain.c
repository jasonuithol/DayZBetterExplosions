//! A player caught by a blast: the damage (straight to health and shock - clothing soaks
//! up nearly all of a hit), a vanilla heat hit that makes them flinch, a cry of pain, the
//! red flash on their screen, and the health display kept up to date. A hit that would
//! kill holds them at the last sliver of health a moment first, so the display has
//! drained to the bottom before the screen goes black.
class BetterExplosionsPain
{
	static const int RPC_HIT = 0x42455802;   //!< "BEX" 2: server -> the player hit: flash red
	static const int CRY_MS = 1500;            //!< at most one cry of pain this often
	static const int LAST_BREATH_MS = 700;     //!< held at the last sliver of health before dying
	static const float LAST_SLIVER = 0.5;

	protected static ref map<int, int> s_LastCry = new map<int, int>;
	protected static ref array<PlayerBase> s_Dying = new array<PlayerBase>;

	//! Server: a blast hits p
	static void Hit(PlayerBase p, EntityAI source, float health, float shock)
	{
		if (!p || !p.IsAlive() || s_Dying.Find(p) >= 0)
			return;
		bool dies = p.GetHealth("", "Health") - health <= LAST_SLIVER;
		if (dies)
			p.SetHealth("", "Health", LAST_SLIVER);
		else
			p.AddHealth("", "Health", -health);
		p.AddHealth("", "Shock", -shock);
		// the flinch (its own damage is a token 1)
		p.ProcessDirectDamage(DamageType.CUSTOM, source, "", "HeatDamage", "0.5 0.5 0.5", 1);
		int now = GetGame().GetTime();
		int key = p.GetID();
		if (!s_LastCry.Contains(key) || now - s_LastCry.Get(key) >= CRY_MS)
		{
			s_LastCry.Set(key, now);
			p.RequestSoundEvent(EPlayerSoundEventID.TAKING_DMG_HEAVY);
		}
		if (p.GetIdentity())
			GetGame().RPCSingleParam(p, RPC_HIT, null, true, p.GetIdentity());
		if (p.m_NotifiersManager)
			p.m_NotifiersManager.RequestImmediateUpdate(eNotifiers.NTF_HEALTHY);
		if (dies)
		{
			s_Dying.Insert(p);
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(BetterExplosionsPain.Die, LAST_BREATH_MS, false, p);
		}
	}

	static void Die(PlayerBase p)
	{
		s_Dying.RemoveItem(p);
		if (p && p.IsAlive())
		{
			p.SetHealth("", "Health", 0);
			Print("[BetterExplosions] " + p.GetType() + " was killed by a blast");
		}
	}

	//! Client: the server says we were hit - the red flash a hit gives
	static void OnGameRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type != RPC_HIT || GetGame().IsDedicatedServer())
			return;
		PlayerBase p = PlayerBase.Cast(target);
		if (p && p == GetGame().GetPlayer())
			p.SpawnDamageDealtEffect();
	}
}
