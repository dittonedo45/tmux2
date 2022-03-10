/*XXX This Document was modified on 1646811084 */
#include <tmux.h>
#include <string.h>

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

static int tl_opt_foreach (  )
{
 luaL_checktype ( s, 1, LUA_TFUNCTION );
 struct options_table_entry *p = options_table;

 while( p && p->name ) {
  int g = 1;
  lua_pushnil ( s );
  lua_copy ( s, 1, -1 );
  {
   lua_pushstring ( s, p->name );
   lua_pushstring ( s, p->text );
   if( p->default_str ) {
	lua_pushstring ( s, p->default_str );
   } else {
	lua_pushnil ( s );
   }
   lua_call ( s, 3, 1 );
  }

  if( lua_type ( s, -1 ) == LUA_TBOOLEAN ) {
   g = lua_toboolean ( s, -1 );
  }
  lua_remove ( s, -1 );

  if( !g )
   break;
  p++;
 }
 return 0;
}

static void tl_pr_show ( lua_State * s, int l, int i )
{
 switch ( lua_type ( s, i ) ) {
 case LUA_TNUMBER:
 case LUA_TBOOLEAN:
 case LUA_TNIL:
 case LUA_TSTRING:
  {
   const char *str = lua_tostring ( s, i );
   if( str ) {
	char buf[strlen ( str ) + l + 1];
	for( int j = 0; j < l; j++ ) {
	 buf[j] = ' ';
	}
	strcpy ( &buf[l], str );
	cmdq_print ( lua_item, "%s", buf );
   }
  }
  break;
 case LUA_TFUNCTION:
  {
   char *str = "<method>";

   char buf[strlen ( str ) + l + 1];
   for( int j = 0; j < l; j++ ) {
	buf[j] = ' ';
   }
   strcpy ( &buf[l], str );
   cmdq_print ( lua_item, "%s", buf );
  }
  break;
 case LUA_TTABLE:
  {
   lua_pushnil ( s );
   while( lua_next ( s, i ) ) {
	tl_pr_show ( s, l + 1, lua_gettop ( s ) - 1 );
	tl_pr_show ( s, l + 2, lua_gettop ( s ) );
	lua_remove ( s, -1 );
   }
  }
 }
}

static int tl_print ( lua_State * s )
{
#if 1
 for( int i = 0; i < lua_gettop ( s ); i++ ) {
  tl_pr_show ( s, 1, i + 1 );
 }
#endif
 return 0;
}

int cmd_lua_exec ( struct cmd *self, struct cmdq_item *item )
{
 lua_item = item;
 struct args *args = cmd_get_args ( self );
 if( !s ) {
  s = luaL_newstate (  );
  luaL_openlibs ( s );
 }
 int ret = CMD_RETURN_NORMAL;
 do {
  ret = CMD_RETURN_ERROR;

  lua_newtable ( s );
  lua_newtable ( s );
  lua_pushcfunction ( s, tl_opt_foreach );
  lua_setfield ( s, -2, "foreach" );
  lua_setfield ( s, -2, "options" );
  lua_setglobal ( s, "tmux" );

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
