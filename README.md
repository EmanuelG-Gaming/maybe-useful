# Arena allocator
This implements an efficient arena (bump) allocator in C.

# Features:
- An arena gets chained with another arena once its memory gets full, forming a linked list
- OS-dependent virtual memory commiting
- 2 scratch arenas in each thread for temporary things, like in a function
