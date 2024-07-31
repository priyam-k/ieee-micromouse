# ieee-micromouse
ieee micromouse repo for 2024-25 yr
  

## maze_setup.py
- a tool for generating maze layouts easily and exporting to floodfill visualizer
- maze is represented by a 2d array of cells
- each cell is represented by a dictionary that contains the top, left, bottom, and right walls (whether they are walls or not)
- controls:
	- drag to move around
	- scroll to zoom
	- arrow keys to resize maze grid
	- click a wall to toggle it on/off (some hitboxes overlap so be careful ig)
	- esc: exit
	- R: reset maze dimensions to 16x16
	- C: clear all walls in maze
	- F: fill all walls in maze
	- B: fill in borders in maze
	- G: generate a random maze (using DFS)

## floodfilltest.py
- not implemented yet
- will be a testing thing to try out floodfill
- will use same base as maze_setup (drag + zoom, same maze representation)