import time
import API
import sys

def log(string):
    sys.stderr.write("{}\n".format(string))
    sys.stderr.flush()

cells = [[{"top": False, "bottom": False, "left": False, "right": False} for _ in range(16)] for _ in range(16)]

def is_path(x1, y1, x2, y2):
    global cells
    if x1 == x2:
        if y1 < y2:
            return not cells[y1][x1]["bottom"] and not cells[y2][x2]["top"]
        elif y1 > y2:
            return not cells[y1][x1]["top"] and not cells[y2][x2]["bottom"]
    elif y1 == y2:
        if x1 < x2:
            return not cells[y1][x1]["right"] and not cells[y2][x2]["left"]
        elif x1 > x2:
            return not cells[y1][x1]["left"] and not cells[y2][x2]["right"]
    else:
        return False

def flood_fill(start_x, start_y):
    flood = [[999 for _ in range(16)] for _ in range(16)]  # Initialize flood array with a high number
    flood[start_y][start_x] = 0  # Starting point is distance 0
    queue = [(start_x, start_y)]  # BFS queue initialized with starting point

    # BFS to propagate the flood values
    while queue:
        x, y = queue.pop(0)  # Current cell
        
        # Current distance from the start
        current_distance = flood[y][x]
        
        # Explore neighbors (right, left, top, bottom)
        for dx, dy, direction in [(1, 0, "right"), (-1, 0, "left"), (0, 1, "bottom"), (0, -1, "top")]:
            nx, ny = x + dx, y + dy

            # Check bounds and if the neighbor has already been visited (flood value 999 means not visited)
            if 0 <= nx < 16 and 0 <= ny < 16 and flood[ny][nx] == 999:
                # Check if there's a valid path between (x, y) and (nx, ny)
                if is_path(x, y, nx, ny):
                    flood[ny][nx] = current_distance + 1  # Update flood value for neighbor
                    queue.append((nx, ny))  # Add neighbor to queue to continue BFS

    return flood

def resolve_walls(maze):
    for i, row in enumerate(maze):
        for j, cell in enumerate(row):
            # right wall
            if j < len(row) - 1:
                right_cell = maze[i][j + 1]
                cell["right"] = right_cell["left"] = cell["right"] or right_cell["left"]
            # bottom wall
            if i < len(maze) - 1:
                bottom_cell = maze[i + 1][j]
                cell["bottom"] = bottom_cell["top"] = (
                    cell["bottom"] or bottom_cell["top"]
                )
    return maze

def orient(dir, dxy): # transform dxy by turning by dir
    assert dxy in [(0, -1), (0, 1), (-1, 0), (1, 0)]
    if dir.lower() == "r":
        ndxy = (-dxy[1], dxy[0])
    elif dir.lower() == "l":
        ndxy = (dxy[1], -dxy[0])
    return ndxy

def get_dir(dir, dxy): # gets maze-centric direction from robot-centric direction
    # dir = front, back, left, right
    # return = top, bottom, left, right
    # get direction of robot in relation to maze, given direction in terms of robot and orientation of robot
    assert dxy in [(0, -1), (0, 1), (-1, 0), (1, 0)]
    if dxy == (0, -1): # up
        if dir == "front":
            return "top"
        elif dir == "back":
            return "bottom"
        elif dir == "left":
            return "left"
        elif dir == "right":
            return "right"
    elif dxy == (0, 1): # down
        if dir == "front":
            return "bottom"
        elif dir == "back":
            return "top"
        elif dir == "left":
            return "right"
        elif dir == "right":
            return "left"
    elif dxy == (-1, 0): # left
        if dir == "front":
            return "left"
        elif dir == "back":
            return "right"
        elif dir == "left":
            return "bottom"
        elif dir == "right":
            return "top"
    elif dxy == (1, 0): # right
        if dir == "front":
            return "right"
        elif dir == "back":
            return "left"
        elif dir == "left":
            return "top"
        elif dir == "right":
            return "bottom"

def card(dir): # converts direction to cardinal direction
    if dir == "top":
        return "n"
    elif dir == "bottom":
        return "s"
    elif dir == "left":
        return "w"
    elif dir == "right":
        return "e"

def add(a, b):
    return (a[0] + b[0], a[1] + b[1])

def ibtp(lst, tpl): # index 2d list by tuple of indices
    return lst[tpl[0]][tpl[1]]

def validpos(pos):
    return 0 <= pos[0] < 16 and 0 <= pos[1] < 16

def updateGUIflood(flood, print=False):
    for i in range(len(flood)): 
        for j in range(len(flood[i])):
            API.setText(j, 15-i, str(flood[i][j]))
    
    if print:
        [log(flood[i]) for i in range(len(flood))]

