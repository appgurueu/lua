#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>

static void check(int cond) {
  if (!cond) {
    printf("Check failed at " __FILE__ ":%d\n", __LINE__);
    exit(EXIT_FAILURE);
  }
}

static int us_elapsed(const struct timeval *start, const struct timeval *end) {
  return ((end->tv_sec - start->tv_sec) * 1000000) + (end->tv_usec - start->tv_usec);
}

static struct timeval tv_start, tv_end;

static void start_timer() {
  check(gettimeofday(&tv_start, NULL) == 0);
}

static void stop_timer(const char *msg) {
  check(gettimeofday(&tv_end, NULL) == 0);
  printf("Elapsed time for %s: %d us\n", msg, us_elapsed(&tv_start, &tv_end));
}

static int count_ints(lua_State *L, int t) {
  int ints = 0;
  lua_pushnil(L);  /* first key */
  while (lua_next(L, t) != 0) {
    /* key at index -2, value at index -1 */
    if (lua_isinteger(L, -2))
      ints++;
    lua_pop(L, 1);
  }
  return ints;
}

static void set_int_kv(lua_State *L, int t, int k, int v) {
  lua_pushnumber(L, v);
  lua_rawseti(L, t, k);
}

static void test_lua_issequence() {
  lua_State *L = luaL_newstate();
  lua_createtable(L, 3, 0);
  int t = lua_gettop(L);
  set_int_kv(L, t, 1, 1);
  set_int_kv(L, t, 2, 2);
  set_int_kv(L, t, 3, 3);
  check(lua_issequence(L, t));
  set_int_kv(L, t, 5, 5);
  check(!lua_issequence(L, t));
  set_int_kv(L, t, 4, 4);
  check(lua_issequence(L, t));
  lua_pushstring(L, "foo");
  lua_setfield(L, t, "bar");
  check(!lua_issequence(L, t));
  lua_pop(L, 1); /* remove the table */
  lua_close(L);
}

int main(void) {
  test_lua_issequence();

  const int n = 100000000;
  lua_State *L = luaL_newstate();

  /* run a rudimentary benchmark to see how fast different iteration vs. an optimized lua_issequence are at scale */

  /* populate a table and see how expensive that is, just to get an idea of the costs involved */
  start_timer();
  lua_createtable(L, n, 0);
  int t = lua_gettop(L);
  for (int i = 1; i <= n; i++) {
    lua_pushnumber(L, i*2);
    lua_rawseti(L, t, i);  /* t[i] = 2*i */
  }
  stop_timer("constructing table");

  /* benchmark lua_issequence */
  start_timer();
  check(lua_issequence(L, t));
  stop_timer("lua_issequence");

  /* benchmark a simple traversal */
  start_timer();
  check(count_ints(L, t) == n);
  stop_timer("count_ints (traversal using lua_next)");

  lua_pop(L, 1);  /* Take the returned value out of the stack */
  lua_close(L);

  return 0;
}
