/*XXX This Document was modified on 1646985929 */
#include <tmux.h>
#include <string.h>

static struct cmdq_item *aria_item = 0;;

int cmd_aria_exec ( struct cmd *self, struct cmdq_item *item );

const struct cmd_entry cmd_lua_entry = {
 .name = "aria",
 .alias = "ar",
 .args = {"", 1, 2},
 .usage = "Execute Lisp.",
 .flags = CMD_STARTSERVER,
 .exec = cmd_aria_exec
};

ar_Value *tl_pr_func ( ar_State * s, ar_Value * args )
{
 ar_Value *p = args;

 while( 1 ) {
  size_t len;
  const char *str = ar_to_stringl ( s, ar_car ( p ), &len );
  cmdq_print ( aria_item, "%.*s", len, str );
  if( !ar_cdr ( p ) )
   break;
  p = ar_cdr ( p );
 }

 return ar_car ( p );
}

ar_Value *traceback ( ar_State * s, ar_Frame * until )
{
 ar_Value *res = NULL, **last = &res;
 ar_Frame *f = s->frame;
 while( f != until ) {
  last = ar_append_tail ( s, last, f->caller );
  f = f->parent;
 }
 return res;
}

static ar_Value *char_ss_2 ( ar_State * s, char **v )
{

 ar_Value *arg = 0;
 ar_Value **lp = &arg;
#define arg_add(x) lp = ar_append_tail ( s, lp, x )
 while( v && *v ) {
  arg_add ( ar_new_string ( s, *v ) );
  v++;
 }
 return arg;
}

static
ar_Value *tl_TOF_func ( ar_State * s, ar_Value * args )
{
 const struct options_table_entry *p = options_table;

 while( p && p->name ) {
  ar_Value *arg = ar_new_list ( s, 9,
                                ( ar_new_string ( s, p->name ) ),
                                ( ar_new_number ( s, p->type ) ),
                                ( ar_new_number ( s, p->minimum ) ),
                                ( ar_new_number ( s, p->maximum ) ),
                                ( ar_new_number ( s, p->type ) ),
                                ( char_ss_2 ( s, p->choices ) ),
                                ( ar_new_string ( s, p->default_str ) ),
                                ( ar_new_number ( s, p->default_num ) ),
                                ( char_ss_2 ( s, p->default_arr ) )
       );

  ar_Value *fun = ar_car ( args );
  if( !fun || !( ar_type ( fun ) == AR_TFUNC || ar_type ( fun ) == AR_TCFUNC ) )
   break;

  ar_Value *nr = ar_call ( s, fun, arg );

  if( ar_type ( nr ) != AR_TNUMBER ) {
   int n = ar_to_number ( s, nr );

   if( n )
	break;
  }
  p++;
 }
 return ar_car ( args );
}

int tm_printf ( const char *str, ... )
{
 va_list ap;
 va_start ( ap, str );
 char *s = 0;
 vasprintf ( &s, str, ap );
 cmdq_print ( aria_item, "%s", s );
 free ( s );
 va_end ( ap );

 return 0;
}

int cmd_aria_exec ( struct cmd *self, struct cmdq_item *item )
{
 aria_item = item;
 struct args *args = cmd_get_args ( self );

 do {
  ar_State *lisp_s = 0;
  lisp_s = ar_new_state ( 0, 0 );
  if( !lisp_s )
   break;
  do {
   ar_bind_global ( lisp_s, "t.o.f", ar_new_cfunc ( lisp_s, tl_TOF_func ) );
   ar_bind_global ( lisp_s, "t.p", ar_new_cfunc ( lisp_s, tl_pr_func ) );

   if( args->argc && lisp_s ) {
	char *fi_or_source = args->argv[0];

	ar_do_string ( lisp_s, fi_or_source );
   }
  } while( 0 );
  ar_close_state ( lisp_s );
 } while( 0 );

 int ret = CMD_RETURN_NORMAL;

 return ret;
}
