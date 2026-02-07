# Maybe-useful
This implements a bunch of half-baked things in C that can be used in areas like game development.

# Features:
- Scene system
- Event system
- Dense-sparse ECS
- Inventory system
- Logging system
- Math library
- Object pool for quickly adding/removing elements in an array
- Physics engine
- Arena allocator
- - An arena gets chained with another arena once its memory gets full, forming a linked list
- - OS-dependent virtual memory commiting
- - 2 scratch arenas in each thread for temporary things, like in a function
