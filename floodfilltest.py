import pygame
import numpy as np
import math
import json

RESOLUTION = np.array([800, 600])
VIEW_SIZE = 20

pygame.init()
screen = pygame.display.set_mode(RESOLUTION)
font = pygame.font.SysFont(None, 24)



with open("maze.json", "r") as f:
    maze = json.load(f)
width, height = 10, 10
offset = (20, 30)

gray = (100, 100, 100)  # grid color
white = (255, 255, 255)  # wall color

MOUSE_DRAG = False
MOUSE_POS = None

def is_path(x1, y1, x2, y2):
    if x1 == x2:
        if y1 < y2:
            return not maze[y1][x1]["bottom"] and not maze[y2][x2]["top"]
        elif y1 > y2:
            return not maze[y1][x1]["top"] and not maze[y2][x2]["bottom"]
    elif y1 == y2:
        if x1 < x2:
            return not maze[y1][x1]["right"] and not maze[y2][x2]["left"]
        elif x1 > x2:
            return not maze[y1][x1]["left"] and not maze[y2][x2]["right"]
    else:
        return False

def flood_fill(x, y):
    flood = [[999 for _ in range(16)] for _ in range(16)]
    flood[x][y] = 0
    queue = [(x, y)]
    while queue:
        x, y = queue.pop(0)
        for dx, dy in [(0, 1), (0, -1), (1, 0), (-1, 0)]:
            nx, ny = x + dx, y + dy
            if 0 <= nx < 16 and 0 <= ny < 16 and is_path(x, y, nx, ny):
                if flood[nx][ny] > flood[x][y] + 1:
                    flood[nx][ny] = flood[x][y] + 1
                    queue.append((nx, ny))
    return flood

def draw_maze(maze, flood):
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
            # Render the flood value inside the cell
            flood_value = flood[i][j]
            text_surface = font.render(str(flood_value), True, (100, 100, 100))
            text_rect = text_surface.get_rect(center=(x + VIEW_SIZE // 2, y + VIEW_SIZE // 2))
            screen.blit(text_surface, text_rect)


while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()

        if event.type == pygame.MOUSEWHEEL:
            VIEW_SIZE *= math.exp(event.y /10)

        if event.type == pygame.MOUSEBUTTONDOWN:
            MOUSE_DRAG = True
            MOUSE_POS = pygame.mouse.get_pos()
        if event.type == pygame.MOUSEBUTTONUP:
            MOUSE_DRAG = False

        if MOUSE_DRAG and event.type == pygame.MOUSEMOTION:
            offset = (offset[0] + event.rel[0], offset[1] + event.rel[1])

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                break
        


    screen.fill((0, 0, 0))
    flood = flood_fill(0, 0)
    draw_maze(maze, flood)

    pygame.display.flip()