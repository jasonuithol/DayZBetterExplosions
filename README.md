# Better Explosions

A DayZ mod: fuelled vehicles and fuel pumps that go up properly. On its own, vanilla's
explosives set vehicles off; other mods can set them off too, through its API
([Molotov](https://github.com/jasonuithol/DayZMolotov) does, when a molotov fire reaches a
fuelled vehicle).

## What sets a vehicle off

A fuelled car, boat or motorbike goes up a moment after a vanilla explosive damages it:

| Explosive | Ammo | When |
|---|---|---|
| Landmine (stepped on, or driven over) | `LandMineExplosion_Ammo`, `LandMineExplosion_CarWheel` | any hit |
| M79's 40 mm explosive round | `Explosion_40mm_Ammo` | any hit |
| C4, and an IED with C4 in it | `Plastic_Explosive_Ammo` | any hit |
| Hand grenade (RGD-5, M67), and an IED with one in it | `RGD5Grenade_Ammo`, `M67Grenade_Ammo` | only within 1.5 m of the vehicle's sides (against it, under it) |

Claymores, flashbangs, gas and fireworks don't. An empty or ruined vehicle doesn't either.
The lists are `TRIGGER_AMMO` and `GRENADE_AMMO` in `BetterExplosionsSettings`. A vehicle's
own blast doesn't set off the next one: chain reactions are up to a fire mod (Molotov's
fire spreads).

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

Afterwards it burns for a minute (`BetterExplosionsBurning`: flames, smoke, light and
sound, 3 m across, 8 health a second to whoever stands in it; it doesn't spread). A mod
with its own fire turns that off and lights its own from `Event_OnExploded` - Molotov
does, whatever set the vehicle off.
Wrecks and flying bits are server-spawned, unsaved: gone at restart. The flying bits are
placed by the server where they come to rest; the clients that see it happen play the
flight (and the sinking) themselves.

## For modders

Server side; require `BetterExplosions` in your `CfgPatches` and load it before your mod.

```c
BetterExplosions.CanExplode(obj);        // a fuelled vehicle, or a working fuel pump
BetterExplosions.Explode(obj);           // it goes up now (false if it can't)
BetterExplosions.BlastPeople(src, pos);  // just the blast's hit on the living
BetterExplosions.Event_OnExploded.Insert(MyHandler);   // void MyHandler(Object what, vector where)
BetterExplosions.s_OwnFire = false;      // don't leave our burning wreck (you light your own)
BetterExplosionsWreck.Add("CarClass", "WreckClass", 180);        // its wreck (turned 180)
BetterExplosionsWreck.Fling("CarClass", "PieceClass", onEdge);   // a bit flung off it
```

`Add`, `Fling`, `s_OwnFire` and the event handler belong in `MissionServer.OnInit`. The wreck may be several classes,
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
| `BetterExplosions/Scripts/4_World/BetterExplosions/` | `BetterExplosions.c` (settings, `Explode`, what sets it off, the blast, the fireball, the burning wreck), `BetterExplosionsTriggers.c` (vehicles hearing about explosive hits), `BetterExplosionsWreck.c` (cars to wrecks), `BetterExplosionsDebris.c` (flying and sinking bits, boats, motorbikes), `BetterExplosionsPain.c` |
| `BetterExplosions/data/` | The burnt boat hull, outboard and rotor blade (MLOD, binarized by `tools/build.sh`) |
| `BetterExplosions_Expansion/` | The Expansion add-on |
| `mission/` | The test bed (`missions/betterexplosions.chernarusplus` in DayZMissions) |
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

[`mission/`](mission) is the test bed, linked into DayZMissions as
`missions/betterexplosions.chernarusplus` (Better Explosions alone, no Molotov). Players
spawn on Molotov's test beach with landmines, grenades, C4 with remote detonators, an IED
and an M79 with 40 mm explosive rounds at their feet, one of every vanilla vehicle (fuelled)
nose to tail behind them and three boats at the water's edge. `tools/test.sh
betterexplosions` sets a different explosive off against each kind of vehicle (landmine,
40 mm, C4, IED with C4, grenade against it, grenade 4 m off - which mustn't - landmine by a
motorbike, 40 mm by a boat) and checks which went up, left their wreck and burn
(`[BXTest]`). Molotov's test beds (`molotov`, `molotov_expansion`) cover the wrecks, the
flying bits and the blast's damage in more detail.

## Releasing

Tag (`git tag -a v0.1 -m "Better Explosions 0.1"`), push, `tools/build.sh`, then from
DayZMissions `tools/workshop_publish.sh BetterExplosions <steam login>` (twice the first
time: the first run creates the item). Same for `BetterExplosions_Expansion`. On each
item's Workshop page, add the required items: Better Explosions for the add-on and for
Molotov; DayZ-Expansion-Vehicles for the add-on.
