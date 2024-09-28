import pygame
import numpy as np
import math
import json

RESOLUTION = np.array([800, 600])
VIEW_SIZE = 20

pygame.init()
screen = pygame.display.set_mode(RESOLUTION)


with open("maze.json", "r") as f:
    maze = json.load(f)
width, height = 10, 10
offset = (20, 30)

gray = (100, 100, 100)  # grid color
white = (255, 255, 255)  # wall color

MOUSE_DRAG = False
MOUSE_POS = None

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

    draw_maze()

    pygame.display.flip()
