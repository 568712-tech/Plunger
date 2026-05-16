# Roblox-Style 3D Game Engine — Project Outline
### Built with Native OpenGL + SFML | C++

---

## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Technology Stack & Architecture](#2-technology-stack--architecture)
3. [Phase 1 — Windowing & OpenGL Context](#3-phase-1--windowing--opengl-context)
4. [Phase 2 — Core 3D Renderer](#4-phase-2--core-3d-renderer)
5. [Phase 3 — Scene Graph & Asset Pipeline](#5-phase-3--scene-graph--asset-pipeline)
6. [Phase 4 — Physics & Collision](#6-phase-4--physics--collision)
7. [Phase 5 — Character Controller & Platforms](#7-phase-5--character-controller--platforms)
8. [Phase 6 — Animation System](#8-phase-6--animation-system)
9. [Phase 7 — PvP & Game Logic](#9-phase-7--pvp--game-logic)
10. [Phase 8 — UI & HUD](#10-phase-8--ui--hud)
11. [Phase 9 — Networking (Multiplayer PvP)](#11-phase-9--networking-multiplayer-pvp)
12. [Phase 10 — Polish & Release](#12-phase-10--polish--release)
13. [Project Structure](#13-project-structure)
14. [Milestones & Timeline](#14-milestones--timeline)
15. [References & Resources](#15-references--resources)

---

## 1. Project Overview

A from-scratch 3D game engine and Roblox-style game written in **C++** using:
- **OpenGL 3.3+ Core Profile** — all rendering
- **SFML** — window creation, input, audio, and clock
- **GLM** — mathematics (vectors, matrices, quaternions)
- **Custom engine systems** — physics, animation, PvP game logic

### Goals
- Blocky, voxel-adjacent visual style reminiscent of Roblox
- Rigid-body physics with AABB/OBB collision
- Platform traversal (jumping, moving platforms, ramps)
- Skeletal animation for characters
- Real-time PvP combat (local split-screen or networked)
- Modular, documented codebase that can be extended

---

## 2. Technology Stack & Architecture

| Layer | Technology |
|---|---|
| Language | C++17 or C++20 |
| Window / Input / Audio | SFML 2.6+ |
| OpenGL Loader | GLAD (generated for GL 3.3 Core) |
| Math Library | GLM 0.9.9+ |
| Image Loading | stb_image (header-only) |
| Font Rendering | FreeType via SFML |
| Networking (optional) | ENet or SFML Sockets |
| Build System | CMake 3.20+ |
| IDE | Any (VS Code, CLion, Visual Studio) |

### High-Level Architecture

```
Engine
├── Core          (loop, clock, event pump)
├── Renderer      (OpenGL calls, shaders, framebuffers)
├── Scene         (scene graph, entity-component)
├── Physics       (broadphase, narrowphase, dynamics)
├── Animation     (skeleton, blending, state machine)
├── Input         (SFML event wrapper)
├── Audio         (SFML sound system)
├── UI            (immediate-mode HUD overlay)
├── Network       (optional: client/server sockets)
└── Game          (Roblox-style game logic, PvP)
```

---

## 3. Phase 1 — Windowing & OpenGL Context

**Goal:** Get an OpenGL window up, clear the screen, handle input.

### Tasks
- [ ] Set up CMake project with SFML and GLAD
- [ ] Create `sf::Window` with `sf::ContextSettings` requesting OpenGL 3.3 Core
- [ ] Initialize GLAD after context creation
- [ ] Implement the main game loop:
  - Fixed timestep update (e.g. 60 Hz physics)
  - Variable render step with interpolation
- [ ] Poll SFML events: close, resize, keyboard, mouse
- [ ] Log OpenGL version, vendor, renderer on startup

### Key Concepts
- `sf::ContextSettings` for depth buffer (24-bit), stencil (8-bit), antialiasing
- `glViewport` on resize
- `glEnable(GL_DEPTH_TEST)` and `glClear` each frame

---

## 4. Phase 2 — Core 3D Renderer

**Goal:** Render textured, lit 3D geometry with a camera.

### 4.1 Shader System
- [ ] `Shader` class: load/compile/link vert + frag GLSL
- [ ] Uniform setters: `setMat4`, `setVec3`, `setFloat`, `setInt`
- [ ] Support multiple shader programs (geometry, shadow, UI, post-process)

### 4.2 Mesh & Buffer Management
- [ ] `Mesh` class wrapping VAO / VBO / EBO
- [ ] Vertex layout: `position (vec3)`, `normal (vec3)`, `texcoord (vec2)`, `tangent (vec3)`
- [ ] Primitive generators: cube, sphere, cylinder, plane, capsule
- [ ] Index buffer support for shared vertices

### 4.3 Camera
- [ ] `Camera` class with position, yaw, pitch
- [ ] View matrix via `glm::lookAt`
- [ ] Perspective projection via `glm::perspective`
- [ ] Third-person follow camera (offset behind character, collision-aware)
- [ ] Frustum computation for culling

### 4.4 Lighting (Phong / Blinn-Phong)
- [ ] Directional light (sun)
- [ ] Point lights (up to 8, passed as uniform arrays)
- [ ] Material: ambient, diffuse, specular, shininess
- [ ] Texture sampling: albedo map, optional normal map

### 4.5 Shadow Mapping
- [ ] Depth framebuffer (FBO + depth texture)
- [ ] Render scene from light's perspective into shadow map
- [ ] PCF soft shadows in main pass

### 4.6 Texture System
- [ ] `Texture` class: load PNG/JPG via stb_image, upload to GL
- [ ] Mipmaps, anisotropic filtering
- [ ] Texture atlas support for block faces (Roblox-style)

### 4.7 Skybox
- [ ] Cubemap texture loader
- [ ] Skybox VAO rendered with depth write disabled, depth func `GL_LEQUAL`

---

## 5. Phase 3 — Scene Graph & Asset Pipeline

**Goal:** Organize the world into a manageable hierarchy.

### 5.1 Entity-Component System (ECS)
- [ ] `Entity`: unique ID + component bitmask
- [ ] Components (plain data structs): `Transform`, `MeshRenderer`, `RigidBody`, `Collider`, `Health`, `PlayerController`, `AnimationState`
- [ ] Systems iterate over entities matching a component mask

### 5.2 Transform Hierarchy
- [ ] Local / world transform (TRS decomposition)
- [ ] Parent-child relationships (for skeleton bones, attachments)
- [ ] Dirty flag propagation

### 5.3 Model Loading
- [ ] OBJ loader (hand-written or tinyobjloader)
- [ ] GLTF 2.0 loader (tinygltf) for skinned meshes
- [ ] Skeleton extraction from GLTF joints
- [ ] Material binding from GLTF materials

### 5.4 Roblox-Style Block World
- [ ] `Part` primitive: axis-aligned colored/textured box
- [ ] `PartRenderer`: batch-render all parts in one draw call (instanced rendering with `glDrawElementsInstanced`)
- [ ] Instance data: model matrix + color packed into VBO updated per frame
- [ ] Map format: JSON or binary describing parts, positions, sizes, colors

---

## 6. Phase 4 — Physics & Collision

**Goal:** Simulate rigid-body dynamics with stable collision response.

### 6.1 Broadphase
- [ ] AABB (Axis-Aligned Bounding Box) per entity
- [ ] Sweep-and-prune or spatial hash grid for broad collision pairs
- [ ] Frustum culling integration

### 6.2 Narrowphase
- [ ] AABB vs AABB overlap test + penetration vector (MTV)
- [ ] OBB (Oriented Bounding Box) via Separating Axis Theorem (SAT)
- [ ] Capsule vs AABB for character collision
- [ ] Sphere vs sphere, sphere vs AABB

### 6.3 Rigid Body Dynamics
- [ ] `RigidBody` component: mass, velocity, angular velocity, restitution, friction
- [ ] Integration: semi-implicit Euler
- [ ] Forces: gravity, applied impulse
- [ ] Torque and angular momentum (optional for blocks)
- [ ] Sleep / wake system to skip idle bodies

### 6.4 Collision Response
- [ ] Impulse-based resolution (coefficient of restitution + friction)
- [ ] Depenetration correction (position correction to avoid sinking)
- [ ] Contact manifold for stable stacking
- [ ] Static bodies: infinite mass, never moved

### 6.5 Triggers & Events
- [ ] Trigger volumes (overlap, no physical response)
- [ ] Collision event callbacks: `onCollisionEnter`, `onCollisionStay`, `onCollisionExit`

---

## 7. Phase 5 — Character Controller & Platforms

**Goal:** Fluid, game-feel character movement and platform mechanics.

### 7.1 Character Controller
- [ ] Capsule collider for the player character
- [ ] Move-and-slide: iterative collision resolution that slides along surfaces
- [ ] Ground detection: raycast or shape-cast downward, store `isGrounded`
- [ ] Slope handling: max walkable angle, slide down steep slopes
- [ ] Step climbing: small obstacles (<= step height) are automatically climbed

### 7.2 Movement
- [ ] WASD walk / run with acceleration and deceleration curves
- [ ] Jump with variable height (hold to jump higher)
- [ ] Double jump (unlock via game logic)
- [ ] Coyote time (brief grace period for jumping after walking off ledge)
- [ ] Jump buffering (queued jump input before landing)
- [ ] Air control (reduced horizontal influence while airborne)

### 7.3 Platform Types
- [ ] **Static platforms**: solid geometry, never moves
- [ ] **Moving platforms**: translate on a path, carry the player via velocity inheritance
- [ ] **Rotating platforms**: spin on an axis, impart angular motion
- [ ] **Falling platforms**: trigger on contact, delay, then fall away
- [ ] **Bouncy platforms**: high restitution, catapult player upward
- [ ] **Kill bricks**: trigger `onDeath` immediately on contact

### 7.4 Camera Integration
- [ ] Third-person camera follows character, orbits with mouse
- [ ] Camera collision: push camera forward if blocked by geometry
- [ ] First-person toggle (optional)

---

## 8. Phase 6 — Animation System

**Goal:** Skeletal animation with blending for characters.

### 8.1 Skeleton & Skinning
- [ ] `Bone`: name, parent index, inverse bind pose matrix
- [ ] `Skeleton`: flat bone array, bind-pose hierarchy
- [ ] Skinning shader: upload up to 100 bone matrices as `uniform mat4 uBones[100]`
- [ ] Vertex attributes: `boneIndices (ivec4)`, `boneWeights (vec4)`
- [ ] GPU skinning in vertex shader

### 8.2 Keyframe Animation
- [ ] `AnimationClip`: array of channels (one per bone), each channel is keyframe list
- [ ] Keyframe interpolation: linear (LERP) for position/scale, SLERP for rotation quaternions
- [ ] Sample clip at time `t` to produce local bone transforms

### 8.3 Animation State Machine
- [ ] States: `Idle`, `Walk`, `Run`, `Jump`, `Fall`, `Attack`, `Hit`, `Die`
- [ ] Transitions with conditions (e.g. `speed > 0.1` → Walk)
- [ ] Blend time: cross-fade between clips over N frames
- [ ] Additive layer: upper-body attack on top of lower-body locomotion

### 8.4 Inverse Kinematics (Optional)
- [ ] Two-bone IK for foot placement on slopes
- [ ] FABRIK solver for arm reach

---

## 9. Phase 7 — PvP & Game Logic

**Goal:** A playable Roblox-style PvP game mode.

### 9.1 Health & Damage System
- [ ] `Health` component: current HP, max HP, armor
- [ ] Damage events: source entity, damage amount, damage type
- [ ] Death handling: ragdoll trigger or respawn
- [ ] Respawn: fade out, teleport to spawn point, fade in

### 9.2 Combat
- [ ] **Melee**: hitbox activation on attack animation frame window
- [ ] **Ranged**: ray-cast or projectile entity from weapon muzzle
- [ ] Knockback: apply impulse on hit
- [ ] Hit indicator: brief flash on hit entity
- [ ] Cooldowns: attack rate limited per player

### 9.3 Weapons
- [ ] Sword / sword-equivalent (melee, hitbox sweep)
- [ ] Blaster (projectile, spawned `Projectile` entity with velocity)
- [ ] Tool equip / unequip system (attach mesh to hand bone)
- [ ] Ammo / energy system

### 9.4 Game Modes
- [ ] **Free-for-All**: last player standing wins
- [ ] **Team Deathmatch**: two teams, score limit
- [ ] **Obby (Obstacle Course)**: race to the finish, fall = respawn at last checkpoint
- [ ] Round timer, score tracking, win condition evaluation

### 9.5 Spawning & Teams
- [ ] Spawn points placed in world, team-tagged
- [ ] Spawn protection (brief invincibility on respawn)
- [ ] Team color applied to character material

---

## 10. Phase 8 — UI & HUD

**Goal:** In-game heads-up display and menus.

### 10.1 Immediate-Mode UI Renderer
- [ ] Orthographic 2D projection for UI pass
- [ ] Quad batcher: colored / textured quads in a single draw call
- [ ] Text rendering: FreeType atlas baked to texture, glyph quads

### 10.2 HUD Elements
- [ ] Health bar (filled quad, color-coded)
- [ ] Kill feed (scrolling text overlay)
- [ ] Score / timer display
- [ ] Crosshair
- [ ] Minimap (top-down render to texture, displayed as HUD quad)

### 10.3 Menus
- [ ] Main menu: Play, Settings, Quit
- [ ] Pause menu (overlay, resume / restart / quit)
- [ ] Settings: resolution, fullscreen, mouse sensitivity, audio volume
- [ ] End-of-round scoreboard

---

## 11. Phase 9 — Networking (Multiplayer PvP)

**Goal:** Online multiplayer using a client-server model.

### 11.1 Architecture
- [ ] Dedicated server binary (headless, no renderer)
- [ ] Client connects via TCP/UDP (ENet recommended for reliable + unreliable channels)
- [ ] Server is authoritative: all physics & game logic run server-side

### 11.2 Protocol
- [ ] Define message types: `PlayerInput`, `WorldSnapshot`, `SpawnEntity`, `DestroyEntity`, `DamageEvent`, `ChatMessage`
- [ ] Serialize with a compact binary format (hand-rolled or use `cereal` / `flatbuffers`)
- [ ] Reliable channel: game events (spawns, deaths, damage)
- [ ] Unreliable channel: position snapshots (high-frequency, loss-tolerant)

### 11.3 Client-Side Prediction & Reconciliation
- [ ] Client applies own input immediately (no wait for server)
- [ ] Server sends authoritative state; client reconciles if diverged
- [ ] Entity interpolation: smooth remote player positions between snapshots

### 11.4 Lag Compensation
- [ ] Server rewinds world state for hit detection (compensates for client latency)

> **Note:** Networking is the most complex phase. Consider implementing local split-screen PvP first and adding networking later.

---

## 12. Phase 10 — Polish & Release

**Goal:** A shippable, playable game.

### Visual Polish
- [ ] Post-processing FBO pipeline: HDR tonemapping, bloom, FXAA
- [ ] Particle system: CPU particles (sparks, hit effects, explosions)
- [ ] Ambient occlusion (SSAO)
- [ ] Dynamic day/night cycle (directional light rotation)

### Audio
- [ ] SFML `SoundBuffer` / `Sound` for SFX (jump, land, hit, shoot)
- [ ] `Music` stream for background music
- [ ] 3D positional audio (attenuated by distance)

### Performance
- [ ] Frustum culling (skip rendering off-screen entities)
- [ ] Occlusion culling (optional, portal-based or HZB)
- [ ] Instanced rendering for repeated geometry (blocks, trees)
- [ ] Level-of-detail (LOD): swap lower-poly meshes at distance

### Quality of Life
- [ ] In-game map editor (place / move / resize Parts)
- [ ] Console / debug overlay (entity count, FPS, physics step time)
- [ ] Screenshot and video capture hook
- [ ] Configurable key bindings

---

## 13. Project Structure

```
roblox-engine/
├── CMakeLists.txt
├── assets/
│   ├── shaders/
│   │   ├── geometry.vert / .frag
│   │   ├── shadow.vert / .frag
│   │   ├── skinned.vert / .frag
│   │   ├── ui.vert / .frag
│   │   └── postprocess.vert / .frag
│   ├── textures/
│   ├── models/
│   ├── maps/
│   └── audio/
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Engine.h / .cpp
│   │   ├── GameLoop.h / .cpp
│   │   └── Clock.h
│   ├── renderer/
│   │   ├── Shader.h / .cpp
│   │   ├── Mesh.h / .cpp
│   │   ├── Texture.h / .cpp
│   │   ├── Camera.h / .cpp
│   │   ├── Framebuffer.h / .cpp
│   │   ├── Renderer.h / .cpp
│   │   └── InstanceBatch.h / .cpp
│   ├── scene/
│   │   ├── Entity.h
│   │   ├── Components.h
│   │   ├── Scene.h / .cpp
│   │   └── Transform.h / .cpp
│   ├── physics/
│   │   ├── RigidBody.h / .cpp
│   │   ├── Collider.h / .cpp
│   │   ├── PhysicsWorld.h / .cpp
│   │   └── CollisionDetection.h / .cpp
│   ├── animation/
│   │   ├── Skeleton.h / .cpp
│   │   ├── AnimationClip.h / .cpp
│   │   ├── Animator.h / .cpp
│   │   └── StateMachine.h / .cpp
│   ├── input/
│   │   └── InputManager.h / .cpp
│   ├── audio/
│   │   └── AudioManager.h / .cpp
│   ├── ui/
│   │   ├── UIRenderer.h / .cpp
│   │   └── HUD.h / .cpp
│   ├── network/
│   │   ├── Server.h / .cpp
│   │   ├── Client.h / .cpp
│   │   └── Protocol.h
│   └── game/
│       ├── Player.h / .cpp
│       ├── CharacterController.h / .cpp
│       ├── Weapon.h / .cpp
│       ├── GameMode.h / .cpp
│       └── Platform.h / .cpp
├── third_party/
│   ├── glad/
│   ├── glm/
│   ├── stb/
│   ├── tinygltf/
│   └── enet/        (if networking)
└── docs/
    └── architecture.md
```

---

## 14. Milestones & Timeline

| # | Milestone | Deliverable | Est. Duration |
|---|---|---|---|
| 1 | Windowing & GL Context | Colored clearing window | 1–2 days |
| 2 | Textured Cube | Lit, textured rotating cube | 3–5 days |
| 3 | Camera & Scene | Fly-cam, multiple objects | 3–5 days |
| 4 | Block World | Instanced Part renderer, map loading | 1 week |
| 5 | Shadow Mapping | Shadows from directional light | 3–5 days |
| 6 | Physics Engine | Blocks falling, stacking, collision | 2 weeks |
| 7 | Character Controller | Player moves, jumps, lands | 1 week |
| 8 | Platforms | All platform types working | 1 week |
| 9 | Skeletal Animation | Character walks and jumps | 2 weeks |
| 10 | PvP Combat | Health, melee, ranged, death | 2 weeks |
| 11 | HUD & Menus | Full in-game UI | 1 week |
| 12 | Polish & Audio | SFX, post-process, particles | 1–2 weeks |
| 13 | Networking (opt.) | Online multiplayer | 3–4 weeks |
| **Total** | | **Playable game** | **~4–5 months** |

---

## 15. References & Resources

### OpenGL
- [learnopengl.com](https://learnopengl.com) — comprehensive OpenGL tutorials
- [OpenGL 4.6 Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/)
- [GLSL Specification](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.pdf)

### SFML
- [SFML Documentation](https://www.sfml-dev.org/documentation/)
- [SFML + OpenGL Tutorial](https://www.sfml-dev.org/tutorials/2.6/window-opengl.php)

### Math & Physics
- [GLM Documentation](https://glm.g-truc.net/0.9.9/index.html)
- [Real-Time Collision Detection — Christer Ericson](http://realtimecollisiondetection.net/)
- [Game Physics Engine Development — Ian Millington](https://www.crcpress.com/Game-Physics-Engine-Development/Millington/p/book/9780123819765)

### Animation
- [Skeletal Animation Tutorial — ogldev](https://ogldev.org/www/tutorial38/tutorial38.html)
- [GLTF Specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)

### Architecture
- [Game Engine Architecture — Jason Gregory](https://www.gameenginebook.com/)
- [Data-Oriented Design — Richard Fabian](https://www.dataorienteddesign.com/dodbook/)

### Networking
- [ENet Library](http://enet.bespin.org/)
- [Gaffer on Games — Networking for Games](https://gafferongames.com/)
- [Quake 3 Source Code](https://github.com/id-Software/Quake-III-Arena) — battle-tested reference