/*XXX This Document was modified on 1646722243 */
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
  cmdq_print ( lua_item, "%s", str );
 return 0;
}

int cmd_lua_exec ( struct cmd *self, struct cmdq_item *item )
{
 lua_item = item;
 struct args *args = cmd_get_args ( self );

 lua_State *s = luaL_newstate (  );
 int ret = CMD_RETURN_NORMAL;
 do {
  lua_pushcfunction ( s, tl_print );
  lua_setglobal ( s, "print" );

  luaL_loadstring ( s, args->argv[0] );
  lua_pcall ( s, 0, 0, 0 );
  if( lua_gettop ( s ) ) {
   cmdq_error ( item, "%s", lua_tostring ( s, -1 ) );
   ret = CMD_RETURN_ERROR;
  }
 } while( 0 );

 lua_close ( s );
 return ret;
}
