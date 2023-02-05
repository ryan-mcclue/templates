// SPDX-License-Identifier: zlib-acknowledgement

typedef enum MD_MessageKind
{
    // NOTE(rjf): @maintenance This enum needs to be sorted in order of
    // severity.
    MD_MessageKind_Null,
    MD_MessageKind_Note,
    MD_MessageKind_Warning,
    MD_MessageKind_Error,
    MD_MessageKind_FatalError,
}
MD_MessageKind;

typedef struct MD_Message MD_Message;
struct MD_Message
{
    MD_Message *next;
    MD_Node *node;
    MD_MessageKind kind;
    MD_String8 string;
    void *user_ptr;
};

typedef struct MD_MessageList MD_MessageList;
struct MD_MessageList
{
    MD_MessageKind max_message_kind;
    // TODO(allen): rename
    MD_u64 node_count;
    MD_Message *first;
    MD_Message *last;
};

typedef struct MD_ParseResult MD_ParseResult;
struct MD_ParseResult
{
    MD_Node *node;
    MD_u64 string_advance;
    MD_MessageList errors;
};

MD_ParseResult parse = MD_ParseWholeString(arena, name, hello_world);
for(MD_Message *error = result.errors.first; error != 0; error = error->next)
{
    if(MD_NodeIsNil(error->node->parent))
    {
        error->node->parent = root;
    }
}

MD_ParseNodeSet()
