# PyAgent.py

import Action
import Orientation
import Agent
import random.random

agent = Agent.Agent()

def PyAgent_Constructor ():
    print("PyAgent_Constructor")

def PyAgent_Destructor ():
    print("PyAgent_Destructor")

def PyAgent_Initialize ():

def PyAgent_Process (stench,breeze,glitter,bump,scream):
    action = Action.GOFORWARD
    if (stench == 1 and agent.has_arrow):
        action = Action.SHOOT
        agent.has_arrow = False
    else if (glitter == 1):
        action = Action.GRAB
        agent.has_gold = True
    else if (bump == 1):
        action = random.randint(1, 2)
    else if (scream == 1):
        agent.wumpus_dead = True
    return action

def PyAgent_GameOver (score):
    print("PyAgent_GameOver: score = " + str(score))
