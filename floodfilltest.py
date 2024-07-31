import pygame
import numpy as np
import math

RESOLUTION = np.array([800, 600])
VIEW_SIZE = 20

pygame.init()
screen = pygame.display.set_mode(RESOLUTION)

maze = []
width, height = 10, 10
offset = (20, 30)

MOUSE_DRAG = False
MOUSE_POS = None

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

    for x in range(width + 1):
        pygame.draw.line(
            screen,
            (255, 255, 255),
            (x * VIEW_SIZE + offset[0], offset[1]),
            (x * VIEW_SIZE + offset[0], height * VIEW_SIZE + offset[1]),
        )
    for y in range(height + 1):
        pygame.draw.line(
            screen,
            (255, 255, 255),
            (offset[0], y * VIEW_SIZE + offset[1]),
            (width * VIEW_SIZE + offset[0], y * VIEW_SIZE + offset[1]),
        )

    for x, y in maze:
        pygame.draw.rect(screen, (255, 255, 255), (x * 20, y * 20, 20, 20))

    pygame.display.flip()
