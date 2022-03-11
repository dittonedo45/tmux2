/*XXX This Document was modified on 1646980744 */
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

ar_Value *traceback ( ar_State * S, ar_Frame * until )
{
 ar_Value *res = NULL, **last = &res;
 ar_Frame *f = S->frame;
 while( f != until ) {
  last = ar_append_tail ( S, last, f->caller );
  f = f->parent;
 }
 return res;
}

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

static
ar_Value *tl_TOF_func ( ar_State * s, ar_Value * args )
{
 const struct options_table_entry *p = options_table;

 while( p && p->name ) {
  ar_Value *arg = ar_new_string ( s, p->name );

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
   ar_bind_global ( lisp_s, "tmux.options.foreach",
                    ar_new_cfunc ( lisp_s, tl_TOF_func ) );
   ar_bind_global ( lisp_s, "tmux.display",
                    ar_new_cfunc ( lisp_s, tl_pr_func ) );

   if( args->argc && lisp_s ) {
	char *fi_or_source = args->argv[0];

	if( stat ( fi_or_source, NULL ) ) {
	 ar_do_string ( lisp_s, fi_or_source );
	} else {
	 ar_do_file ( lisp_s, fi_or_source );
	}
   }
  } while( 0 );
  ar_close_state ( lisp_s );
 } while( 0 );

 int ret = CMD_RETURN_NORMAL;

 return ret;
}
