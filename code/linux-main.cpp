// SPDX-License-Identifier: zlib-acknowledgement

#define HAS_FLAGS_ANY(field, flags) (!!((field) & (flags)))
#define HAS_FLAGS_ALL(field, flags) (((field) & (flags)) == (flags))

// Cryptic expression comma chaining necessary for returning a value

struct TreeNode
{
  TreeNode *first_child;
  TreeNode *last_child;
  TreeNode *next_sibling;
};

struct Node
{
  Node *next;
  Node *prev;

  int x;
};

INTERNAL void
dll_push_front(void *first, void *last, void *node)
{
  if (first == NULL) 
  {
    first = last = node;
    node->next = node->prev = NULL;
  } 
  else 
  {
    node->prev = NULL;
    node->next = first;
    first->prev = node;
    first = node;
  }
}

INTERNAL void
dll_push_back(void *first, void *last, void *node)
{
  if (first == NULL) 
  {
    first = last = node;
    node->next = node->prev = NULL;
  } 
  else 
  {
    node->prev = last;
    node->next = NULL;
    last->next = node;
    last = node;
  }
}

INTERNAL void
dll_remove(void *first, void *last, void *node)
{
  if (node == first) 
  {
    if (first == last) 
    {
      first = last = NULL;
    } 
    else 
    {
      first = first->next;
      first->prev = NULL;
    }
  } 
  else if (node == last) 
  {
    last = last->prev;
    last->next = NULL;
  } 
  else 
  {
    node->next->prev = node->prev;
    node->prev->next = node->next;
  }
}

// IMPORTANT(Ryan): Macro wrapper necessary for C++ version as no implicit void * casts
#define DLL_PUSH_FRONT(first, last, node, type) \
  dll_push_front((type *)(first), (type *)(last), (type *)(node))
#define DLL_PUSH_BACK(first, last, node, type) \
  dll_push_back((type *)(first), (type *)(last), (type *)(node))
#define DLL_REMOVE(first, last, node, type) \
  dll_remove((type *)(first), (type *)(last), (type *)(node))


INTERNAL void
sll_queue_push(void *first, void *last, void *node)
{
  if (first == NULL) 
  {
    first = last = node;
    node->next = NULL;
  } 
  else 
  {
    node->next = first;
    first = node;
  }
}

INTERNAL void *
sll_queue_pop(void *first, void *last)
{
  void *result = first;

  if (first == last)
  {
    first = last = NULL;
  }
  else
  {
    first = first->next;
  }

  return result;
}

#define SLL_QUEUE_PUSH(first, last, node, type) \
  sll_queue_push((type *)(first), (type *)(last), (type *)(node))
#define SLL_QUEUE_POP(first, last, type) \
  (type *)sll_queue_pop((type *)(first), (type *)(last))

INTERNAL void
sll_stack_push(void *first, void *node)
{
  node->next = first;
  first = node;
}

INTERNAL void *
sll_stack_pop(void *first)
{
  void *result = first;

  if (first != NULL)
  {
    first = first->next;
  }

  return result;
}

#define SLL_STACK_PUSH(first, node, type) \
  sll_stack_push((type *)(first), (type *)(node))
#define SLL_STACK_POP(first, type) \
  (type *)sll_stack_pop((type *)(first))

struct ThreadContext
{
  MemArena *arenas[2];  
  const char *file_name;
  u64 line_number;
};

PER_THREAD ThreadContext *tl_thread_context = NULL;

GLOBAL MemArena *perm_arena = NULL;
GLOBAL String8 linux_initial_path = {};

// IMPORTANT(Ryan): Any arena passed in as parameter is a scratch arena
// Any function has access to the permanent arena
INTERNAL String8
get_linux_path(MemArena *arena)
{
  MemTemp scratch = MemGetScratch(&arena); 

  ReleaseScratch(scratch);
}

int
main(int argc, char *argv[])
{
  IGNORED(argc);
  IGNORED(argv);

  // just inits the 2 memory arenas to say, 8GiB
  ThreadContext thread_context = init_thread_context();
  set_thread_local_context(&thread_context);

  // because this is shared, less than thread local?
  // when to use what?
  perm_arena = mem_arena_alloc(GB(1)); 


  Node nodes[10] = {};
  Node *first = NULL, *last = NULL;
  DLL_PUSH_BACK(first, last, &nodes[i]);

  for (Node *node = first; node != NULL; node = node->next)
  {

  }

  return 0;
}

// UIs exist to transfer information between user and program (decide what is useful information)
// So, when making a UI decision, if requires less information to be sent, then a good one (e.g. button press over typing a long string)
// Also, how quickly the user can enter this information is important to

// use well known design elements, e.g. buttons, boxes
// provide usage hints, e.g. buttons embossed, dynamically animate (in time suitable for human brain) in response to user input

// when creating a good interface, implementation and user must agree on how something is inputted and recieved

// core, buildler (majority of code), escape hatch code

// common UI code API has building and response code segmented, i.e. callbacks or event messages
// also use retained-mode, i.e. individually manage widget lifetime to add/remove from heirarchy

// immediate mode has all this in one spot, with widget hierarchy constructed on every frame
// indentation hierarchy acheived with stack
// hierarchy of widgets, not layouts
// widget rendering in IM is deferred; require frame of delay to perform offline (given all data, solve problem in one) autolayout
