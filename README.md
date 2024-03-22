
# Team 5  

This project is available on GitHub : [ishant4iZard/CSC8508 (github.com)](https://github.com/ishant4iZard/CSC8508)

# How to build and run the project

### Platform : Windows 

- Create a folder titled "Build" in the source directory
- Open CMake and set the source directory
- Select the root/Build directory as the destination
- Delete cache
- Configure and generate the project

#### Platform : PS5

- Open cmd in the current directory 
- Execute Generate.bat
- Set CSC8503 as the startup project

----

## ⚠ Please read the following before building or executing the project

1. Steam is **required** to be running to play the game on windows .
2. On windows the first build usually gives a build error, it has something to do with `pdb`s but there should not be any errors with the project.
3. For PS5 : If you make any changes to the projects other than CSC8503 then before building the project please clean it. Otherwise there will be undefined symbols.
4. For PS5 : Textures(gnf/others) are already provided in the compressed project submission. Building the textures project in VS sometimes deletes textures and this might cause the game to crash.

----

# How to play the game

#### Windows

| Key binding     | Action           |
| --------------- | ---------------- |
| Space           | Score table      |
| i               | Debug table      |
| Cursor position | Move player      |
| Left click      | Fire projectiles |

#### PS5

| Key binding          | Action          |
| -------------------- | --------------- |
| Menu : Cross         | Start           |
| Menu : Circle        | Quit            |
| Game : Option        | Pause           |
| Game : R2 trigger    | Fire projectile |
| Game : Left stick    | Rotate          |
| Game : Right stick   | Move            |
| Pause menu : Cross   | Round over      |
| Pause menu : Circle  | Resume          |
| Round over menu : R1 | Go to Menu      |
| Triangle             | Debug table     |

---

# Brief overview of project features

#### Physics engine 
- Static trees are used to store non moving object. This tree does not need to be updated every frame
- Friction was added to the engine 
- The engine has most common collision volumes like : AABBs, Spheres, OBBs and Capsules
- Physics objects have an elasticity value which influences how objects rebound

#### Renderer(OpenGL)
- Physically based rendering(PBR) was added to the project
- The renderer also has gamma correction as well as tone mapping
- Skeletal animations are supported
	- Animation syncing between the client and the server was implemented in one of the branches of the project(skeletal animation branch).
	- However when merging said branch issues were encountered, so the feature was not merged into main.
 - GPU Instancing had been added to the project and it reduced the number of draw calls from around 600 to 300 calls
- Deferred rendering(with PBR) was added to the project but was not merged into main because it would have caused problems in rendering transparent objects
	- It can be tested at the deferred rendering branch
	- It features more than 64 point lights
	- All the point lights are instanced 
- The project supports point light made using spheres and directional lights made using screen quads
#### Renderer(AGC)
- The AGC renderer in the main branch is the base renderer provided in the starter project
- PBR was partially added to the renderer and can be tested in the PS5_ToneMapping branch. But due to it making the scene too dark it was not added to the main branch.

#### Gameplay
- The goal of the game is simple : Get as many projectiles into the target as possible
- The following are some of the gameplay elements :
- Gravity wells/blackholes : These game objects pull projectiles towards their center and if the projectile collides with the center, the projectiles is deactivated.
- Portals : Teleports projectiles from one teleporter to another
- Bounce pads : Bounce pads are static cubes with a elasticity value greater than 1, meaning when a projectile collides with them, the projectile gains energy and is rebounded at a speed greater than the collision speed.
- AI :
	- The AI ***predicts*** the position of the projectiles and tries to intercept it. Once the AI is colliding with the projectiles it reduces the projectile's lifespan drastically. 
	- The AI uses the A* path finding algorithm to navigate to a projectile ***only if*** the nearest projectile to it is not in direct line of sight
	- The navigation grid is dynamically generated at the start of a round by firing rays from the top of the level to the ground.
	- If a ray collides with the ground(i.e an object with the "ground" tag) then the cell corresponding to the ray is movable
- Powerups :
	- The game features 3 types of powerups
	- The ice powerup reduces the friction experienced by projectiles
	- The wind powerup adds a random directional force to the projectiles 
	- The sand powerup increases the friction experienced by projectiles

#### Multiplayer and match making(only on windows)
- The game on windows features a multiplayer experience for up to 4 players 
- In a multiplayer game, one player is made the server and the rest are made clients.
- The physics update takes place only in the server and the clients are responsible for render updates
- After the physics update the server sends the updated entity transforms to the clients
- Game states are all synced between the server and the clients
- Matchmaking :
	- ***Steam*** has been used for matchmaking on windows
	- A player can create a lobby 
	- Other players can find this lobby and join it all without having to rely on hard coded IP addresses
	- If the owner of the lobby leaves the second member of the lobby becomes the owner
	- The owner of a lobby is always made the server when the game starts

#### Other miscellaneous features :
- The game makes use of a multitude of different shaders for rendering elements like portals, blackholes, the center target etc.
- The UI framework uses ImGui on windows to display elements like buttons and texts.
- The audio engine uses FMOD to play looping music as well as trigger music.
- A debug menu is also accessible in the project to view the following information :
	- Frame rate
	- FPS
	- Physics time cost
	- Render time cost
	- Number of collisions
	- Number of game objects
	- Outgoing packets
	- Incoming packets
- The game is multi-threaded :
	- Threads are pooled so that they are not created and deleted every frame
	- Tasks like physics updates, non physics updates and animation updates are enqueued into a queue of tasks
	- A free thread from the thread pool can pick the first task in the queue and process it.
	- Conditional variable, mutex and unique locks are used for synchronising threads.
	- The game runs at around 1000fps
- An event system was added to the project to allow multiple components to communicate with each other without having to share state