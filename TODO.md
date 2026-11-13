# Next release

- Survivors of the blast get the same chance to be thrown out of an exploding car as
  the dead (`BetterExplosionsSettings.EJECT_CHANCE`, now only the dead are thrown out:
  `CarScript.EjectBECrew`). A living player's vehicle command runs on their own client, so
  this needs the server to tell that client to knock them out of the seat
  (`HumanCommandVehicle.KnockedOutVehicle` / `JumpOutVehicle`) rather than moving them
  server-side.
