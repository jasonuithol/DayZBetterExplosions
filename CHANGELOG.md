# Changelog

## 0.1 - 2026-11-13

First release: the vehicle explosions from the Molotov mod (0.4 to 0.5), on their own.

- Vanilla explosives set fuelled vehicles off: landmines, the M79's 40 mm explosive round,
  C4 (and IEDs with C4), and hand grenades going off right against or under a vehicle.
- A vehicle that's gone up burns for a minute, hurting whoever stands in it (not when
  another mod lights its own fire: `s_OwnFire`, `Event_OnExploded`).

- `BetterExplosions.Explode(obj)` blows up a fuelled vehicle or a working fuel pump:
  vanilla's fuel pump blast and sound over it, a fireball, everything in it badly damaged,
  and a real hit on the living around (up to 60 health and shock within 2 m, nothing at
  8 m; infected and animals 2.5x), with a flinch, a cry and the red flash.
- Cars become vanilla's wreck of the same model and colour, cargo thrown out round it, a
  burnt wheel and the hood flying off. Boats lose their outboard (it flies off and sinks)
  and their burnt hull sinks. Motorbikes fly apart.
- `BetterExplosionsWreck.Add` / `Fling` for more vehicles' wrecks and flying bits (the
  Better Explosions - Expansion Vehicles add-on uses them for DayZ Expansion's vehicles).
