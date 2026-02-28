# Minnral

In the verge of total destruction to humanity's home, you are tasked to collect some of the rarest known minerals in the universe. This mineral holds a promising amount of energy due to its peculiarly strong molecular structure, of which will be used to power the XFU-3400 meteor destructor machine to prevent the collapse of planet Earth. How fast can you reach the core of planet (placeholder), while dodging from the hostile environment around you?

## How to Play

You are an exploration robot sent by your humans, as the environment in (placeholder) is too lethal for humans. Use left and right keys (or A and D keys) to move left and right, respectively. Move your cursor to aim at the walls, which enables you to shoot out a hook that allows you to swing—enabling you to move from one place to another quicker. As an addition, you can hurt enemies by hitting them while you swing.

Collect (placeholder) (which appears as (placeholder)) as you go through the obstacles. This mineral is secondary to your main goal of reaching the core and obtaining the rare (placeholder) mineral to power the (placeholder).

## Technical Details

### Game Engine
The game engine used in this game is [Raylib](https://raylib.com).
### Map Generation
This game uses procedural map generation. It starts with a small 50x50 map. Each tile of the small map represents 333x333 of actual rendered pixels. The map is stored as a 50x50 grid of booleans. True marks there's a wall tile, and false otherwise. The generation process is as follows:
1. The bottom row is entirely blank (set to false). This is used as the "core" of the planet.
2. Recursively mark a random row (choose from the third-first row until the third-last row) as false (and then run the same thing for both created sides). This process stops when the number of the available rows to choose from is 1 (just one gap that has to be left). There's a 20% chance that the recursion stops randomly too, to increase the randomesss of the generated map.
3. Make the start and stop of each row random. The starting point will be picked from [0, 49], and the length is chosen at random (minimum of 2). This works from top to bottom. For each of the row, the rows i made such that the x-axis overlaps with the previous row.
4. Create between 1 to 3 connection from one row to the bottom row(s) for all of the existing rows. At least one of them is going to connect to the next row, but the rest may be for other rows at the bottom. This constraint makes sure that there will always be way to go down (or conversely up).

After that, the map is rendered for the player by sampling 80x80

## About
Minnral is made as part of Hack Club Campfire Jakarta 2026 game jam.
