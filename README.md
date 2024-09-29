# ieee-micromouse
ieee micromouse repo for 2024-25 yr

## mms-floodfill.py
- full floodfill algorithm and maze solving implemented
- uses mms tool from https://github.com/mackorone/mms
- API.py has some tools to help communicate between this file and the mms.exe program
- TODO:
	- optimize turns (currently only turns right, should turn left when thats faster)
	- optimize pathing (once it knows final path, it should do long straight parts faster)
	- optimize diagonals (recognize diagonals and go thru at 45 degrees for final path)

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
	- S: save maze to maze.json, to be used with floodfilltest.py

## floodfilltest.py
- uses maze from maze_setup.py (after saving to maze.json)
- currently just shows the flood matrix on top of it
- not super useful tbh