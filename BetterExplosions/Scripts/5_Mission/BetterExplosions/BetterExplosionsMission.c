//! Clients act on the server telling them they were hit by a blast (BetterExplosionsPain)
modded class MissionGameplay
{
	void MissionGameplay()
	{
		DayZGame.Event_OnRPC.Insert(BetterExplosionsPain.OnGameRPC);
	}
}
