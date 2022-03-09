/*XXX This Document was modified on 1646806042 */
#include <tmux.h>

lua_State *s = 0;
int cmd_lua_exec ( struct cmd *self, struct cmdq_item *item );
const struct cmd_entry cmd_lua_entry = {
 .name = "lua",
 .alias = "l",
 .args = {"", 0, 2},
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
 if( !s ) {
  s = luaL_newstate (  );
 }
 int ret = CMD_RETURN_NORMAL;
 do {
  ret = CMD_RETURN_ERROR;
  lua_pushcfunction ( s, tl_print );
  lua_setglobal ( s, "print" );

  if( args->argc == 1 ) {
   luaL_loadstring ( s, args->argv[0] );
   if( lua_pcall ( s, 0, 0, 0 ) ) {
	cmdq_error ( item, "%s", lua_tostring ( s, -1 ) );
	break;
   }
  } else {

   if( luaL_loadfile ( s, args->argv[1] ) ) {
	cmdq_error ( item, "%s", "Be Utter, speacific.\n" );
	break;
   }

   if( lua_pcall ( s, 0, 0, 0 ) ) {
	cmdq_error ( item, "%s", lua_tostring ( s, -1 ) );
	break;
   }

   if( luaL_loadstring ( s, args->argv[0] ) )
	break;
   if( lua_pcall ( s, 0, 0, 0 ) ) {
	cmdq_error ( item, "%s", lua_tostring ( s, -1 ) );
	break;
   }

  }

  ret = CMD_RETURN_NORMAL;
 } while( 0 );

 //lua_close ( s );
 return ret;
}
