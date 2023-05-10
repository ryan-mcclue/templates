# IMPORTANT(Ryan): QTCreator does not load user commands
# class EntityListDump(gdb.Command):
#    """Collect required info for a bug report"""
#    def __init__(self, val):
#      super().__init__("entity_list_dump", gdb.COMMAND_USER)
# 
#    def complete(self, text, word):
#      # We expect the argument passed to be a symbol so fallback to the
#      # internal tab-completion handler for symbols
#      return gdb.COMPLETE_SYMBOL
# 
#    def invoke(self, args, from_tty):
#      # gdb.execute("thread apply all backtrace full")
#      # result = gdb.execute("thread apply all backtrace full", to_string=True)
# 
#      # convert to a gdb.Value
#      entity_ptr = gdb.parse_and_eval(args)
#      if str(entity_ptr.type) != "Entity *":
#        print("Expected pointer argument of type (Entity *)")
#      else:
#        result_str = ""
#        entity_i = 0
#        while entity_ptr != 0:
#          z_index = entity_ptr["z_index"]
#          result_str += f"\n{entity_i} z_index: {z_index}"
#          entity_ptr = entity_ptr["next"]
#          entity_i += 1
#        result_str = f"List with {entity_i} node{'s' if entity_i > 1 else ''}:" + result_str
#        return result_str
# EntityListDump()

class PrintEntityList:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    entity_ptr = self.val
    result_str = ""
    entity_i = 0
    while entity_ptr != 0:
      result_str += f"\n{entity_i} z_index: {entity_ptr['sprite_component']['z_index']}"
      entity_ptr = entity_ptr["next"]
      entity_i += 1
    result_str = f"List with {entity_i} node{'s' if entity_i > 1 else ''}:" + result_str
    return result_str

def my_pp_func(val):
  if str(val.type) == "Entity *": 
    return PrintEntityList(val)

gdb.pretty_printers.append(my_pp_func)
