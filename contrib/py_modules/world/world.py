#!/usr/bin/python3

import os
if os.name == 'nt':
    os.add_dll_directory( 'c:/QtPlatz/bin' )

import world
planet = world.world()
planet.set('howdy')
planet.greet()
