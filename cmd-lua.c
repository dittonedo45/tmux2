/*XXX This Document was modified on 1646721337 */
#include <tmux.h>
#include <lua.h>
#include <lauxlib.h>

int cmd_lua_exec ( struct cmd *self, struct cmdq_item *item );
const struct cmd_entry cmd_lua_entry = {
 .name = "lua",
 .alias = "l",
 .args = {"", 0, 1},
 .usage = "Execute Lua.",
 .flags = CMD_STARTSERVER,.exec = cmd_lua_exec
};

static struct cmdq_item *lua_item = 0;;

static int tl_print ( lua_State * s )
{
 const char *str = lua_tostring ( s, -1 );
 if( str )
  cmdq_print ( lua_item, str );
 return 0;
}

int cmd_lua_exec ( struct cmd *self, struct cmdq_item *item )
{
 lua_item = item;
 return 0;
}
