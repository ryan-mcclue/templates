// SPDX-License-Identifier: zlib-acknowledgement

#define has_flags_any(field, flags) (!!((field) & (flags)))
#define has_flags_all(field, flags) (((field) & (flags)) == (flags))

struct Node
{
  Node *next;
  Node *prev;

  int x;
};

// IMPORTANT(Ryan): Macro wrapper necessary for C++ version as no implicit void * casts
// Cryptic expression comma chaining necessary for returning a value
#define DLL_PUSH_BACK(first, last, node, type) (type *)dll_push_back(first, last, node)

INTERNAL void *
dll_push_back(void *first, void *last, void *node)
{
  if (first == NULL) {
    first = last = node;
    node->next = node->prev = NULL;
  } else {
    node->prev = last;
    node->next = NULL;
    last->next = node;
    last = node;
  }
}

#define DLL_PUSH_FRONT(first, last, node) 
    if (first == NULL) {
     first = last = node;
     node->next = node->prev = NULL;
    } else {
     node->prev = NULL;
     node->next = first;
     first->prev = node;
     first = node;
    }

#define DLL_REMOVE(first, last, node) \
    if (node == first) {
      if (first == last) {
        first = last = NULL;
      } else {
        first = first->next;
        first->prev = NULL;
      }
    } else if (node == last) {
      last = last->prev;
      last->next = NULL;
    } else {
      node->next->prev = node->prev;
      node->prev->next = node->next;
    }

#define SLLQueuePush(first,l,n) 
  if (first == NULL) {
    first=last=node:\
    //node->next = NULL;
  } else {
   last->next=node;
   last=node;\
   node->next = NULL;
  }
#define SLLQueuePush(f,l,n) SLLQueuePush_N(f,l,n,next)

#define SLLQueuePushFront_N(f,l,n,next) ((f)==0?\
((f)=(l)=(n),(n)->next=0):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_N(f,l,n,next)

#define SLLQueuePop_N(f,l,next) 
  if (first == last) {
    first = last = NULL;
  } else {
    first = first->next;
  }

int
main(int argc, char *argv[])
{
  IGNORED(argc);
  IGNORED(argv);

  Node nodes[10] = {};
  Node *first = NULL, *last = NULL;
  DLL_PUSH_BACK(first, last, &nodes[i]);

  for (Node *node = first; node != NULL; node = node->next)
  {

  }

  return 0;
}
