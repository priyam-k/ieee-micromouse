import random
import pygame
import numpy as np
import math
import json

RESOLUTION = np.array([800, 600])
VIEW_SIZE = 20

pygame.init()
screen = pygame.display.set_mode(RESOLUTION)


width, height = 10, 10
offset = (20, 30)

gray = (100, 100, 100)  # grid color
white = (255, 255, 255)  # wall color

maze = [
    [
        {"top": False, "bottom": False, "left": False, "right": False, "visited": False}
        for _ in range(width)
    ]
    for _ in range(height)
]

MOUSE_DRAG = False
MOUSE_POS = None

pygame.key.set_repeat(300, 50)


def draw_maze():
    for i, row in enumerate(maze):
        for j, cell in enumerate(row):
            x, y = j * VIEW_SIZE + offset[0], i * VIEW_SIZE + offset[1]
            pygame.draw.line(
                screen,
                white if cell["top"] else gray,
                (x, y),
                (x + VIEW_SIZE, y),
                3 if cell["top"] else 1,
            )
            pygame.draw.line(
                screen,
                white if cell["bottom"] else gray,
                (x, y + VIEW_SIZE),
                (x + VIEW_SIZE, y + VIEW_SIZE),
                3 if cell["bottom"] else 1,
            )
            pygame.draw.line(
                screen,
                white if cell["left"] else gray,
                (x, y),
                (x, y + VIEW_SIZE),
                3 if cell["left"] else 1,
            )
            pygame.draw.line(
                screen,
                white if cell["right"] else gray,
                (x + VIEW_SIZE, y),
                (x + VIEW_SIZE, y + VIEW_SIZE),
                3 if cell["right"] else 1,
            )


def line_clicked(start, length, dir, pos):
    HITBOX = VIEW_SIZE / 4
    if dir == "h":
        return (
            start[0] < pos[0] < start[0] + length
            and start[1] - HITBOX < pos[1] < start[1] + HITBOX
        )
    else:
        return (
            start[1] < pos[1] < start[1] + length
            and start[0] - HITBOX < pos[0] < start[0] + HITBOX
        )


def gen_border():
    for i, row in enumerate(maze):
        for j, cell in enumerate(row):
            if i == 0:
                cell["top"] = True
            if i == len(maze) - 1:
                cell["bottom"] = True
            if j == 0:
                cell["left"] = True
            if j == len(row) - 1:
                cell["right"] = True


def resolve_walls():
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


def change_dimensions(new_width, new_height):
    global width, height, maze

    # print(f"changing dimensions from {width}x{height} to {new_width}x{new_height}")

    if new_width < width:
        maze = [row[:new_width] for row in maze]
    elif new_width > width:
        for row in maze:
            row += [
                {
                    "top": False,
                    "bottom": False,
                    "left": False,
                    "right": False,
                    "visited": False,
                }
                for _ in range(new_width - width)
            ]
    if new_height < height:
        maze = maze[:new_height]
    elif new_height > height:
        maze += [
            [
                {
                    "top": False,
                    "bottom": False,
                    "left": False,
                    "right": False,
                    "visited": False,
                }
                for _ in range(new_width)
            ]
            for _ in range(new_height - height)
        ]
    width, height = new_width, new_height
    resolve_walls()


def fill_maze(wall=False):
    global maze
    maze = [
        [
            {"top": wall, "bottom": wall, "left": wall, "right": wall, "visited": False}
            for _ in range(width)
        ]
        for _ in range(height)
    ]


def generate_maze(x, y):
    # Directions for DFS
    DIRECTIONS = [
        ("top", (0, -1)),
        ("bottom", (0, 1)),
        ("left", (-1, 0)),
        ("right", (1, 0)),
    ]
    maze[y][x]["visited"] = True
    random.shuffle(DIRECTIONS)
    for direction, (dx, dy) in DIRECTIONS:
        nx, ny = x + dx, y + dy
        if 0 <= nx < width and 0 <= ny < height and not maze[ny][nx]["visited"]:
            maze[y][x][direction] = False
            maze[ny][nx][
                {"top": "bottom", "bottom": "top", "left": "right", "right": "left"}[
                    direction
                ]
            ] = False
            generate_maze(nx, ny)

def save_maze():
    cleaned_maze = [
        [
            {k: v for k, v in cell.items() if k != "visited"}
            for cell in row
        ]
        for row in maze
    ]
    with open("maze.json", "w") as f:
        json.dump(cleaned_maze, f)

while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()

        if event.type == pygame.MOUSEWHEEL:
            VIEW_SIZE *= math.exp(event.y / 10)

        if event.type == pygame.MOUSEBUTTONDOWN:
            MOUSE_DRAG = True
            MOUSE_POS = pygame.mouse.get_pos()

            if event.button == 1:
                for i, row in enumerate(maze):
                    for j, cell in enumerate(row):
                        x, y = j * VIEW_SIZE + offset[0], i * VIEW_SIZE + offset[1]
                        if line_clicked((x, y), VIEW_SIZE, "h", MOUSE_POS):
                            maze[i][j]["top"] = not maze[i][j]["top"]
                        if line_clicked((x, y + VIEW_SIZE), VIEW_SIZE, "h", MOUSE_POS):
                            maze[i][j]["bottom"] = not maze[i][j]["bottom"]
                        if line_clicked((x, y), VIEW_SIZE, "v", MOUSE_POS):
                            maze[i][j]["left"] = not maze[i][j]["left"]
                        if line_clicked((x + VIEW_SIZE, y), VIEW_SIZE, "v", MOUSE_POS):
                            maze[i][j]["right"] = not maze[i][j]["right"]

        if event.type == pygame.MOUSEBUTTONUP:
            MOUSE_DRAG = False

        if MOUSE_DRAG and event.type == pygame.MOUSEMOTION:
            offset = (offset[0] + event.rel[0], offset[1] + event.rel[1])

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                break

            if event.key == pygame.K_DOWN:
                change_dimensions(width, height + 1)
            if event.key == pygame.K_UP:
                change_dimensions(width, max(height - 1, 1))
            if event.key == pygame.K_LEFT:
                change_dimensions(max(width - 1, 1), height)
            if event.key == pygame.K_RIGHT:
                change_dimensions(width + 1, height)

            if event.key == pygame.K_r:  # reset
                change_dimensions(16, 16)
            if event.key == pygame.K_c:  # clear
                fill_maze()
            if event.key == pygame.K_f:  # fill
                fill_maze(True)
            if event.key == pygame.K_b:  # generate border
                gen_border()
            if event.key == pygame.K_RETURN:  # resolve walls
                resolve_walls()
            if event.key == pygame.K_g:  # generate maze
                fill_maze(True)  # fill maze so it can be carved
                generate_maze(0, 0)
            if event.key == pygame.K_s:
                save_maze()

    screen.fill((0, 0, 0))

    draw_maze()

    pygame.display.flip()