def colorPredictedPath(flood, pos, dxy, taken):
    x, y = pos  # Unpack the current position
    
    # Loop until we reach the goal (flood value = 0)
    while flood[y][x] != 0:
        # Color the current position
        if (x, y) not in taken:
            API.setColor(x, 15 - y, "B")
        else:
            API.setColor(x, 15 - y, "C")
        
        # Track the next position with the lowest flood value
        next_pos = None
        next_flood_value = flood[y][x]  # Current flood value

        # Store possible moves (right, left, bottom, top)
        moves = [(1, 0), (-1, 0), (0, 1), (0, -1)]
        
        # Prioritize the direction the robot is facing (move this direction to the front of the list)
        if dxy in moves:
            moves.remove(dxy)
            moves.insert(0, dxy)  # Move the current direction to the front of the list

        # Check all possible moves
        for dx, dy in moves:
            nx, ny = x + dx, y + dy
            if 0 <= nx < 16 and 0 <= ny < 16:  # Ensure within bounds
                if flood[ny][nx] < next_flood_value and is_path(x, y, nx, ny):  # Find the cell with the lower flood value
                    next_flood_value = flood[ny][nx]
                    next_pos = (nx, ny)

        # If no next position is found, something went wrong
        if next_pos is None:
            log(f"Error: No valid next step from ({x}, {y})")
            break

        # Update the current position to the next position
        x, y = next_pos

        # Optionally, update direction based on chosen move
        dxy = (x - pos[0], y - pos[1])  # Update direction based on movement
        pos = (x, y)  # Update current position
    API.setColor(x, 15 - y, "B")  # Color the goal cell

def colorPaths(flood, pos, dxy, taken):
    API.clearAllColor()
    for p in taken:
        API.setColor(p[0], 15-p[1], "G")
    colorPredictedPath(flood, pos, dxy, taken)
    

def runMouse(pos, dxy, target=(7, 7)):
    global cells
    flood = flood_fill(target[0], target[1])
    updateGUIflood(flood)
    pathTaken = []
    predDist = flood[pos[1]][pos[0]]

    while flood[pos[1]][pos[0]] != 0:
        ## check if we have new data
        ##if cells[pos[1]][pos[0]][get_dir("front", dxy)] != API.wallFront() or cells[pos[1]][pos[0]][get_dir("left", dxy)] != API.wallLeft() or cells[pos[1]][pos[0]][get_dir("right", dxy)] != API.wallRight():
        ##    newData = True

        # update known maze data
        cells[pos[1]][pos[0]][get_dir("front", dxy)] = API.wallFront()
        cells[pos[1]][pos[0]][get_dir("left", dxy)] = API.wallLeft()
        cells[pos[1]][pos[0]][get_dir("right", dxy)] = API.wallRight()
        cells = resolve_walls(cells)
        # update visuals on gui
        if API.wallFront(): API.setWall(pos[0], 15-pos[1], card(get_dir("front", dxy)))
        if API.wallLeft(): API.setWall(pos[0], 15-pos[1], card(get_dir("left", dxy)))
        if API.wallRight(): API.setWall(pos[0], 15-pos[1], card(get_dir("right", dxy)))
        # reset flood
        flood = flood_fill(target[0], target[1])
        # update flood values on gui
        updateGUIflood(flood)
        # color taken path and predicted path
        pathTaken.append(pos)
        colorPaths(flood, pos, dxy, pathTaken)
        # move to cell with lower flood value
        npos = add(pos, dxy) # need to verify that new pos is inside maze
        while not(is_path(pos[0], pos[1], npos[0], npos[1]) and flood[npos[1]][npos[0]] < flood[pos[1]][pos[0]]):
            API.turnRight()
            dxy = orient("r", dxy)
            npos = add(pos, dxy)
            nflood = flood[npos[1]][npos[0]] if validpos(npos) else "invalid"
            log(f"turning right - Position: {pos}, Flood: {flood[pos[1]][pos[0]]}, nflood: {nflood}, path: {is_path(pos[0], pos[1], npos[0], npos[1])}")
            colorPaths(flood, pos, dxy, pathTaken)
        API.moveForward()
        pos = add(pos, dxy)

    flood = flood_fill(target[0], target[1])
    updateGUIflood(flood, True)
    log("Done!")
    return API.getStat("current-run-distance") > predDist, pathTaken, pos, dxy

def sub(a, b):
    return (a[0] - b[0], a[1] - b[1])

def pathToInstr(pos, dxy, path):
    instructions = []
    for i in range(len(path)-1):
        npos = path[i+1]
        ndxy = sub(npos, pos)
        assert is_path(pos[0], pos[1], npos[0], npos[1])
        if ndxy == dxy:
            instructions.append("f")
        elif ndxy == orient("r", dxy):
            instructions.append("r")
            dxy = ndxy
        elif ndxy == orient("l", dxy):
            instructions.append("l")
            dxy = ndxy
        else:
            instructions.append("rr")
            dxy = orient("r", orient("r", dxy))

        pos = npos
    return instructions

def runPath(pos, dxy, path):
    pass
    


def main():
    global cells
    #time.sleep(5)
    log("Running...")
    API.setColor(0, 0, "G")
    API.setText(0, 0, "abc")
    #flood = flood_fill(7, 7)
    #updateGUIflood(flood)
    
    dxy = (0, -1) # facing up
    pos = (0, 15) # starting at bottom left (x:0, y:15)
    notOptimal = True

    while notOptimal:
        notOptimal, path, pos, dxy = runMouse(pos, dxy)
        time.sleep(3)
        notOptimal, path, pos, dxy = runMouse(pos, dxy, (0, 15))
        time.sleep(3)
        #API.ackReset()
    log("\n\n"+"-"*30+"\nOptimal path found!")
    log("Replaying optimal path...")
    time.sleep(2)
    runMouse(pos, dxy)


if __name__ == "__main__":
    main()
