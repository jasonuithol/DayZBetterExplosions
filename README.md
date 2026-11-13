# Better Explosions

A DayZ mod library: fuelled vehicles and fuel pumps that go up properly. It makes the
explosion; other mods decide when (today: [Molotov](https://github.com/jasonuithol/DayZMolotov),
when a molotov fire reaches a fuelled vehicle).

What going up does:

- vanilla's fuel pump explosion (`LandFuelFeed_Ammo`) goes off just over the vehicle: its
  sound and its blast on objects; a fireball (`BetterExplosionsFireball`);
- the vehicle is ruined and everything in it loses 75% of its health;
- the living take a real hit (vanilla's barely touches a player): 60 health and shock
  within 2 m, falling to nothing at 8 m, 2.5x for infected and animals; players flinch,
  cry out and see the red flash, and a fatal hit holds them at the last sliver of health a
  moment so the display drains before the screen goes black (`BetterExplosionsPain`);
- a **car** (Niva, Golf, Olga, Sarka, Humvee, M3S) becomes vanilla's abandoned wreck of
  the same model and colour, centred where it stood; its cargo is thrown out round it (its
  own parts go with it) and a burnt wheel and its hood fly off at 45 degrees;
- a **boat**'s outboard motor flies off the stern, splashes down and sinks; its burnt,
  burst hull sinks where it was; its cargo goes in the water;
- a **motorbike** flies apart: wheels, fenders, exhaust, shields;
- a **fuel pump** becomes vanilla's ruined pump.

Nothing burns afterwards: the mod that set it off lights its own fire if it wants one.
Wrecks and flying bits are server-spawned, unsaved: gone at restart. The flying bits are
placed by the server where they come to rest; the clients that see it happen play the
flight (and the sinking) themselves.

Planned: vanilla explosives (landmines, the 40 mm grenade launcher, C4, maybe hand
grenades) setting vehicles off on their own, so admins can run Better Explosions without
any other mod.

## For modders

Server side; require `BetterExplosions` in your `CfgPatches` and load it before your mod.

```c
BetterExplosions.CanExplode(obj);        // a fuelled vehicle, or a working fuel pump
BetterExplosions.Explode(obj);           // it goes up now (false if it can't)
BetterExplosions.BlastPeople(src, pos);  // just the blast's hit on the living
BetterExplosionsWreck.Add("CarClass", "WreckClass", 180);        // its wreck (turned 180)
BetterExplosionsWreck.Fling("CarClass", "PieceClass", onEdge);   // a bit flung off it
```

`Add` and `Fling` belong in `MissionServer.OnInit`. The wreck may be several classes,
`"A|B"`, picked at random; `turn` is how far the wreck's model is turned from the car's
(vanilla's wreck props face backwards: 180). The most specific class added wins, ahead of
the built-in table. Pieces are `BetterExplosionsDebris` classes (a config class with a
model, a script class extending `BetterExplosionsDebris`), on the clients too; they fly
off one side, the front, the other side, the back, then anywhere. `onEdge`: the model
stands on edge (a wheel, a door), so it comes down on its side. The tunables are
`BetterExplosionsSettings`.

## Better Explosions - Expansion Vehicles

[`BetterExplosions_Expansion/`](BetterExplosions_Expansion), after BetterExplosions and
Expansion: the tractor, bus, UAZ, Land Rover and UH-1H get the matching vanilla wreck and
fling Expansion's own burnt parts (vanilla's unreleased Ikarus parts for the bus, our
rotor blade for the Huey). It names the vehicles as strings, so it needs nothing of
Expansion's scripts. It replaces the Molotov - Expansion Vehicles add-on, which now just
requires it.

## Layout

| Path | What |
|---|---|
| `BetterExplosions/` | The library (PBO prefix `BetterExplosions`): config, scripts, debris models, Workshop files |
| `BetterExplosions/Scripts/4_World/BetterExplosions/` | `BetterExplosions.c` (settings, `Explode`, the blast, the fireball), `BetterExplosionsWreck.c` (cars to wrecks), `BetterExplosionsDebris.c` (flying and sinking bits, boats, motorbikes), `BetterExplosionsPain.c` |
| `BetterExplosions/data/` | The burnt boat hull, outboard and rotor blade (MLOD, binarized by `tools/build.sh`) |
| `BetterExplosions_Expansion/` | The Expansion add-on |
| `blender/build_debris.py` | Builds the debris models, `models/debris.blend` and a preview |
| `tools/build.sh` | Binarizes and stages `build/BetterExplosions` |

## Building and testing

Built, signed and run with [DayZMissions](https://github.com/jasonuithol/DayZMissions):
`mods/BetterExplosions` links to `build/BetterExplosions`, `mods/BetterExplosions_Expansion`
to the source folder.

```sh
blender --background --python blender/build_debris.py   # only when a model changes
tools/build.sh
```

The test beds are Molotov's (`tools/test.sh molotov` and `molotov_expansion` from
DayZMissions), which blow up every kind of vehicle and check the wrecks, the flying bits,
the sunk boat and the blast's damage.

## Releasing

Tag (`git tag -a v0.1 -m "Better Explosions 0.1"`), push, `tools/build.sh`, then from
DayZMissions `tools/workshop_publish.sh BetterExplosions <steam login>` (twice the first
time: the first run creates the item). Same for `BetterExplosions_Expansion`. On each
item's Workshop page, add the required items: Better Explosions for the add-on and for
Molotov; DayZ-Expansion-Vehicles for the add-on.
