//! Vanilla explosives hitting a fuelled vehicle set it off (BetterExplosions.OnHit)
modded class CarScript
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		BetterExplosions.OnHit(this, damageType, source, ammo);
	}
}

modded class BoatScript
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		BetterExplosions.OnHit(this, damageType, source, ammo);
	}
}

modded class MotorbikeScript
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		BetterExplosions.OnHit(this, damageType, source, ammo);
	}
}
