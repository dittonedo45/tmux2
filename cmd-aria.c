/*XXX This Document was modified on 1647332679 */
#include <tmux.h>
#include <stdio.h>
#include <string.h>

static struct cmdq_item *aria_item = 0;

int cmd_aria_exec ( struct cmd *self, struct cmdq_item *item );

const struct cmd_entry cmd_lua_entry = {
 .name = "aria",
 .alias = "ar",
 .args = {"", 1, 2},
 .usage = "Execute Lisp.",
 .flags = CMD_STARTSERVER,
 .exec = cmd_aria_exec
};

static void print_buffer ( SCM arg, struct evbuffer *buffer, int level )
{
 int len;
 char buff[1054];

 memset ( buff, 0, sizeof ( buff ) );

#define show_level() buff

 if( scm_is_string ( arg ) ) {
  char *str = scm_to_stringn ( arg, &len, "utf-8", 0 );
  evbuffer_add_printf ( buffer, "%s\"%s\" ", show_level (  ), str );
 } else if( scm_is_number ( arg ) ) {
  mpz_t a;
  mpz_init ( a );

  scm_to_mpz ( arg, a );
  {
   char str[1054];
   gmp_sprintf ( str, "%Zd", a );
   evbuffer_add_printf ( buffer, "%s%s ", show_level (  ), str );
  }
  mpz_clear ( a );
 } else if( scm_is_bool ( arg ) ) {
  evbuffer_add_printf ( buffer, "%s%s ", show_level (  ),
                        scm_to_bool ( arg ) ? "true" : "false" );
 } else if( scm_is_null ( arg ) ) {
  evbuffer_add_printf ( buffer, "null" );
 } else if( scm_is_array ( arg ) || scm_is_pair ( arg ) ) {
  char buf[1054];
  evbuffer_add_printf ( buffer, "%s( ", show_level (  ) );
  while( 1 ) {
   if( scm_is_null ( arg ) || scm_car ( arg ) == 0 ) {
	break;
   }

   print_buffer ( scm_car ( arg ), buffer, level + 2 );
   arg = scm_cdr ( arg );
  }
  evbuffer_add_printf ( buffer, "%s )", show_level (  ) );
 } else if( scm_is_symbol ( arg ) ) {
  char *str = scm_to_utf8_string ( arg );

  evbuffer_add_printf ( buffer, "%s%s ", show_level (  ), str );

 }

}

static void print_value ( SCM arg, int level )
{
 int len;
 char buff[1054];

 memset ( buff, 0, sizeof ( buff ) );

 for( int i = 0; i < level; i++ )
  buff[i] = ' ';

#define show_level() buff

 if( scm_is_string ( arg ) ) {
  char *str = scm_to_stringn ( arg, &len, "utf-8", 0 );
  cmdq_print ( aria_item, "%s%s", show_level (  ), str );
 } else if( scm_is_number ( arg ) ) {
  mpz_t a;
  mpz_init ( a );

  scm_to_mpz ( arg, a );
  {
   char str[1054];
   gmp_sprintf ( str, "%Zd", a );
   cmdq_print ( aria_item, "%s%s", show_level (  ), str );
  }
  mpz_clear ( a );
 } else if( scm_is_bool ( arg ) ) {
  cmdq_print ( aria_item, "%s%s", show_level (  ),
               scm_to_bool ( arg ) ? "true" : "false" );
 } else if( scm_is_null ( arg ) ) {
  cmdq_print ( aria_item, "null" );
 } else if( scm_is_array ( arg ) || scm_is_pair ( arg ) ) {
  char buf[1054];
  cmdq_print ( aria_item, "%s( ", show_level (  ) );
  while( 1 ) {
   if( scm_is_null ( arg ) || scm_car ( arg ) == 0 ) {
	break;
   }

   print_value ( scm_car ( arg ), level + 2 );
   arg = scm_cdr ( arg );
  }
  cmdq_print ( aria_item, "%s )", show_level (  ) );
 } else if( scm_is_symbol ( arg ) ) {
  char *str = scm_to_utf8_string ( arg );

  cmdq_print ( aria_item, "%s%s", show_level (  ), str );

 }

}

static SCM TMUX_display ( SCM arg )
{
 struct evbuffer *buffer = evbuffer_new (  );
 print_buffer ( arg, buffer, 0 );

 cmdq_print ( aria_item, "%s", EVBUFFER_DATA ( buffer ) );
 return arg;
}

static SCM TMUX_puts ( SCM arg )
{
 print_value ( arg, 0 );
 return arg;
}

static SCM TMUX_all_options ( SCM callback )
{
 const struct options_table_entry *p = options_table;
 while( p && p->name ) {
  SCM res = scm_call_1 ( callback, scm_from_utf8_string ( p->name ) );
  if( scm_is_bool ( res ) && scm_to_bool ( res ) == 0 ) {
   break;
  }
  p++;
 }
 return SCM_BOOL_T;
}

static void *lisp_thread ( void *s )
{
 scm_c_define_gsubr ( "t.d", 1, 0, 0, TMUX_display );
 scm_c_define_gsubr ( "t.p", 1, 0, 0, TMUX_puts );
 scm_c_define_gsubr ( "t.o.f", 1, 0, 0, TMUX_all_options );
 scm_eval_string ( scm_from_utf8_string ( ( char * )s ) );
 return NULL;
}

int cmd_aria_exec ( struct cmd *self, struct cmdq_item *item )
{
 aria_item = item;
 struct args *args = cmd_get_args ( self );
 char *fi_or_source = args->argv[0];
 scm_with_guile ( lisp_thread, fi_or_source );
 return CMD_RETURN_NORMAL;
}
