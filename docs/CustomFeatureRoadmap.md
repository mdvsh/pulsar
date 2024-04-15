## Custom Feature RoadMap

> for eecs498.7 game engine architecture w24

_high level_: portability, ease-of-use and **faster** engine

aiming for combination of features (in order of importance rn): 
-  Unity-esque engine UI
	- w/ Lua editor support (will try to get simple hot-reloading)
- performance engineering
	- multithreading
	- optimized filesystem interactions
- advanced scripting ? exposing UI widgets through imgui to Lua devs
---
- [x] move away from AG dependencies
- [x] refactor for C++20  (needed for [glaze](https://github.com/stephenberry/glaze): faster, in-memory JSON lib)
	- [ ] JSON --> glaze switch didn't end up working out, too many build errors and breaking conflicts with existing codebase (try again later)
- [x] switch to and implement cmake build system for cross-platform installation
	- [x] drastically cut down on build times and sizes (ext dependencies not bundled together
- [x]  high-DPI, multi-window rendering
- [ ] multi-threading
	- [x] isolate and parallelize physics, renderer, ECS, SceneManager calls with stl's synchronization primitives for single window
		- [x] does give minuscule boost
	- [x] change to a StateMachine architecture and add play,pause,stop functionality
	- [ ] need to debug `WindowManager` for multi-window context switching and correct `SDLevent` polling
- [x] UI
	- [x] design and layout Unity style
	- [x] implement docking window/panels and saving layout changes stored on next boot 
	- [x] setup media assets manager
		- [ ] thumbnail support todo
	- [x] scene hierarchy panel
	- [x] component properties panel
		- [x] add default views for most common ones
		- [x] create new (from scratch or template)
		- [x] 
	- [ ] lua script editor inside engine window
		- [x] integrate [zep](https://github.com/Rezonality/zep)
		- [ ] [fix windowmanager bug before] implement editor panel
			- [ ] _reach goal_ hot reload script
- [ ] perf benchmarks


